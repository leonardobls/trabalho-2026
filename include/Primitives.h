#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "Vec2i.h"

// Primitivas gráficas de baixo nível: todas as formas do trabalho são
// desenhadas em cima destas funções (nenhuma usa SDL_RenderDrawLine ou
// equivalente pronto do SDL — os algoritmos são implementados aqui).
class Primitives {
public:
    static void setPixel(SDL_Surface* surface, int x, int y, Uint32 color);
    static Uint32 getPixel(SDL_Surface* surface, int x, int y);
    static Uint32 rgbToUint32(SDL_Surface* surface, Uint8 r, Uint8 g, Uint8 b);

    // Reta: algoritmo de Bresenham
    static void drawLine(SDL_Surface* surface, int x1, int y1, int x2, int y2, Uint32 color);

    // Círculo: algoritmo do ponto médio de Bresenham (8 octantes)
    static void drawCircle(SDL_Surface* surface, int cx, int cy, int radius, Uint32 color);

    // Curva de Bézier cúbica (4 pontos de controle), conforme fórmula da aula
    static std::vector<Vec2i> bezierPoints(Vec2i p0, Vec2i p1, Vec2i p2, Vec2i p3, int steps = 200);
    static void drawBezierCubic(SDL_Surface* surface, Vec2i p0, Vec2i p1, Vec2i p2, Vec2i p3, Uint32 color);

    // Preenchimento: flood-fill com pilha (evita stack overflow da versão recursiva)
    static void floodFill(SDL_Surface* surface, int x, int y, Uint32 fillColor);

    // Distância euclidiana de um ponto a um segmento (usada na seleção por clique)
    static double distPointToSegment(const Vec2i& p, const Vec2i& a, const Vec2i& b);

    // Teste ponto-dentro-de-polígono (ray casting), usado pela ferramenta de coloração
    static bool pointInPolygon(const Vec2i& p, const std::vector<Vec2i>& poly);
};
