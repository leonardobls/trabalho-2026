#pragma once
#include "Shape.h"
#include "Primitives.h"
#include <cmath>

// points_[0] = centro, points_[1] = ponto na borda que define o raio.
class CircleShape : public Shape {
public:
    CircleShape(const Vec2i& center, const Vec2i& edge, Uint32 color) : Shape(color) {
        points_ = {center, edge};
    }

    ShapeType type() const override { return ShapeType::Circle; }
    bool isClosed() const override { return true; }
    Vec2i centroid() const override { return points_[0]; }

    void draw(SDL_Surface* surface) const override {
        Primitives::drawCircle(surface, points_[0].x, points_[0].y, radius(), color);
    }

    bool hitTest(const Vec2i& p, double maxDist) const override {
        return std::fabs(Vec2i::distance(p, points_[0]) - radius()) <= maxDist;
    }

    bool contains(const Vec2i& p) const override {
        return Vec2i::distance(p, points_[0]) <= radius();
    }

private:
    int radius() const { return (int)std::lround(Vec2i::distance(points_[0], points_[1])); }
};
