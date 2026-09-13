#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "Vec2i.h"

enum class Tool { Select, Line, Rectangle, Circle, Bezier, Polygon, Fill };

// Barra de ferramentas desenhada no topo da janela: uma caixa por
// ferramenta (selecionável por clique ou tecla) e uma paleta de cores.
class Toolbox {
public:
    static const int HEIGHT = 56;

    explicit Toolbox(int windowW);

    void draw(SDL_Surface* surface, Tool current, Uint32 currentColor) const;

    bool hitTestTool(const Vec2i& click, Tool& outTool) const;
    bool hitTestColor(const Vec2i& click, int& outIndex) const;

    std::vector<Uint32> palette; // preenchida pela DrawingApp após criar a surface

private:
    struct Btn { Tool tool; SDL_Rect rect; };

    std::vector<Btn> buttons_;
    std::vector<SDL_Rect> swatches_;

    void drawIcon(SDL_Surface* surface, const Btn& b) const;
};
