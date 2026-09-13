#pragma once
#include <cmath>

// Ponto/vetor 2D inteiro, usado tanto para coordenadas de tela quanto
// para pontos de controle das formas.
struct Vec2i {
    int x = 0;
    int y = 0;

    Vec2i() = default;
    Vec2i(int x, int y) : x(x), y(y) {}

    Vec2i operator+(const Vec2i& o) const { return Vec2i(x + o.x, y + o.y); }
    Vec2i operator-(const Vec2i& o) const { return Vec2i(x - o.x, y - o.y); }

    static double distance(const Vec2i& a, const Vec2i& b) {
        double dx = a.x - b.x;
        double dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }
};
