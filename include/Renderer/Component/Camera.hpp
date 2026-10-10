#pragma once

#include <Renderer/Core/API.hpp>

#include <Renderer/Component/Transform.hpp>


namespace Renderer {
  class RENDER3D_API Camera {
  public:
    Camera(const glm::vec3 _pos = glm::vec3(0), const glm::quat _rotation = glm::quat())
    : transform(_pos, _rotation, glm::vec3(1)) {}

    inline glm::mat4x4 getViewMatrix() const noexcept {
      return transform.getViewMatrix();
    }
    inline glm::mat4x4 getProjectionMatrix(const glm::ivec2 _windowSize) const noexcept {
      return transform.getProjectionMatrix(_windowSize, fov, nearPlane, farPlane);
    }

    Transform transform;
    float nearPlane = .1;
    float farPlane = 500;
    float fov = 90;
  };
}