#include "Shape.h"
#include "Primitives.h"
#include <algorithm>
#include <cmath>

Vec2i Shape::centroid() const {
    if (points_.empty()) return Vec2i(0, 0);
    long sx = 0, sy = 0;
    for (const auto& p : points_) { sx += p.x; sy += p.y; }
    return Vec2i((int)(sx / (long)points_.size()), (int)(sy / (long)points_.size()));
}

void Shape::setControlPoint(size_t i, const Vec2i& p) {
    if (i < points_.size()) points_[i] = p;
}

void Shape::translate(int dx, int dy) {
    for (auto& p : points_) { p.x += dx; p.y += dy; }
}

// Escala em torno de um pivô arbitrário: transladar para a origem (relativo
// ao pivô), aplicar o fator, transladar de volta — exatamente como na aula.
void Shape::scale(double factor, const Vec2i& pivot) {
    for (auto& p : points_) {
        p.x = pivot.x + (int)std::lround((p.x - pivot.x) * factor);
        p.y = pivot.y + (int)std::lround((p.y - pivot.y) * factor);
    }
}

// Rotação em torno de um pivô arbitrário, usando x'=x*cosθ-y*sinθ,
// y'=x*sinθ+y*cosθ (aula 06), com o mesmo esquema de transladar/rotacionar/
// transladar de volta.
void Shape::rotate(double degreesCCW, const Vec2i& pivot) {
    double rad = degreesCCW * 0.017453293;
    double c = std::cos(rad), s = std::sin(rad);
    for (auto& p : points_) {
        double dx = p.x - pivot.x, dy = p.y - pivot.y;
        double nx = dx * c - dy * s;
        double ny = dx * s + dy * c;
        p.x = pivot.x + (int)std::lround(nx);
        p.y = pivot.y + (int)std::lround(ny);
    }
}

void Shape::drawHandles(SDL_Surface* surface) const {
    if (points_.empty()) return;
    Uint32 highlight = Primitives::rgbToUint32(surface, 255, 0, 200);

    for (const auto& p : points_) {
        for (int dy = -3; dy <= 3; dy++)
            for (int dx = -3; dx <= 3; dx++)
                Primitives::setPixel(surface, p.x + dx, p.y + dy, highlight);
    }

    int minX = points_[0].x, maxX = points_[0].x;
    int minY = points_[0].y, maxY = points_[0].y;
    for (const auto& p : points_) {
        minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
    }
    minX -= 6; minY -= 6; maxX += 6; maxY += 6;

    Primitives::drawLine(surface, minX, minY, maxX, minY, highlight);
    Primitives::drawLine(surface, maxX, minY, maxX, maxY, highlight);
    Primitives::drawLine(surface, maxX, maxY, minX, maxY, highlight);
    Primitives::drawLine(surface, minX, maxY, minX, minY, highlight);
}

void Shape::applyFill(SDL_Surface* surface) const {
    if (!hasFill) return;
    Vec2i c = centroid();
    Primitives::floodFill(surface, c.x, c.y, fillColor);
}
