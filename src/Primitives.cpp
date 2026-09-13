#include "Primitives.h"
#include <cmath>
#include <stack>
#include <utility>

void Primitives::setPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
    if (!surface || x < 0 || y < 0 || x >= surface->w || y >= surface->h) return;
    Uint32* pixels = (Uint32*)surface->pixels;
    int pitch = surface->pitch / 4;
    pixels[y * pitch + x] = color;
}

Uint32 Primitives::getPixel(SDL_Surface* surface, int x, int y) {
    if (!surface || x < 0 || y < 0 || x >= surface->w || y >= surface->h) return 0;
    Uint32* pixels = (Uint32*)surface->pixels;
    int pitch = surface->pitch / 4;
    return pixels[y * pitch + x];
}

Uint32 Primitives::rgbToUint32(SDL_Surface* surface, Uint8 r, Uint8 g, Uint8 b) {
    return SDL_MapRGB(surface->format, r, g, b);
}

// Algoritmo de Bresenham para retas
void Primitives::drawLine(SDL_Surface* surface, int x1, int y1, int x2, int y2, Uint32 color) {
    int dx = std::abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -std::abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        setPixel(surface, x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

// Algoritmo do ponto médio (Bresenham) para círculos, com simetria de 8 octantes
void Primitives::drawCircle(SDL_Surface* surface, int cx, int cy, int radius, Uint32 color) {
    int x = 0, y = radius, d = 1 - radius;

    auto plot8 = [&](int px, int py) {
        setPixel(surface, cx + px, cy + py, color);
        setPixel(surface, cx - px, cy + py, color);
        setPixel(surface, cx + px, cy - py, color);
        setPixel(surface, cx - px, cy - py, color);
        setPixel(surface, cx + py, cy + px, color);
        setPixel(surface, cx - py, cy + px, color);
        setPixel(surface, cx + py, cy - px, color);
        setPixel(surface, cx - py, cy - px, color);
    };

    plot8(x, y);
    while (x < y) {
        x++;
        if (d < 0) {
            d += 2 * x + 1;
        } else {
            y--;
            d += 2 * (x - y) + 1;
        }
        plot8(x, y);
    }
}

// xu = (1-u)^3*x0 + 3*u*(1-u)^2*x1 + 3*u^2*(1-u)*x2 + u^3*x3  (idem para y)
std::vector<Vec2i> Primitives::bezierPoints(Vec2i p0, Vec2i p1, Vec2i p2, Vec2i p3, int steps) {
    std::vector<Vec2i> pts;
    pts.reserve(steps + 1);
    for (int i = 0; i <= steps; i++) {
        double u = (double)i / (double)steps;
        double mu = 1.0 - u;
        double xu = mu * mu * mu * p0.x + 3 * u * mu * mu * p1.x + 3 * u * u * mu * p2.x + u * u * u * p3.x;
        double yu = mu * mu * mu * p0.y + 3 * u * mu * mu * p1.y + 3 * u * u * mu * p2.y + u * u * u * p3.y;
        pts.push_back(Vec2i((int)std::lround(xu), (int)std::lround(yu)));
    }
    return pts;
}

void Primitives::drawBezierCubic(SDL_Surface* surface, Vec2i p0, Vec2i p1, Vec2i p2, Vec2i p3, Uint32 color) {
    std::vector<Vec2i> pts = bezierPoints(p0, p1, p2, p3);
    for (size_t i = 0; i + 1 < pts.size(); i++) {
        drawLine(surface, pts[i].x, pts[i].y, pts[i + 1].x, pts[i + 1].y, color);
    }
}

// Flood-fill iterativo com pilha explícita (stack da STL), como mostrado em aula
void Primitives::floodFill(SDL_Surface* surface, int x, int y, Uint32 fillColor) {
    Uint32 target = getPixel(surface, x, y);
    if (target == fillColor) return;

    std::stack<std::pair<int, int>> pixels;
    pixels.push({x, y});

    while (!pixels.empty()) {
        auto [px, py] = pixels.top();
        pixels.pop();

        if (px < 0 || py < 0 || px >= surface->w || py >= surface->h) continue;
        if (getPixel(surface, px, py) != target) continue;

        setPixel(surface, px, py, fillColor);

        pixels.push({px + 1, py});
        pixels.push({px - 1, py});
        pixels.push({px, py + 1});
        pixels.push({px, py - 1});
    }
}

double Primitives::distPointToSegment(const Vec2i& p, const Vec2i& a, const Vec2i& b) {
    double abx = b.x - a.x, aby = b.y - a.y;
    double apx = p.x - a.x, apy = p.y - a.y;
    double lenSq = abx * abx + aby * aby;
    double t = (lenSq > 0.0) ? (apx * abx + apy * aby) / lenSq : 0.0;
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    double cx = a.x + t * abx, cy = a.y + t * aby;
    double dx = p.x - cx, dy = p.y - cy;
    return std::sqrt(dx * dx + dy * dy);
}

bool Primitives::pointInPolygon(const Vec2i& p, const std::vector<Vec2i>& poly) {
    bool inside = false;
    size_t n = poly.size();
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        int xi = poly[i].x, yi = poly[i].y;
        int xj = poly[j].x, yj = poly[j].y;
        bool intersects = ((yi > p.y) != (yj > p.y)) &&
            (p.x < (double)(xj - xi) * (p.y - yi) / (double)(yj - yi) + xi);
        if (intersects) inside = !inside;
    }
    return inside;
}
