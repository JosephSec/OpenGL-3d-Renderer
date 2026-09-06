#pragma once

#include <Engine/EngineAPI.hpp>

#include <Engine/SceneManager/Transform.hpp>
#include <Engine/Renderer/MeshRenderer.hpp>
#include <Engine/Renderer/Camera.hpp>

#include <map>
#include <functional>


struct ZENGINE_API GameObject {
public:
  GameObject(const MeshRenderer &_meshRenderer, const Transform &_transform = Transform());

  void update() {
    if(m_update != nullptr) m_update(this);
  }
  void draw() const {
    if(m_draw != nullptr) m_draw(this);
  }
  void drawGizmos() const {
    if(m_drawGizmos != nullptr) m_drawGizmos(this);
  }
  

  std::string name = "Empty GameObject";
  bool isActive = true;

  MeshRenderer meshRenderer;
  Transform transform;

  std::function<void(GameObject*)> m_update;
  std::function<void(const GameObject*)> m_draw;
  std::function<void(const GameObject*)> m_drawGizmos;
};
class ZENGINE_API Scene {
public:
  std::map<std::string, Mesh> meshes;
  std::vector<GameObject> gameObjects;

  Camera *camera;


  Scene();

  void update();
  void draw() const;
  void drawGizmos() const;

  Mesh *LoadMeshToScene(const std::string &_name, const Mesh &_mesh);
  Mesh *GetMeshFromScene(const std::string &_name);
};