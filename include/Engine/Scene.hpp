#pragma once

#include <Engine/EngineAPI.hpp>

#include <Engine/Transform.hpp>
#include <Engine/Renderer/MeshRenderer.hpp>

#include <map>
#include <functional>


struct ZENGINE_API GameObject {
public:
  GameObject(const MeshRenderer &_meshRenderer, const Transform &_transform);

  void update() {
    if(m_update != nullptr) m_update(this);
  }
  void draw() const {
    if(m_draw != nullptr) m_draw(this);
  }


  MeshRenderer meshRenderer;
  Transform transform;

  std::function<void(GameObject*)> m_update;
  std::function<void(const GameObject*)> m_draw;
};
class ZENGINE_API Scene {
public:
  std::vector<Mesh> meshes;
  std::map<std::string, int> meshIndices;
  std::vector<GameObject> gameObjects;


  Scene();

  void update();
  void draw() const;

  Mesh *LoadMeshToScene(const std::string &_name, const Mesh &_mesh);
  Mesh *GetMeshFromScene(const std::string &_name);
};