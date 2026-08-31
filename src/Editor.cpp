#include <Editor.hpp>
#include <System.hpp>
#include <User.hpp>
#include <Engine/Renderer.hpp>

#include <fstream>

#include <iostream>


Camera *Editor::camera;
float Editor::sensitivity = 1;
float Editor::slowModeSpeed = 10;
float Editor::fastModeSpeed = 20;

MeshRenderer Editor::worldGridRenderer;
Mesh Editor::worldGridMesh;

std::vector<Camera*> Editor::cameras;


void Editor::init() {
  camera = new Camera();
  camera->farPlane = 500;
  camera->fov = 90;

  Renderer::camera = camera;
  cameras.push_back(Renderer::camera);

  Renderer::HandleResize();
  Renderer::UpdateProjectionMatrix();
  Renderer::UpdateViewMatrix();

  worldGridRenderer = MeshRenderer(nullptr, &Renderer::UnlitShader);
  worldGridMesh = Mesh(MeshType::Lines); {
    const int16_t GRID_SIZE = 20;
    
    for(int i = -GRID_SIZE; i <= GRID_SIZE; i++) {
      const float alpha = ((i % 10) == 0)? .5f : .25f;

      worldGridMesh.vertices.push_back(Mesh::Vertex{{-GRID_SIZE,0,i}, {1,1,1, alpha}});
      worldGridMesh.vertices.push_back(Mesh::Vertex{{ GRID_SIZE,0,i}, {1,1,1, alpha}});

      worldGridMesh.vertices.push_back(Mesh::Vertex{{i,0,-GRID_SIZE}, {1,1,1, alpha}});
      worldGridMesh.vertices.push_back(Mesh::Vertex{{i,0, GRID_SIZE}, {1,1,1, alpha}});
    }

    const uint32_t vertexCount = worldGridMesh.vertices.size();
    worldGridMesh.indices.resize(vertexCount);
    for(int i = 0; i < vertexCount; i++) worldGridMesh.indices[i] = i;
    
    worldGridRenderer.setMesh(&worldGridMesh);
  }
}
void Editor::update() {
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
void Editor::draw() {
  worldGridRenderer.draw(glm::mat4x4(1));

  Mesh cubeMesh;
  LoadMeshPrimitive(cubeMesh, std::filesystem::path(System::PATH)/"assets"/"meshes"/"cube.mesh");
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
    cubeRenderer.draw(_camera->transform.getMatrix());
    lineRenderer.draw(_camera->transform.getMatrix());
  }
}
void Editor::end() {
  for(Camera *cameraPtr : cameras) delete cameraPtr;
}

void Editor::SaveMeshPrimitive(const Mesh &_mesh, const std::filesystem::path &_path) {
  if(std::filesystem::exists(_path.parent_path()) == false) {
    std::filesystem::create_directories(_path.parent_path());
  }

  std::ofstream file(_path, std::ios::binary);
  if(!file.is_open()) {
    std::cout << "[Editor Error]: Failed to open file for writing: " << _path.string() << '\n';
    return;
  }

  const uint32_t vertexCount = static_cast<uint32_t>(_mesh.vertices.size());
  const uint32_t indexCount = static_cast<uint32_t>(_mesh.indices.size());

  file.write(reinterpret_cast<const char*>(&vertexCount), sizeof(vertexCount));
  file.write(reinterpret_cast<const char*>(_mesh.vertices.data()), sizeof(Mesh::Vertex) * vertexCount);

  file.write(reinterpret_cast<const char*>(&indexCount), sizeof(indexCount));
  file.write(reinterpret_cast<const char*>(_mesh.indices.data()), sizeof(uint32_t) * indexCount);

  file.close();
}
bool Editor::LoadMeshPrimitive(Mesh &_mesh, const std::filesystem::path &_path) {
  if(std::filesystem::exists(_path) == false) {
    std::cout << "[Editor Error]: File does not exist: " << _path.string() << '\n';
    return false;
  }

  std::ifstream file(_path, std::ios::binary);
  if(!file.is_open()) {
    std::cout << "[Editor Error]: Failed to open file for reading: " << _path.string() << '\n';
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