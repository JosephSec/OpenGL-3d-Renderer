#include <SceneManager.hpp>

#include <System.hpp>
#include <User.hpp>

#include <Engine/Renderer.hpp>
#include <Editor.hpp> 

#include <Engine/PrimitiveMeshGenerator.hpp>


Scene SceneManager::scene;

bool SceneManager::playMode = false;

Camera *SceneManager::playerCamera = nullptr;
float SceneManager::playerCameraXRotation = 0;


void SceneManager::init() {
  const std::vector<glm::vec4> colors = {
    {1,0,0, 1},
    {0,1,0, 1},
    {0,0,1, 1},
    {1,1,1, 1},
    {0,0,0, 1},
  };

  { //Player  
    playerCamera = new Camera(glm::vec3(0,2,0));
    playerCamera->nearPlane = .2f;
    Editor::playModeCamera = playerCamera;
    Editor::cameras.push_back(playerCamera);
  }
}
void SceneManager::update() {
  if(playMode == true) scene.update();
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