#include <SceneManager.hpp>

#include <System.hpp>
#include <User.hpp>

#include <Engine/Renderer.hpp>
#include <Editor.hpp> 

#include <Engine/PrimitiveMeshGenerator.hpp>


std::map<std::string, Mesh*> SceneManager::sceneMeshes;
std::vector<GameObject*> SceneManager::gameObjects;

float SceneManager::animationT = 0;
GameObject *SceneManager::itemDropObject = nullptr;
GameObject *SceneManager::playerObject = nullptr;

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

  for(int i = 0; i < Editor::primitiveMeshPaths.size(); i++) {
    Mesh *mesh = LoadMeshToScene(std::next(Editor::primitiveMeshPaths.begin(), i)->first);

    MeshRenderer meshRenderer(mesh, &Renderer::UnlitShader);
    gameObjects.push_back(new GameObject{meshRenderer, Transform(glm::vec3(i * 2, 0, 0))});
  }

  
  { //Ground
    Mesh *mesh = CopySceneMesh("quad", "ground");
    Editor::RandomizeMeshColors(*mesh, {{0,0,0, 1}});

    MeshRenderer groundMeshRenderer(mesh, &Renderer::UnlitShader);
    gameObjects.push_back(new GameObject{groundMeshRenderer, Transform(glm::vec3(0), glm::rotate(glm::mat4x4(1), glm::radians<float>(-90), glm::vec3(1,0,0)), glm::vec3(30,30,1))});
  }

  { //Item Drop
    Mesh *mesh = CopySceneMesh("cube", "itemDrop");
    Editor::RandomizeMeshColors(*mesh, colors);

    MeshRenderer groundMeshRenderer(mesh, &Renderer::UnlitShader);
    itemDropObject = new GameObject{groundMeshRenderer, Transform(glm::vec3(0,1,-5), glm::quat(), glm::vec3(.5f))};
    gameObjects.push_back(itemDropObject);
  }

  { //Player
    Mesh playerBodyMesh = GenerateCylinder(12, 2, .5f);
    Editor::RandomizeMeshColors(playerBodyMesh, {{1,0,0,1}});
    Mesh *mesh = LoadMeshToScene(playerBodyMesh, "playerBody");

    MeshRenderer playerRenderer(mesh, &Renderer::UnlitShader);
    playerObject = new GameObject{playerRenderer, Transform(glm::vec3(0,1,0))};
    gameObjects.push_back(playerObject);
  
    playerCamera = new Camera(glm::vec3(0,2,0));
    Editor::playModeCamera = playerCamera;
    Editor::cameras.push_back(playerCamera);
  }
}
void SceneManager::update() {
  animationT += System::deltaTime * 100;

  if(Editor::playMode) {
    if(true && itemDropObject != nullptr) { //Item Drop Movement
      itemDropObject->transform.position = glm::vec3(0,1 + glm::sin(glm::radians<float>(animationT)) * .25,-5);
      itemDropObject->transform.rotation = glm::angleAxis(glm::radians<float>(animationT), glm::vec3(0,1,0));
    }

    if(true) { //Player Movement
      glm::vec3 moveDir = glm::vec3(0);
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) moveDir -= glm::vec3(0,0,1);
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) moveDir -= glm::vec3(1,0,0);
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) moveDir += glm::vec3(0,0,1);
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) moveDir += glm::vec3(1,0,0);
      if(glm::length(moveDir) != 0) {
        const float speed = System::deltaTime * (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)? 5 : 2);
        playerObject->transform.position += (playerObject->transform.rotation * glm::normalize(moveDir)) * speed;
        playerCamera->transform.position = playerObject->transform.position + glm::vec3(0,1,0);

        Renderer::UpdateViewMatrix();
      }

      if(glm::length(glm::vec2(User::Mouse::delta)) != 0) {
        const glm::vec2 lookDelta = -glm::vec2(User::Mouse::delta) * Editor::sensitivity * System::deltaTime;
        playerCameraXRotation = glm::clamp(playerCameraXRotation + lookDelta.y, glm::radians<float>(-90), glm::radians<float>(90));

        const glm::quat yaw = glm::angleAxis(lookDelta.x, glm::vec3(0,1,0));
        const glm::quat pitch = glm::angleAxis(playerCameraXRotation, glm::vec3(1,0,0));

        playerObject->transform.rotation = glm::normalize(playerObject->transform.rotation * yaw);
        playerCamera->transform.rotation = glm::normalize(playerObject->transform.rotation * pitch);

        Renderer::UpdateViewMatrix();
      }


      if(itemDropObject != nullptr && glm::length(playerObject->transform.position - itemDropObject->transform.position) <= 1) {
        for(int i = 0; i < gameObjects.size(); i++) {
          if(gameObjects[i] != itemDropObject) continue;

          std::swap(gameObjects[i], gameObjects.back());
          gameObjects.pop_back();
          delete itemDropObject;
          break;
        }
      }
    }
  }
}
void SceneManager::draw() {
  for(const GameObject *gameObject : gameObjects) gameObject->draw();
}
void SceneManager::end() {
  for(const auto &[name, meshPtr] : sceneMeshes) delete meshPtr;
  for(GameObject *gameObject : gameObjects) delete gameObject;
}


Mesh *SceneManager::LoadMeshToScene(const std::string &_name) {
  Mesh *mesh = new Mesh();
  if(Editor::LoadMeshPrimitive(*mesh, _name) == false) {
    std::cout << "[SceneManager Error]: Failed to load mesh: " << _name << '\n';
    delete mesh;
    return nullptr;
  }

  sceneMeshes[_name] = mesh;
  return mesh;
}
Mesh *SceneManager::LoadMeshToScene(Mesh &_mesh, const std::string &_name) {
  sceneMeshes[_name] = new Mesh(_mesh);
  return sceneMeshes[_name];
}

Mesh *SceneManager::GetSceneMesh(const std::string &_name) {
  const auto &it = sceneMeshes.find(_name);
  if(it != sceneMeshes.end()) return it->second;

  std::cout << "[SceneManager Error]: GetSceneMesh(" << _name << ") returned nullptr, mush was not found\n";
  return nullptr;
}
Mesh *SceneManager::CopySceneMesh(const std::string &_name, const std::string &_copyName) {
  Mesh *mesh = GetSceneMesh(_name);
  if(mesh == nullptr) return nullptr;

  return LoadMeshToScene(*mesh, _copyName);
}


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