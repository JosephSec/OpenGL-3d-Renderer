#pragma once

#include <ostream>
#include <cmath>
#include <SFML/System/Vector3.hpp>

using Vector3i = sf::Vector3i;
using Vector3f = sf::Vector3f;


struct Vec3 {
public:
  float x,y,z;


  inline Vec3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}

  inline Vec3 operator-() const {
    return Vec3(-x, -y, -z);
  }

  inline Vec3 operator+(const Vec3& r) const {
    return Vec3(x + r.x, y + r.y, z + r.z);
  }
  inline Vec3 operator-(const Vec3& r) const {
    return Vec3(x - r.x, y - r.y, z - r.z);
  }
  inline Vec3 operator*(float r) const {
    return Vec3(x * r, y * r, z * r);
  }
  inline Vec3 operator/(float r) const {
    return Vec3(x / r, y / r, z / r);
  }

  inline void operator+=(const Vec3& r) {
    x += r.x;
    y += r.y;
    z += r.z;
  }
  inline void operator-=(const Vec3& r) {
    x -= r.x;
    y -= r.y;
    z -= r.z;
  }
  inline void operator*=(float r) {
    x *= r;
    y *= r;
    z *= r;
  }
  inline void operator/=(float r) {
    x /= r;
    y /= r;
    z /= r;
  }

  inline friend std::ostream& operator<<(std::ostream& os, const Vec3& vec) {
    os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return os;
  }

  inline operator Vector3f() const {
    return Vector3f(x,y,z);
  }
  inline operator Vector3i() const {
    return Vector3i(x,y,z);
  }

  inline Vec3 inverse() const  {
    return Vec3(-x, -y, -z);
  }
  //@warning vectors with magnitude of 0 isn't handled will return NaN
  inline Vec3 normalize() const {
    const float mag = magnitude();
    return Vec3(x/mag, y/mag, z/mag);
  }
  inline float magnitude() const {
    return sqrt(x*x + y*y + z*z);
  }

  static inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(
      a.y * b.z - a.z * b.y,
      a.z * b.x - a.x * b.z,
      a.x * b.y - a.y * b.x
    );
  }
  static inline float dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
  }
};

static const Vec3 Vec3zero    (0,0,0);
static const Vec3 Vec3one     (1,1,1);
static const Vec3 Vec3right   (1,0,0);
static const Vec3 Vec3up      (0,1,0);
static const Vec3 Vec3forward (0,0,1);


#include <nlohmann/json.hpp>
using json = nlohmann::json;

inline void to_json(json& j, const Vec3& vec) {
  j = json{
    {"x", vec.x},
    {"y", vec.y},
    {"z", vec.z},
  };
}
inline void from_json(const json& j, Vec3& vec) {
  j.at("x").get_to(vec.x);
  j.at("y").get_to(vec.y);
  j.at("z").get_to(vec.z);
}