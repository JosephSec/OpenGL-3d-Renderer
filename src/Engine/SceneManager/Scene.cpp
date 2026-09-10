#include <Engine/SceneManager/Scene.hpp>

#include <iostream>


GameObject::GameObject(const MeshRenderer &_meshRenderer, const Transform &_transform)
: meshRenderer(_meshRenderer), transform(_transform) {}


Scene::Scene() {}
Scene::Scene(const Scene &_scene) {}
Scene::~Scene() {
  delete camera;
}

void Scene::update() {
  for(GameObject &gameObject : gameObjects) {
    if(gameObject.isActive == true) gameObject.update();
  }
  for(Rigidbody &rigidbody : rigidbodys) {
    rigidbody.update();
  }
}
void Scene::draw() const {
  for(const GameObject &gameObject : gameObjects) {
    if(gameObject.isActive == true) gameObject.draw();
  }
  for(const Rigidbody &rigidbody : rigidbodys) {
    rigidbody.draw();
  }
}
void Scene::drawGizmos() const {
  for(const GameObject &gameObject : gameObjects) {
    if(gameObject.isActive == true) gameObject.drawGizmos();
  }
  for(const Rigidbody &rigidbody : rigidbodys) {
    rigidbody.drawGizmos();
  }
}

Mesh *Scene::LoadMeshToScene(const std::string &_name, const Mesh &_mesh) {
  if(meshes.find(_name) != meshes.end()) {
    std::cout << "Scene::LoadMeshToScene: Mesh with name '" << _name << "' already exists in scene. Returning existing mesh." << std::endl;
    return &meshes[_name];
  }

  meshes.insert({_name, _mesh});
  return &meshes[_name];
}
Mesh *Scene::GetMeshFromScene(const std::string &_name) {
  if(meshes.find(_name) == meshes.end()) return nullptr;
  return &meshes[_name];
}