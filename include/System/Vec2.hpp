#pragma once

#include <ostream>
#include <cmath>
#include <SFML/System/Vector2.hpp>

using Vector2i = sf::Vector2i;
using Vector2u = sf::Vector2u;
using Vector2f = sf::Vector2f;


struct Vec2 {
public:
  float x,y;


  inline Vec2(float _x = 0, float _y = 0) : x(_x), y(_y) {}

  template <typename T>
  inline Vec2(const sf::Vector2<T>& vec) : x(vec.x), y(vec.y) {}

  inline friend Vec2 operator+(const Vec2& l, const Vec2& r) {
    return Vec2(l.x + r.x, l.y + r.y);
  }
  inline friend Vec2 operator-(const Vec2& l, const Vec2& r) {
    return Vec2(l.x - r.x, l.y - r.y);
  }
  inline friend Vec2 operator*(const Vec2& l, float r) {
    return Vec2(l.x * r, l.y * r);
  }
  inline friend Vec2 operator/(const Vec2& l, float r) {
    return Vec2(l.x / r, l.y / r);
  }

  inline friend void operator+=(Vec2& l, const Vec2& r) {
    l.x += r.x;
    l.y += r.y;
  }
  inline friend void operator-=(Vec2& l, const Vec2& r) {
    l.x -= r.x;
    l.y -= r.y;
  }
  inline void operator*=(float r) {
    x *= r;
    y *= r;
  }
  inline void operator/=(float r) {
    x /= r;
    y /= r;
  }

  inline friend Vec2 operator-(const Vec2& l) {
    return Vec2(-l.x,-l.y);
  }
  inline friend bool operator==(const Vec2& l, const Vec2& r) {
    return l.x == r.x && l.y == r.y;
  }
  inline friend bool operator!=(const Vec2& l, const Vec2& r) {
    return l.x != r.x || l.y != r.y;
  }

  inline friend std::ostream& operator<<(std::ostream& os, const Vec2& vec) {
    os << "(" << vec.x << ", " << vec.y << ")";
    return os;
  }

  inline operator Vector2f() const {
    return Vector2f(x,y);
  }
  inline operator Vector2i() const {
    return Vector2i(x,y);
  }
  inline operator Vector2u() const {
    return Vector2u(x,y);
  }

  inline Vec2 inverse() const {
    return Vec2(-x, -y);
  }
  inline Vec2 normalize() const {
    const float length = sqrt(x*x+y*y);
    if(length == 0) return Vec2();
    return Vec2(x/length, y/length);
  }
  inline float magnitude() const {
    return sqrt(x*x+y*y);
  }

  static inline float distance(Vec2 a, const Vec2& b) {
    return (b-a).magnitude();
  }
};

static const Vec2 Vec2zero(0,0);
static const Vec2 Vec2right(1,0);
static const Vec2 Vec2up(0,1);
static const Vec2 Vec2one(1,1);