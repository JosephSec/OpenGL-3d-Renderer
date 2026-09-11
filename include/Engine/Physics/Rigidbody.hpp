#pragma once

#include <Engine/EngineAPI.hpp>

#include <glm/vec3.hpp>
#include <Engine/SceneManager/Transform.hpp>


class ZENGINE_API Rigidbody {
public:
  Rigidbody(
    const Transform &_transform,
    float _mass = 1,
    const glm::vec3 _initialVelocity = glm::vec3(0),
    float _radius = .5
  );

  void fixedUpdate();
  void draw() const;
  void drawGizmos() const;

  Transform transform;
  glm::vec3 velocity = glm::vec3(0);
  float mass = 1;
  float radius = .5;
  float elasticity = 1;
};