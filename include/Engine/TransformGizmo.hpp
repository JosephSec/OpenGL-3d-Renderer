#pragma once

#include <optional>
#include <Engine/Gizmos.hpp>

#include <Renderer/Mesh.hpp>
#include <System/Transform.hpp>


class TransformGizmo {
public:
  static GameObject* gameObject;

  static Gizmo translationGizmo;
  static std::optional<Vec3> translationAxis;
  static Vec3 grabOrigin;
  static Vec3 grabOffset;


  static void init();
  static void update();
  /**
   * @brief gameObject pointer must not be null
   */
  static void draw();
  static void drawUI();


  static void HandleTranslationPress(uint32_t faceIndex);
  static void HandleTranslationRelease();


private:
  static inline Vec3 GetTranslateDir(const uint32_t faceIndex) {
    switch(faceIndex) {
      case 0: return Vec3forward;
      case 1: return Vec3up;
      case 2: return Vec3right;
    }
    return Vec3zero;
  }
};