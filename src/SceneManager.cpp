#include <SceneManager.hpp>


std::vector<GameObject*> SceneManager::gameObjects;


void SceneManager::init() {}
void SceneManager::update() {}
void SceneManager::draw() {
  for(const GameObject *gameObject : gameObjects) gameObject->draw();
}


void SceneManager::DrawMeshRenderer(const MeshRenderer &_meshRenderer, const Transform &_transform) {
  _meshRenderer.draw(_transform.getMatrix());
}
void SceneManager::DrawMeshRenderer(const MeshRenderer &_meshRenderer, const std::vector<Transform> &_transforms) {
  for(const Transform &transform : _transforms) {
    _meshRenderer.draw(transform.getMatrix());
  }
}