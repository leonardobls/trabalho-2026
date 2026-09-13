#include "Toolbox.h"
#include "Primitives.h"
#include <cmath>

namespace {
const double PI = 3.14159265358979323846;
const int BTN_SIZE = 48;
const int BTN_GAP = 6;
const int BTN_START_X = 8;
const int SWATCH_SIZE = 28;
const int SWATCH_GAP = 6;

void border(SDL_Surface* s, const SDL_Rect& r, Uint32 color) {
    Primitives::drawLine(s, r.x, r.y, r.x + r.w - 1, r.y, color);
    Primitives::drawLine(s, r.x + r.w - 1, r.y, r.x + r.w - 1, r.y + r.h - 1, color);
    Primitives::drawLine(s, r.x + r.w - 1, r.y + r.h - 1, r.x, r.y + r.h - 1, color);
    Primitives::drawLine(s, r.x, r.y + r.h - 1, r.x, r.y, color);
}
}

Toolbox::Toolbox(int windowW) {
    (void)windowW;
    Tool order[] = {Tool::Select, Tool::Line, Tool::Rectangle, Tool::Circle,
                     Tool::Bezier, Tool::Polygon, Tool::Fill};
    for (int i = 0; i < 7; i++) {
        SDL_Rect r{BTN_START_X + i * (BTN_SIZE + BTN_GAP), 4, BTN_SIZE, BTN_SIZE};
        buttons_.push_back({order[i], r});
    }

    int lastRight = BTN_START_X + 7 * (BTN_SIZE + BTN_GAP);
    int paletteStart = lastRight + 20;
    int swatchY = (HEIGHT - SWATCH_SIZE) / 2;
    for (int i = 0; i < 8; i++) {
        SDL_Rect r{paletteStart + i * (SWATCH_SIZE + SWATCH_GAP), swatchY, SWATCH_SIZE, SWATCH_SIZE};
        swatches_.push_back(r);
    }
}

void Toolbox::drawIcon(SDL_Surface* s, const Btn& b) const {
    Uint32 black = Primitives::rgbToUint32(s, 0, 0, 0);
    int x = b.rect.x, y = b.rect.y;

    switch (b.tool) {
        case Tool::Select:
            Primitives::drawLine(s, x + 10, y + 8, x + 10, y + 34, black);
            Primitives::drawLine(s, x + 10, y + 34, x + 20, y + 26, black);
            Primitives::drawLine(s, x + 20, y + 26, x + 10, y + 8, black);
            break;
        case Tool::Line:
            Primitives::drawLine(s, x + 10, y + 38, x + 38, y + 10, black);
            break;
        case Tool::Rectangle:
            Primitives::drawLine(s, x + 10, y + 14, x + 38, y + 14, black);
            Primitives::drawLine(s, x + 38, y + 14, x + 38, y + 38, black);
            Primitives::drawLine(s, x + 38, y + 38, x + 10, y + 38, black);
            Primitives::drawLine(s, x + 10, y + 38, x + 10, y + 14, black);
            break;
        case Tool::Circle:
            Primitives::drawCircle(s, x + 24, y + 24, 14, black);
            break;
        case Tool::Bezier:
            Primitives::drawBezierCubic(s, Vec2i(x + 8, y + 34), Vec2i(x + 16, y + 8),
                                         Vec2i(x + 32, y + 40), Vec2i(x + 40, y + 14), black);
            break;
        case Tool::Polygon: {
            const int n = 5;
            Vec2i pts[n];
            for (int i = 0; i < n; i++) {
                double a = -PI / 2 + i * (2 * PI / n);
                pts[i] = Vec2i(x + 24 + (int)std::lround(14 * std::cos(a)),
                                y + 24 + (int)std::lround(14 * std::sin(a)));
            }
            for (int i = 0; i < n; i++) {
                Vec2i a = pts[i], bp = pts[(i + 1) % n];
                Primitives::drawLine(s, a.x, a.y, bp.x, bp.y, black);
            }
            break;
        }
        case Tool::Fill:
            Primitives::drawLine(s, x + 10, y + 14, x + 34, y + 14, black);
            Primitives::drawLine(s, x + 34, y + 14, x + 22, y + 34, black);
            Primitives::drawLine(s, x + 22, y + 34, x + 10, y + 14, black);
            for (int dy = -2; dy <= 2; dy++)
                for (int dx = -2; dx <= 2; dx++)
                    Primitives::setPixel(s, x + 34 + dx, y + 34 + dy, black);
            break;
    }
}

void Toolbox::draw(SDL_Surface* surface, Tool current, Uint32 currentColor) const {
    SDL_Rect band{0, 0, surface->w, HEIGHT};
    SDL_FillRect(surface, &band, Primitives::rgbToUint32(surface, 235, 235, 235));
    Primitives::drawLine(surface, 0, HEIGHT - 1, surface->w - 1, HEIGHT - 1,
                          Primitives::rgbToUint32(surface, 0, 0, 0));

    for (const auto& b : buttons_) {
        bool active = (b.tool == current);
        Uint32 bg = active ? Primitives::rgbToUint32(surface, 180, 210, 255)
                            : Primitives::rgbToUint32(surface, 220, 220, 220);
        SDL_FillRect(surface, &b.rect, bg);
        border(surface, b.rect, Primitives::rgbToUint32(surface, 0, 0, 0));
        if (active) {
            SDL_Rect inset{b.rect.x + 2, b.rect.y + 2, b.rect.w - 4, b.rect.h - 4};
            border(surface, inset, Primitives::rgbToUint32(surface, 255, 120, 0));
        }
        drawIcon(surface, b);
    }

    for (size_t i = 0; i < swatches_.size() && i < palette.size(); i++) {
        const SDL_Rect& r = swatches_[i];
        SDL_FillRect(surface, &r, palette[i]);
        border(surface, r, Primitives::rgbToUint32(surface, 0, 0, 0));
        if (palette[i] == currentColor) {
            SDL_Rect outer{r.x - 3, r.y - 3, r.w + 6, r.h + 6};
            border(surface, outer, Primitives::rgbToUint32(surface, 255, 120, 0));
        }
    }
}

bool Toolbox::hitTestTool(const Vec2i& click, Tool& outTool) const {
    for (const auto& b : buttons_) {
        if (click.x >= b.rect.x && click.x < b.rect.x + b.rect.w &&
            click.y >= b.rect.y && click.y < b.rect.y + b.rect.h) {
            outTool = b.tool;
            return true;
        }
    }
    return false;
}

bool Toolbox::hitTestColor(const Vec2i& click, int& outIndex) const {
    for (size_t i = 0; i < swatches_.size(); i++) {
        const SDL_Rect& r = swatches_[i];
        if (click.x >= r.x && click.x < r.x + r.w && click.y >= r.y && click.y < r.y + r.h) {
            outIndex = (int)i;
            return true;
        }
    }
    return false;
}
