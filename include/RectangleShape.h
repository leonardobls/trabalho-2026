#pragma once
#include "Shape.h"
#include "Primitives.h"
#include <algorithm>
#include <tuple>

// Guarda os dois cantos exatamente como foram clicados/arrastados (podem
// estar em qualquer ordem); bounds() normaliza para desenhar/testar.
class RectangleShape : public Shape {
public:
    RectangleShape(const Vec2i& a, const Vec2i& b, Uint32 color) : Shape(color) {
        points_ = {a, b};
    }

    ShapeType type() const override { return ShapeType::Rectangle; }
    bool isClosed() const override { return true; }

    void draw(SDL_Surface* surface) const override {
        auto [x1, y1, x2, y2] = bounds();
        Primitives::drawLine(surface, x1, y1, x2, y1, color);
        Primitives::drawLine(surface, x2, y1, x2, y2, color);
        Primitives::drawLine(surface, x2, y2, x1, y2, color);
        Primitives::drawLine(surface, x1, y2, x1, y1, color);
    }

    bool hitTest(const Vec2i& p, double maxDist) const override {
        auto [x1, y1, x2, y2] = bounds();
        Vec2i tl{x1, y1}, tr{x2, y1}, br{x2, y2}, bl{x1, y2};
        return Primitives::distPointToSegment(p, tl, tr) <= maxDist ||
               Primitives::distPointToSegment(p, tr, br) <= maxDist ||
               Primitives::distPointToSegment(p, br, bl) <= maxDist ||
               Primitives::distPointToSegment(p, bl, tl) <= maxDist;
    }

    bool contains(const Vec2i& p) const override {
        auto [x1, y1, x2, y2] = bounds();
        return p.x >= x1 && p.x <= x2 && p.y >= y1 && p.y <= y2;
    }

private:
    std::tuple<int, int, int, int> bounds() const {
        int x1 = std::min(points_[0].x, points_[1].x);
        int x2 = std::max(points_[0].x, points_[1].x);
        int y1 = std::min(points_[0].y, points_[1].y);
        int y2 = std::max(points_[0].y, points_[1].y);
        return {x1, y1, x2, y2};
    }
};
