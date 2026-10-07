#pragma once

#include <Renderer/Transform.hpp>


namespace Renderer {
  class Camera {
  public:
    Camera(const glm::vec3 _pos = glm::vec3(0), const glm::quat _rotation = glm::quat());

    glm::mat4x4 getViewMatrix() const;
    glm::mat4x4 getProjectionMatrix(const glm::vec2 _windowSize) const;

    Transform transform;
    float nearPlane = .1;
    float farPlane = 500;
    float fov = 90;
  };
}