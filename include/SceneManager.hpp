#pragma once

#include <vector>

#include <Engine/Renderer/MeshRenderer.hpp>
#include <Engine/Transform.hpp>


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
  static std::vector<GameObject*> gameObjects;


  static void init();
  static void update();
  static void draw();


  static void DrawMeshRenderer(const MeshRenderer &_meshRenderer, const Transform &_transform);
  static void DrawMeshRenderer(const MeshRenderer &_meshRenderer, const std::vector<Transform> &_transforms);
};