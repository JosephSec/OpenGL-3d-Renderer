#pragma once

#include <System/Vec3.hpp>
#include <System/Quaternion.hpp>


static const float identityMatrix[16] = {
  1,0,0,0,
  0,1,0,0,
  0,0,1,0,
  0,0,0,1
};
static inline void SetMatrixVal(float out[16], const float in[16]) {
  for(int i = 0; i < 16; i++) out[i] = in[i];
}


class Transform {
public:
  Vec3 position;
  Quaternion rotation;
  Vec3 scale;

  Transform(const Vec3& _pos = Vec3(), const Quaternion& _rot = Quaternion(), const Vec3& _siz = Vec3one) :
    position(_pos), rotation(_rot), scale(_siz) {}


  inline Vec3 getRotated(const Vec3& vert, bool inverse = false) const {
    if(!inverse) return Quaternion::Normalize(rotation)*Quaternion(vert)*Quaternion::Normalize(Quaternion::Inverse(rotation));
    else return Quaternion::Normalize(Quaternion::Inverse(rotation))*Quaternion(vert)*Quaternion::Normalize(rotation);
  }

  inline void getTranslationMatrix(float out[16]) const {
    SetMatrixVal(out, identityMatrix);

    out[0] = out[5] = out[10] = out[15] = 1;
    out[12] = position.x;
    out[13] = position.y;
    out[14] = position.z;
  }
  inline void getScaleMatrix(float out[16]) const {
    SetMatrixVal(out, identityMatrix);
    
    out[0] = scale.x;
    out[5] = scale.y;
    out[10] = scale.z;
    out[15] = 1;
  }
  inline void getMatrix(float out[16]) const {
    float rotMat[16];
    rotation.getMatrix(rotMat);

    for(int i = 0; i < 3; i++) {
      out[i*4 + 0] = rotMat[i*4 + 0] * scale.x;
      out[i*4 + 1] = rotMat[i*4 + 1] * scale.y;
      out[i*4 + 2] = rotMat[i*4 + 2] * scale.z;
      out[i*4 + 3] = 0;
    }

    out[12] = position.x;
    out[13] = position.y;
    out[14] = position.z;
    out[15] = 1;
  }
};