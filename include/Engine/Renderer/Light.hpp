#pragma once

#include <Engine/API.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>


struct ZENGINE_API Light {
public:
  Light(const glm::vec3 &_position = glm::vec3(0), const glm::vec3 _color = glm::vec3(1), float _radius = 1, float _strength = 1);

  glm::vec3 position = glm::vec3(0);
  glm::vec3 color = glm::vec3(1);
  float radius = 1;
  float strength = 1;
};