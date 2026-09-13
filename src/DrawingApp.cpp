#include "DrawingApp.h"
#include "Primitives.h"
#include "LineShape.h"
#include "RectangleShape.h"
#include "CircleShape.h"
#include "BezierShape.h"
#include "PolygonShape.h"
#include <algorithm>
#include <iostream>

DrawingApp::DrawingApp(const std::string& title, int w, int h)
    : width_(w), height_(h), toolbox_(w) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Erro ao inicializar SDL: " << SDL_GetError() << std::endl;
        std::exit(1);
    }

    window_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                width_, height_, SDL_WINDOW_SHOWN);
    if (!window_) {
        std::cerr << "Erro ao criar janela: " << SDL_GetError() << std::endl;
        SDL_Quit();
        std::exit(1);
    }
    surface_ = SDL_GetWindowSurface(window_);

    toolbox_.palette = {
        Primitives::rgbToUint32(surface_, 0, 0, 0),
        Primitives::rgbToUint32(surface_, 220, 0, 0),
        Primitives::rgbToUint32(surface_, 0, 140, 0),
        Primitives::rgbToUint32(surface_, 0, 0, 220),
        Primitives::rgbToUint32(surface_, 230, 190, 0),
        Primitives::rgbToUint32(surface_, 230, 110, 0),
        Primitives::rgbToUint32(surface_, 150, 0, 190),
        Primitives::rgbToUint32(surface_, 120, 120, 120),
    };
    currentColor_ = toolbox_.palette[0];
}

DrawingApp::~DrawingApp() {
    if (window_) SDL_DestroyWindow(window_);
    SDL_Quit();
}

void DrawingApp::run() {
    running_ = true;
    while (running_) {
        handleEvents();
        int mx = 0, my = 0;
        SDL_GetMouseState(&mx, &my);
        render(Vec2i(mx, my));
        SDL_Delay(16);
    }
}

void DrawingApp::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                running_ = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (e.button.button == SDL_BUTTON_LEFT) {
                    onMouseDown(Vec2i(e.button.x, e.button.y), e.button.clicks);
                } else if (e.button.button == SDL_BUTTON_RIGHT) {
                    onRightClick(Vec2i(e.button.x, e.button.y));
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (e.button.button == SDL_BUTTON_LEFT) {
                    onMouseUp(Vec2i(e.button.x, e.button.y));
                }
                break;
            case SDL_MOUSEMOTION:
                onMouseMove(Vec2i(e.motion.x, e.motion.y));
                break;
            case SDL_KEYDOWN:
                onKeyDown(e.key.keysym.sym, e.key.keysym.mod);
                break;
            default:
                break;
        }
    }
}

void DrawingApp::selectTool(Tool t) {
    currentTool_ = t;
    pendingPoints_.clear();
}

void DrawingApp::onMouseDown(const Vec2i& p, Uint8 clicks) {
    if (p.y < Toolbox::HEIGHT) {
        Tool t;
        int idx;
        if (toolbox_.hitTestTool(p, t)) { selectTool(t); return; }
        if (toolbox_.hitTestColor(p, idx)) { currentColor_ = toolbox_.palette[idx]; return; }
        return;
    }

    if (currentTool_ == Tool::Select) {
        if (selected_) {
            int hi = hitTestHandle(selected_, p);
            if (hi >= 0) {
                draggingHandle_ = true;
                draggingHandleIndex_ = hi;
                lastMouse_ = p;
                return;
            }
        }
        selected_ = hitTestShapes(p);
        if (selected_) {
            draggingBody_ = true;
            lastMouse_ = p;
        }
        return;
    }

    if (currentTool_ == Tool::Fill) {
        for (auto it = shapes_.rbegin(); it != shapes_.rend(); ++it) {
            if ((*it)->isClosed() && (*it)->contains(p)) {
                (*it)->hasFill = true;
                (*it)->fillColor = currentColor_;
                break;
            }
        }
        return;
    }

    // ferramentas de desenho: acumula pontos até a figura estar completa
    pendingPoints_.push_back(p);
    if (currentTool_ == Tool::Polygon) {
        if (clicks >= 2 && pendingPoints_.size() >= 2) {
            pendingPoints_.pop_back(); // remove o ponto duplicado do duplo-clique
            finalizePolygon();
        }
        return;
    }
    finalizeShapeIfReady();
}

void DrawingApp::onRightClick(const Vec2i&) {
    if (currentTool_ == Tool::Polygon && pendingPoints_.size() >= 3) {
        finalizePolygon();
    }
}

void DrawingApp::onMouseUp(const Vec2i&) {
    draggingBody_ = false;
    draggingHandle_ = false;
    draggingHandleIndex_ = -1;
}

void DrawingApp::onMouseMove(const Vec2i& p) {
    if (draggingBody_ && selected_) {
        int dx = p.x - lastMouse_.x;
        int dy = p.y - lastMouse_.y;
        selected_->translate(dx, dy);
        lastMouse_ = p;
    } else if (draggingHandle_ && selected_) {
        selected_->setControlPoint((size_t)draggingHandleIndex_, p);
        lastMouse_ = p;
    }
}

