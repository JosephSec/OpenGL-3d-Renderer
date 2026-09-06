#pragma once

#include <map>
#include <vector>

#include <Engine/Scene.hpp>
#include <Engine/Renderer/Camera.hpp>

#include <iostream>


class SceneManager {
public:
  static Scene scene;

  static bool playMode;

  static Camera *playerCamera;
  static float playerCameraXRotation;


  static void init();
  static void update();
  static void draw();
  static void end();


  static void DrawMeshRenderer(const MeshRenderer &_meshRenderer, const Transform &_transform);
  static void DrawMeshRenderer(const MeshRenderer &_meshRenderer, const std::vector<Transform> &_transforms);


  static void TestPrimitiveMeshLoading();
};