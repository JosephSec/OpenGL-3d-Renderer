#include <Editor.hpp>
#include <System.hpp>
#include <User.hpp>
#include <Engine/Renderer.hpp>


Camera Editor::camera;
float Editor::sensitivity = 1;
float Editor::slowModeSpeed = 1;
float Editor::fastModeSpeed = 5;

MeshRenderer Editor::worldGridRenderer;
Mesh Editor::worldGridMesh;


void Editor::init() {
  Renderer::camera = &camera;

  Renderer::HandleResize();
  Renderer::UpdateProjectionMatrix();
  Renderer::UpdateViewMatrix();

  worldGridRenderer = MeshRenderer(nullptr, &Renderer::UnlitShader);
  worldGridMesh = Mesh(MeshType::Lines); {
    worldGridMesh.vertices = {
      Mesh::Vertex{{-20,0,0}, {1,1,1, .5}},
      Mesh::Vertex{{ 20,0,0}, {1,1,1, .5}},

      Mesh::Vertex{{0,0,-20}, {1,1,1, .5}},
      Mesh::Vertex{{0,0, 20}, {1,1,1, .5}},
    };

    const int16_t GRID_SIZE = 20;
    for(int i = -GRID_SIZE; i <= GRID_SIZE; i++) {
      const float alpha = ((i % 10) == 0)? .5f : .25f;

      worldGridMesh.vertices.push_back(Mesh::Vertex{{-20,0,i}, {1,1,1, alpha}});
      worldGridMesh.vertices.push_back(Mesh::Vertex{{ 20,0,i}, {1,1,1, alpha}});
      
      worldGridMesh.vertices.push_back(Mesh::Vertex{{i,0,-20}, {1,1,1, alpha}});
      worldGridMesh.vertices.push_back(Mesh::Vertex{{i,0, 20}, {1,1,1, alpha}});
    }
    for(int i = 0; i < worldGridMesh.vertices.size(); i++) worldGridMesh.indices.push_back(i);
    
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
      camera.transform.position += (camera.transform.rotation * glm::normalize(moveDir)) * speed;
    }

    if(glm::length(glm::vec2(User::Mouse::delta)) != 0) {
      const glm::vec2 lookDelta = -glm::vec2(User::Mouse::delta) * sensitivity * System::deltaTime;

      const glm::quat pitch = glm::angleAxis(lookDelta.y, glm::vec3(1,0,0));
      const glm::quat yaw = glm::angleAxis(lookDelta.x, glm::vec3(0,1,0));
      camera.transform.rotation = glm::normalize(yaw * camera.transform.rotation * pitch);
    }

    Renderer::UpdateViewMatrix();
  }
}
void Editor::draw() {
  worldGridRenderer.draw(glm::mat4x4(1));
}