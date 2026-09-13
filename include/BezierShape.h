#pragma once
#include "Shape.h"
#include "Primitives.h"

// Curva de Bézier cúbica: 4 pontos de controle (2 âncoras + 2 alças).
class BezierShape : public Shape {
public:
    BezierShape(const Vec2i& p0, const Vec2i& p1, const Vec2i& p2, const Vec2i& p3, Uint32 color)
        : Shape(color) {
        points_ = {p0, p1, p2, p3};
    }

    ShapeType type() const override { return ShapeType::Bezier; }
    bool isClosed() const override { return false; }

    void draw(SDL_Surface* surface) const override {
        Primitives::drawBezierCubic(surface, points_[0], points_[1], points_[2], points_[3], color);
    }

    bool hitTest(const Vec2i& p, double maxDist) const override {
        std::vector<Vec2i> samples = Primitives::bezierPoints(points_[0], points_[1], points_[2], points_[3]);
        for (size_t i = 0; i + 1 < samples.size(); i++) {
            if (Primitives::distPointToSegment(p, samples[i], samples[i + 1]) <= maxDist) return true;
        }
        return false;
    }
};
