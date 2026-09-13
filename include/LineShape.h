#pragma once
#include "Shape.h"
#include "Primitives.h"

class LineShape : public Shape {
public:
    LineShape(const Vec2i& a, const Vec2i& b, Uint32 color) : Shape(color) {
        points_ = {a, b};
    }

    ShapeType type() const override { return ShapeType::Line; }
    bool isClosed() const override { return false; }

    void draw(SDL_Surface* surface) const override {
        Primitives::drawLine(surface, points_[0].x, points_[0].y, points_[1].x, points_[1].y, color);
    }

    bool hitTest(const Vec2i& p, double maxDist) const override {
        return Primitives::distPointToSegment(p, points_[0], points_[1]) <= maxDist;
    }
};
