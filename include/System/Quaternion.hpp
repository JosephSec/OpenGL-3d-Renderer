#pragma once

#include <ostream>
#include <sstream>
#include <System/Vec3.hpp>


static constexpr float deg2rad = M_PI/180;
static constexpr float rad2deg = 180/M_PI;

struct Quaternion {
public:
  float x, y, z, w;

  inline Quaternion(float x = 0, float y = 0, float z = 0, float w = 1) : x(x), y(y), z(z), w(w) {}
  inline Quaternion(const Vec3& vec) : x(vec.x), y(vec.y), z(vec.z), w(0) {}


  static inline Quaternion Euler(float anglex = 0, float angley = 0, float anglez = 0) {
    const float x = anglex*deg2rad / 2;
    const float y = angley*deg2rad / 2;
    const float z = anglez*deg2rad / 2;

    const float sx = sin(x), cx = cos(x);
    const float sy = sin(y), cy = cos(y);
    const float sz = sin(z), cz = cos(z);

    return Quaternion(
      sx*cy*cz - cx*sy*sz,
      cx*sy*cz + sx*cy*sz,
      cx*cy*sz - sx*sy*cz,
      cx*cy*cz + sx*sy*sz);
  }
  static inline Quaternion Euler(const Vec3& rotation) {
    return Euler(rotation.x, rotation.y, rotation.z);
  }

  static inline Quaternion AngleAxis(float angle, Vec3 axis) {
    const float sa = sin(angle/2);

    axis = axis.normalize();
    return Quaternion(
      sa*axis.x,
      sa*axis.y,
      sa*axis.z,

      cos(angle/2)
    );
  }

  static inline Quaternion Inverse(const Quaternion& quaternion) {
    return Quaternion(-quaternion.x, -quaternion.y, -quaternion.z, quaternion.w);
  }
  static inline Quaternion Normalize(const Quaternion& q) {
    float magnitude = sqrt(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
    if(magnitude > 0.00001f)
      return Quaternion(q.x/magnitude, q.y/magnitude, q.z/magnitude, q.w/magnitude);
    return Quaternion();
  }


  inline Quaternion operator*(const Quaternion& q) const {
    return Quaternion(
      w*q.x + x*q.w + y*q.z - z*q.y,
      w*q.y - x*q.z + y*q.w + z*q.x,
      w*q.z + x*q.y - y*q.x + z*q.w,
      w*q.w - x*q.x - y*q.y - z*q.z);
  }

  inline void operator+=(const Vec3& vec) {
    *this =
      AngleAxis(vec.x, Vec3{1,0,0}) *
      AngleAxis(vec.y, Vec3{0,1,0}) *
      AngleAxis(vec.z, Vec3{0,0,1}) * *this;
  }
  inline void operator-=(const Vec3& vec) {
    *this =
      AngleAxis(-vec.x, Vec3{1,0,0}) *
      AngleAxis(-vec.y, Vec3{0,1,0}) *
      AngleAxis(-vec.z, Vec3{0,0,1}) * *this;
  }
  inline void operator*=(const Quaternion& q) {
    x = w*q.x + x*q.w + y*q.z - z*q.y;
    y = w*q.y - x*q.z + y*q.w + z*q.x;
    z = w*q.z + x*q.y - y*q.x + z*q.w;
    w = w*q.w - x*q.x - y*q.y - z*q.z;
  }

  inline friend std::ostream& operator<<(std::ostream& os, const Quaternion& rot) {
    return os << rot.getEuler();
    // os << "(" << rot.x << ", " << rot.y << ", " << rot.z << ", " << rot.w << ")";
    // return os;
  }

  operator Vec3() const {
    return Vec3{x,y,z};
  }

  inline Vec3 getEuler() const {
    return Vec3{
      atan2f(2*(w*x + y*z), 1 - 2*(x*x + y*y)) * rad2deg,
      asinf (2*(w*y - z*x)) * rad2deg,
      atan2f(2*(w*z + x*y), 1 - 2*(y*y + z*z)) * rad2deg
    };
  }
  inline void getMatrix(float out[16]) const {    
    out[0]  = 1 - 2*y*y - 2*z*z;
    out[1]  = 2*x*y + 2*w*z;
    out[2]  = 2*x*z - 2*w*y;
    out[3]  = 0;

    out[4]  = 2*x*y - 2*w*z;
    out[5]  = 1 - 2*x*x - 2*z*z;
    out[6]  = 2*y*z + 2*w*x;
    out[7]  = 0;

    out[8]  = 2*x*z + 2*w*y;
    out[9]  = 2*y*z - 2*w*x;
    out[10] = 1 - 2*x*x - 2*y*y;
    out[11] = 0;

    out[12] = out[13] = out[14] = 0;
    out[15] = 1;
  }
};