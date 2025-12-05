#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <System/Vec2.hpp>

using RectI = sf::IntRect;
using RectF = sf::FloatRect;


struct Rect {
public:
  float x,y,w,h;

  inline Rect(float _x = 0, float _y = 0, float _w = 0, float _h = 0) :
    x(_x), y(_y), w(_w), h(_h) {}
  inline Rect(const Vec2& _p, const Vec2& _s = Vec2()) :
    x(_p.x), y(_p.y), w(_s.x), h(_s.y) {}
  template <typename T>
  inline Rect(const sf::Rect<T>& rect) :
    x(rect.left), y(rect.top), w(rect.width), h(rect.height) {}

  inline void setPosition(const Vec2& vec) {
    x = vec.x; y = vec.y;
  }
  inline void setPosition(float _x, float _y) {
    x = _x; y = _y;
  }
  inline void setSize(const Vec2& vec) {
    w = vec.x; h = vec.y;
  }
  inline void setSize(float _w, float _h) {
    w = _w; h = _w;
  }

  inline Vec2 getPosition() const {
    return Vec2(x,y);
  }
  inline Vec2 getSize() const {
    return Vec2(w,h);
  }

  inline bool getInBounds(Vec2 vec) const {
    vec -= getPosition();
    return vec.x >= 0 && vec.x < w  && vec.y >= 0 && vec.y < h;
  }

  inline friend std::ostream& operator<<(std::ostream& os, const Rect& rect) {
    os << "(" << rect.x << ", " << rect.y << ", " << rect.w << ", " << rect.h << ")";
    return os;
  }
  
  inline operator RectI() const {
    return RectI(x,y,w,h);
  }
  inline operator RectF() const {
    return RectF(x,y,w,h);
  }
};