#pragma once

#include <vector>
#include <Engine/GameObject.hpp>
#include <Engine/Gizmos.hpp>


struct Raycast {
public:
  Vec3 origin;
  Vec3 direction;

  GameObject* object;

  std::vector<Vec3> intersects;
  Vec3 point;


  Raycast(const Vec3& _origin = Vec3zero, const Vec3& _direction = Vec3forward);

  void update();

  inline operator bool() const {
    return object != nullptr;
  }
  inline bool operator==(bool other) const {
    return (object != nullptr) == other;
  }
  inline bool operator!=(bool other) const {
    return (object != nullptr) != other;
  }
};


struct Editorcast {
public:
  Vec3 origin;
  Vec3 direction;

  Gizmo* gizmo;

  Vec3 point;
  unsigned int faceIndex;


  Editorcast(const Vec3& _origin = Vec3zero, const Vec3& _direction = Vec3forward);

  void update();

  inline operator bool() const {
    return gizmo != nullptr;
  }
  inline bool operator==(bool other) const {
    return (gizmo != nullptr) == other;
  }
  inline bool operator!=(bool other) const {
    return (gizmo != nullptr) != other;
  }
};