#pragma once

#include <Engine/API.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>


struct ZENGINE_API Light {
public:
  Light(const glm::vec3 &_position = glm::vec3(0), const glm::vec4 _color = glm::vec4(1), float _radius = 1);

  glm::vec3 position = glm::vec3(0);
  glm::vec4 color = glm::vec4(1); //alpha channel acts as light strength
  float radius = 1;
};