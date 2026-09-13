#pragma once
#include <SDL2/SDL.h>
#include <memory>
#include <string>
#include <vector>
#include "Shape.h"
#include "Toolbox.h"

// Janela, laço principal e máquina de estados de interação (ferramenta
// ativa, forma sendo composta, forma selecionada/arrastada).
class DrawingApp {
public:
    DrawingApp(const std::string& title, int w, int h);
    ~DrawingApp();

    void run();

private:
    SDL_Window* window_ = nullptr;
    SDL_Surface* surface_ = nullptr;
    bool running_ = false;
    int width_, height_;

    Toolbox toolbox_;
    Tool currentTool_ = Tool::Select;
    Uint32 currentColor_ = 0;

    std::vector<std::unique_ptr<Shape>> shapes_;
    std::vector<Vec2i> pendingPoints_; // pontos já clicados da figura em composição

    Shape* selected_ = nullptr;
    bool draggingBody_ = false;
    bool draggingHandle_ = false;
    int draggingHandleIndex_ = -1;
    Vec2i lastMouse_{0, 0};

    void handleEvents();
    void render(const Vec2i& mouse);

    void onMouseDown(const Vec2i& p, Uint8 clicks);
    void onMouseUp(const Vec2i& p);
    void onMouseMove(const Vec2i& p);
    void onRightClick(const Vec2i& p);
    void onKeyDown(SDL_Keycode key, Uint16 mod);

    void selectTool(Tool t);
    void finalizeShapeIfReady();
    void finalizePolygon();
    void saveImage();

    Shape* hitTestShapes(const Vec2i& p) const;
    int hitTestHandle(const Shape* s, const Vec2i& p) const;
};
