#pragma once

#include <Engine/Transform.hpp>


class Camera {
public:
  Camera(const glm::vec3 _pos = glm::vec3(0), const glm::quat _rotation = glm::quat());

  glm::mat4x4 getViewMatrix() const;
  glm::mat4x4 getProjectionMatrix(const glm::vec2 _windowSize) const;

  Transform transform;
  float nearPlane = .1;
  float farPlane = 50;
  float fov = 60;
  // glm::vec4 viewPort = glm::vec4(0,0,1,1);
};