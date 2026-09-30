#pragma once

#include <SFML/Window/Event.hpp>

#include <Renderer/Component/Transform.hpp>
#include <Renderer/Component/Camera.hpp>


class EditorCamera {
public:
  static float sensitivity;
  static float slowModeSpeed;
  static float fastModeSpeed;

  static bool flyThroughControl;
  static bool panningControl;

  static Renderer::Camera *camera;


  static void init();
  static void update();
};