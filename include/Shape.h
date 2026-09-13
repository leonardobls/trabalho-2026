#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "Vec2i.h"

enum class ShapeType { Line, Rectangle, Circle, Bezier, Polygon };

// Classe abstrata para todas as figuras do desenho. Cada figura guarda seus
// próprios "pontos de controle" (points_) — é essa lista, e não os pixels
// desenhados na tela, que representa o modelo do objeto na memória do
// programa. Translação, escala e rotação são implementadas uma única vez
// aqui, operando genericamente sobre points_.
class Shape {
public:
    explicit Shape(Uint32 color) : color(color) {}
    virtual ~Shape() = default;

    virtual ShapeType type() const = 0;
    virtual void draw(SDL_Surface* surface) const = 0;

    // Distância euclidiana mínima entre p e a borda da figura, comparada
    // pelo chamador com o limiar de 0 a 5 pixels pedido no enunciado.
    virtual bool hitTest(const Vec2i& p, double maxDist) const = 0;

    // Figuras fechadas (retângulo, círculo, polígono) podem ser preenchidas;
    // reta e bézier são abertas e não participam da ferramenta de coloração.
    virtual bool isClosed() const = 0;
    virtual bool contains(const Vec2i& p) const { (void)p; return false; }

    // Ponto usado como pivô para escala/rotação; por padrão a média dos
    // pontos de controle, mas círculo sobrescreve para usar o centro.
    virtual Vec2i centroid() const;

    const std::vector<Vec2i>& controlPoints() const { return points_; }
    void setControlPoint(size_t i, const Vec2i& p);

    // Transformações geométricas (fórmulas da aula 06)
    void translate(int dx, int dy);
    void scale(double factor, const Vec2i& pivot);
    void rotate(double degreesCCW, const Vec2i& pivot);

    // Indicação visual de seleção: alças nos pontos de controle + retângulo
    // envolvente.
    void drawHandles(SDL_Surface* surface) const;

    // Se a figura tiver sido colorida pela ferramenta de fill, refaz o
    // preenchimento a partir do centróide (chamado a cada frame).
    void applyFill(SDL_Surface* surface) const;

    Uint32 color;
    bool hasFill = false;
    Uint32 fillColor = 0;

protected:
    std::vector<Vec2i> points_;
};