void DrawingApp::onKeyDown(SDL_Keycode key, Uint16 mod) {
    if ((mod & KMOD_CTRL) && key == SDLK_s) {
        saveImage();
        return;
    }

    switch (key) {
        case SDLK_ESCAPE:
            pendingPoints_.clear();
            selected_ = nullptr;
            break;
        case SDLK_s: selectTool(Tool::Select); break;
        case SDLK_l: selectTool(Tool::Line); break;
        case SDLK_r: selectTool(Tool::Rectangle); break;
        case SDLK_c: selectTool(Tool::Circle); break;
        case SDLK_b: selectTool(Tool::Bezier); break;
        case SDLK_p: selectTool(Tool::Polygon); break;
        case SDLK_f: selectTool(Tool::Fill); break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (currentTool_ == Tool::Polygon && pendingPoints_.size() >= 3) finalizePolygon();
            break;
        default:
            break;
    }

    if (!selected_) return;

    switch (key) {
        case SDLK_DELETE:
        case SDLK_BACKSPACE: {
            auto it = std::find_if(shapes_.begin(), shapes_.end(),
                [&](const std::unique_ptr<Shape>& s) { return s.get() == selected_; });
            if (it != shapes_.end()) shapes_.erase(it);
            selected_ = nullptr;
            break;
        }
        case SDLK_EQUALS:
        case SDLK_KP_PLUS:
            selected_->scale(1.1, selected_->centroid());
            break;
        case SDLK_MINUS:
        case SDLK_KP_MINUS:
            selected_->scale(0.9, selected_->centroid());
            break;
        case SDLK_LEFTBRACKET:
            selected_->rotate(-5.0, selected_->centroid());
            break;
        case SDLK_RIGHTBRACKET:
            selected_->rotate(5.0, selected_->centroid());
            break;
        case SDLK_UP:    selected_->translate(0, -1); break;
        case SDLK_DOWN:  selected_->translate(0, 1); break;
        case SDLK_LEFT:  selected_->translate(-1, 0); break;
        case SDLK_RIGHT: selected_->translate(1, 0); break;
        default:
            break;
    }
}

void DrawingApp::finalizeShapeIfReady() {
    switch (currentTool_) {
        case Tool::Line:
            if (pendingPoints_.size() == 2) {
                shapes_.push_back(std::make_unique<LineShape>(pendingPoints_[0], pendingPoints_[1], currentColor_));
                pendingPoints_.clear();
            }
            break;
        case Tool::Rectangle:
            if (pendingPoints_.size() == 2) {
                shapes_.push_back(std::make_unique<RectangleShape>(pendingPoints_[0], pendingPoints_[1], currentColor_));
                pendingPoints_.clear();
            }
            break;
        case Tool::Circle:
            if (pendingPoints_.size() == 2) {
                shapes_.push_back(std::make_unique<CircleShape>(pendingPoints_[0], pendingPoints_[1], currentColor_));
                pendingPoints_.clear();
            }
            break;
        case Tool::Bezier:
            if (pendingPoints_.size() == 4) {
                shapes_.push_back(std::make_unique<BezierShape>(
                    pendingPoints_[0], pendingPoints_[1], pendingPoints_[2], pendingPoints_[3], currentColor_));
                pendingPoints_.clear();
            }
            break;
        default:
            break;
    }
}

void DrawingApp::finalizePolygon() {
    shapes_.push_back(std::make_unique<PolygonShape>(pendingPoints_, currentColor_));
    pendingPoints_.clear();
}

void DrawingApp::saveImage() {
    if (SDL_SaveBMP(surface_, "files/desenho.bmp") == 0) {
        std::cout << "Imagem salva em files/desenho.bmp" << std::endl;
    } else {
        std::cerr << "Erro ao salvar imagem: " << SDL_GetError() << std::endl;
    }
}

Shape* DrawingApp::hitTestShapes(const Vec2i& p) const {
    for (auto it = shapes_.rbegin(); it != shapes_.rend(); ++it) {
        if ((*it)->hitTest(p, 5.0)) return it->get();
    }
    return nullptr;
}

int DrawingApp::hitTestHandle(const Shape* s, const Vec2i& p) const {
    const std::vector<Vec2i>& pts = s->controlPoints();
    for (size_t i = 0; i < pts.size(); i++) {
        if (Vec2i::distance(p, pts[i]) <= 6.0) return (int)i;
    }
    return -1;
}

void DrawingApp::render(const Vec2i& mouse) {
    Uint32 white = Primitives::rgbToUint32(surface_, 255, 255, 255);
    SDL_FillRect(surface_, nullptr, white);

    for (const auto& s : shapes_) {
        s->draw(surface_);
        s->applyFill(surface_);
    }

    if (selected_) selected_->drawHandles(surface_);

    if (!pendingPoints_.empty()) {
        Uint32 preview = Primitives::rgbToUint32(surface_, 150, 150, 150);
        for (size_t i = 0; i + 1 < pendingPoints_.size(); i++) {
            Primitives::drawLine(surface_, pendingPoints_[i].x, pendingPoints_[i].y,
                                  pendingPoints_[i + 1].x, pendingPoints_[i + 1].y, preview);
        }
        Vec2i last = pendingPoints_.back();
        Primitives::drawLine(surface_, last.x, last.y, mouse.x, mouse.y, preview);
    }

    toolbox_.draw(surface_, currentTool_, currentColor_);

    SDL_UpdateWindowSurface(window_);
}
