#pragma once
#include "Shape.h"
#include "Primitives.h"
#include <vector>

// Número arbitrário de vértices (mínimo 3), com a aresta de fechamento
// implícita entre o último e o primeiro ponto.
class PolygonShape : public Shape {
public:
    PolygonShape(std::vector<Vec2i> vertices, Uint32 color) : Shape(color) {
        points_ = std::move(vertices);
    }

    ShapeType type() const override { return ShapeType::Polygon; }
    bool isClosed() const override { return true; }

    void draw(SDL_Surface* surface) const override {
        size_t n = points_.size();
        for (size_t i = 0; i < n; i++) {
            const Vec2i& a = points_[i];
            const Vec2i& b = points_[(i + 1) % n];
            Primitives::drawLine(surface, a.x, a.y, b.x, b.y, color);
        }
    }

    bool hitTest(const Vec2i& p, double maxDist) const override {
        size_t n = points_.size();
        for (size_t i = 0; i < n; i++) {
            if (Primitives::distPointToSegment(p, points_[i], points_[(i + 1) % n]) <= maxDist) return true;
        }
        return false;
    }

    bool contains(const Vec2i& p) const override {
        return Primitives::pointInPolygon(p, points_);
    }
};
