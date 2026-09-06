#include <Editor.hpp>
#include <System.hpp>
#include <User.hpp>
#include <Engine/Renderer.hpp>

#include <SceneManager.hpp>
#include <Engine/PrimitiveMeshGenerator.hpp>

#include <fstream>
#include <iostream>


std::filesystem::path Editor::primitiveMeshFolder;
std::map<std::string, std::filesystem::path> Editor::primitiveMeshPaths;

std::vector<Camera*> Editor::cameras;

Camera *Editor::camera;
float Editor::sensitivity = 1;
float Editor::slowModeSpeed = 10;
float Editor::fastModeSpeed = 20;

MeshRenderer Editor::worldGridRenderer;

Camera *Editor::playModeCamera = nullptr;
bool Editor::playMode = false;


void Editor::init() {
  primitiveMeshFolder = std::filesystem::path(System::PATH)/"assets"/"meshes";

  for(const auto& entry : std::filesystem::directory_iterator(primitiveMeshFolder)) {
    if(std::filesystem::is_regular_file(entry.status()) == false) continue;
    primitiveMeshPaths.insert({entry.path().stem().string(), entry.path()});
  }

  camera = new Camera();
  camera->farPlane = 500;
  camera->fov = 90;

  Renderer::camera = camera;
  cameras.push_back(Renderer::camera);

  Renderer::HandleResize();
  Renderer::UpdateProjectionMatrix();
  Renderer::UpdateViewMatrix();


  Mesh tempMesh = GenerateGrid(glm::ivec2(30,30), 1);

  const uint32_t gridMeshHalf = tempMesh.vertices.size() / 2;

  for(int i = 0; i < gridMeshHalf / 2; i++) {
    const float alpha = (((i + 5) % 10) == 0)? .75f : .25f;

    const uint32_t startA = i * 2;
    tempMesh.vertices[startA + 0].color = glm::vec4{alpha,alpha,alpha, 1};
    tempMesh.vertices[startA + 1].color = glm::vec4{alpha,alpha,alpha, 1};

    const uint32_t startB = gridMeshHalf + i * 2;
    tempMesh.vertices[startB + 0].color = glm::vec4{alpha,alpha,alpha, 1};
    tempMesh.vertices[startB + 1].color = glm::vec4{alpha,alpha,alpha, 1};
  }

  Mesh *worldGridMesh = SceneManager::LoadMeshToScene(tempMesh, "World Grid");
  worldGridRenderer = MeshRenderer(worldGridMesh, &Renderer::UnlitShader);
}
void Editor::update() {
  if(playMode == false) {
    if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
      glm::vec3 moveDir = glm::vec3(0);
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) moveDir -= glm::vec3(0,0,1);
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) moveDir -= glm::vec3(1,0,0);
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) moveDir += glm::vec3(0,0,1);
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) moveDir += glm::vec3(1,0,0);
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) moveDir -= glm::vec3(0,1,0);
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) moveDir += glm::vec3(0,1,0);
      if(glm::length(moveDir) != 0) {
        const float speed = System::deltaTime * (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)? fastModeSpeed : slowModeSpeed);
        camera->transform.position += (camera->transform.rotation * glm::normalize(moveDir)) * speed;
      }

      if(glm::length(glm::vec2(User::Mouse::delta)) != 0) {
        const glm::vec2 lookDelta = -glm::vec2(User::Mouse::delta) * sensitivity * System::deltaTime;

        const glm::quat pitch = glm::angleAxis(lookDelta.y, glm::vec3(1,0,0));
        const glm::quat yaw = glm::angleAxis(lookDelta.x, glm::vec3(0,1,0));
        camera->transform.rotation = glm::normalize(yaw * camera->transform.rotation * pitch);
      }

      Renderer::UpdateViewMatrix();
    }
  }
}
void Editor::draw() {
  Renderer::ClearDepthBuffer();
  worldGridRenderer.draw(glm::mat4x4(1));

  Mesh cubeMesh;
  LoadMeshPrimitive(cubeMesh, "cube");
  MeshRenderer cubeRenderer(&cubeMesh, &Renderer::UnlitShader);

  Mesh lineMesh(MeshType::Lines); {
    lineMesh.vertices = {
      Mesh::Vertex{{0,0,0}, {1,0,0, 1}},
      Mesh::Vertex{{0,0,-1}, {1,0,0, 1}},
    };
    lineMesh.indices = {0,1};
  }
  MeshRenderer lineRenderer(&lineMesh, &Renderer::UnlitShader);


  for(const Camera *_camera : cameras) {
    if(_camera == Renderer::camera) continue;

    cubeRenderer.draw(Transform(_camera->transform.position, _camera->transform.rotation, glm::vec3(.25f)).getMatrix());
    lineRenderer.draw(_camera->transform.getMatrix());
  }

  if constexpr(false) {
    if(SceneManager::itemDropObject != nullptr) {
      Mesh wireSphere = GenerateWireSphere(24, 1);
      RandomizeMeshColors(wireSphere, {{0,1,0,1}});
      MeshRenderer wireSphereRenderer(&wireSphere, &Renderer::UnlitShader);
      wireSphereRenderer.draw(Transform(SceneManager::itemDropObject->transform.position).getMatrix());

      Mesh arrowMesh = GeneratePyramid(4, 1, .25f);
      RandomizeMeshColors(arrowMesh, {{0,1,0,1}});
      MeshRenderer arrowRenderer(&arrowMesh, &Renderer::UnlitShader);
      arrowRenderer.draw(Transform(SceneManager::itemDropObject->transform.position + glm::vec3(0,2,0), glm::angleAxis(glm::radians<float>(180), glm::vec3(1,0,0))).getMatrix());
    }
  }
}
void Editor::end() {
  for(Camera *cameraPtr : cameras) delete cameraPtr;
}


