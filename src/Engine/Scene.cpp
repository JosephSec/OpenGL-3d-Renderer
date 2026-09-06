#include <Engine/Scene.hpp>

#include <iostream>


GameObject::GameObject(const MeshRenderer &_meshRenderer, const Transform &_transform)
: meshRenderer(_meshRenderer), transform(_transform) {}


Scene::Scene() {}

void Scene::update() {
  for(GameObject &gameObject : gameObjects) gameObject.update();
}
void Scene::draw() const {
  for(const GameObject &gameObject : gameObjects) gameObject.draw();
}

Mesh *Scene::LoadMeshToScene(const std::string &_name, const Mesh &_mesh) {
  if(meshIndices.find(_name) != meshIndices.end()) {
    std::cout << "Scene::LoadMeshToScene: Mesh with name '" << _name << "' already exists in scene. Returning existing mesh." << std::endl;
    return &meshes[meshIndices[_name]];
  }

  meshes.push_back(_mesh);
  meshIndices[_name] = meshes.size() - 1;

  return &meshes.back();
}

Mesh *Scene::GetMeshFromScene(const std::string &_name) {
  if(meshIndices.find(_name) == meshIndices.end()) return nullptr;
  return &meshes[meshIndices[_name]];
}