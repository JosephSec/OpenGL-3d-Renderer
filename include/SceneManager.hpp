#pragma once

#include <map>
#include <vector>

#include <Engine/Renderer/MeshRenderer.hpp>
#include <Engine/Renderer/Camera.hpp>
#include <Engine/Transform.hpp>

#include <iostream>


struct GameObject {
public:
  MeshRenderer meshRenderer;
  Transform transform;

  inline void draw() const {
    meshRenderer.draw(transform.getMatrix());
  }
};
class SceneManager {
public:
  static std::map<std::string, Mesh*> sceneMeshes;
  static std::vector<GameObject*> gameObjects;


  static float animationT;
  static GameObject *itemDropObject;
  static GameObject *playerObject;

  static Camera *playerCamera;
  static float playerCameraXRotation;


  static void init();
  static void update();
  static void draw();


  static Mesh *LoadMeshToScene(const std::string &_name);
  static Mesh *LoadMeshToScene(Mesh &_mesh, const std::string &_name);

  static void DrawMeshRenderer(const MeshRenderer &_meshRenderer, const Transform &_transform);
  static void DrawMeshRenderer(const MeshRenderer &_meshRenderer, const std::vector<Transform> &_transforms);


  static void TestPrimitiveMeshLoading();
};