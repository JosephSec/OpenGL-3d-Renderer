#pragma once

#include <Engine/Renderer/Camera.hpp>
#include <Engine/Renderer/MeshRenderer.hpp>


class Editor {
public:
  static Camera camera;
  static float sensitivity;
  static float slowModeSpeed;
  static float fastModeSpeed;

  static MeshRenderer worldGridRenderer;
  static Mesh worldGridMesh;


  static void init();
  static void update();
  static void draw();
};