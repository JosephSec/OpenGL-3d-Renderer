#include <Engine/SceneManager.hpp>

#include <System.hpp>
#include <User.hpp>

#include <Engine/Renderer.hpp>
#include <Engine/Editor.hpp> 

#include <Engine/SceneManager/PrimitiveMeshGenerator.hpp>


Scene SceneManager::scene;

float SceneManager::playerCameraXRotation = 0;


void SceneManager::init() {}
void SceneManager::update() {
  scene.update();
}
void SceneManager::draw() {
  scene.draw();
}
void SceneManager::end() {}


void SceneManager::DrawMeshRenderer(const MeshRenderer &_meshRenderer, const Transform &_transform) {
  _meshRenderer.draw(_transform.getMatrix());
}
void SceneManager::DrawMeshRenderer(const MeshRenderer &_meshRenderer, const std::vector<Transform> &_transforms) {
  for(const Transform &transform : _transforms) {
    _meshRenderer.draw(transform.getMatrix());
  }
}

void SceneManager::TestPrimitiveMeshLoading() {
  Mesh temp_mesh;
  for(const auto &pair : Editor::primitiveMeshPaths) {
    std::cout << "trying to load mesh (" << pair.first << ")\n";
    if(Editor::LoadMeshPrimitive(temp_mesh, pair.first) == false) {
      std::cout << "failed to load mesh (" << pair.first << ")\n";
      continue;
    }
    std::cout << "mesh loaded successfuly\n";

    if(Editor::SaveMeshPrimitive(temp_mesh, pair.first) == false) {
      std::cout << "failed to save mesh (" << pair.first << ")\n";
      continue;
    }
    std::cout << "mesh saved successfuly\n\n";
  }
}