void Editor::TogglePlayMode() {
  playMode = !playMode;

  Renderer::SetCamera(playMode? playModeCamera : camera);
}

bool Editor::SaveMeshPrimitive(const Mesh &_mesh, const std::string &_name) {
  const std::filesystem::path _path = primitiveMeshFolder/(_name + ".mesh");

  if(std::filesystem::exists(_path.parent_path()) == false) {
    std::filesystem::create_directories(_path.parent_path());
  }

  std::ofstream file(_path, std::ios::binary);
  if(!file.is_open()) {
    std::cout << "[Editor Error]: Failed to open file for writing: " << _path.string() << '\n';
    return false;
  }

  const uint32_t vertexCount = static_cast<uint32_t>(_mesh.vertices.size());
  const uint32_t indexCount = static_cast<uint32_t>(_mesh.indices.size());

  file.write(reinterpret_cast<const char*>(&vertexCount), sizeof(vertexCount));
  file.write(reinterpret_cast<const char*>(_mesh.vertices.data()), sizeof(Mesh::Vertex) * vertexCount);

  file.write(reinterpret_cast<const char*>(&indexCount), sizeof(indexCount));
  file.write(reinterpret_cast<const char*>(_mesh.indices.data()), sizeof(uint32_t) * indexCount);

  file.close();
  return true;
}
bool Editor::LoadMeshPrimitive(Mesh &_mesh, const std::string &_name) {
  if(primitiveMeshPaths.find(_name) == primitiveMeshPaths.end()) {
    std::cout << "[Editor Error]: Primitive Mesh does not exist: " << _name << '\n';
    return false;
  }

  const std::filesystem::path meshPath = primitiveMeshPaths[_name];

  if(std::filesystem::exists(meshPath) == false) {
    std::cout << "[Editor Error]: File does not exist: " << meshPath.string() << '\n';
    return false;
  }

  std::ifstream file(meshPath, std::ios::binary);
  if(!file.is_open()) {
    std::cout << "[Editor Error]: Failed to open file for reading: " << meshPath.string() << '\n';
    return false;
  }

  uint32_t vertexCount = 0;
  uint32_t indexCount = 0;

  file.read(reinterpret_cast<char*>(&vertexCount), sizeof(vertexCount));
  _mesh.vertices.resize(vertexCount);
  file.read(reinterpret_cast<char*>(_mesh.vertices.data()), sizeof(Mesh::Vertex) * vertexCount);

  file.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
  _mesh.indices.resize(indexCount);
  file.read(reinterpret_cast<char*>(_mesh.indices.data()), sizeof(uint32_t) * indexCount);

  file.close();
  return true;
}

void Editor::RandomizeMeshColors(Mesh &_mesh, const std::vector<glm::vec4> &_colors) {  
  const uint32_t vertexCount = _mesh.vertices.size();
  const uint32_t colorCount = _colors.size();

  for(int i = 0; i < vertexCount; i++) {
    _mesh.vertices[i].color = _colors[i % colorCount];
  }
}