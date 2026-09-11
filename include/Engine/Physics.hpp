#pragma once

#include <Engine/EngineAPI.hpp>

#include <vector>
#include <glm/vec3.hpp>

#include <Engine/Physics/Rigidbody.hpp>


class ZENGINE_API Physics {
public:
  static float fixedDeltaTime;

  static glm::vec3 gravity;


  static void init();
  static void update();


private:
  static float fixedUpdateCountdown;
};