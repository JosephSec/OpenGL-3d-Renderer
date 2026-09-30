#include <EditorCamera.hpp>

#include <System.hpp>
#include <User.hpp>
#include <Renderer/Core.hpp>


float EditorCamera::sensitivity = 1;
float EditorCamera::slowModeSpeed = 10;
float EditorCamera::fastModeSpeed = 20;

bool EditorCamera::flyThroughControl = false;
bool EditorCamera::panningControl = false;

Renderer::Camera *EditorCamera::camera;


void EditorCamera::init() {
  camera = new Renderer::Camera();
  camera->fov = 90;
  camera->transform.position = glm::vec3(2.5,2,-4.5);
  camera->transform.rotation = glm::angleAxis(glm::radians<float>(145), glm::vec3(0,1,0)) * glm::angleAxis(glm::radians<float>(-30), glm::vec3(1,0,0));
  Renderer::Core::SetCamera(camera);
}
void EditorCamera::update() {
  if(flyThroughControl == true) {
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

    if(glm::length(glm::vec2(User::Cursor::delta)) != 0) {
      const glm::vec2 lookDelta = -glm::vec2(User::Cursor::delta) * sensitivity * System::deltaTime;

      const glm::quat pitch = glm::angleAxis(lookDelta.y, glm::vec3(1,0,0));
      const glm::quat yaw = glm::angleAxis(lookDelta.x, glm::vec3(0,1,0));
      camera->transform.rotation = glm::normalize(yaw * camera->transform.rotation * pitch);
    }

    Renderer::Core::UpdateViewMatrix();
  }
  else if(panningControl == true) {
    if(glm::length(glm::vec2(User::Cursor::delta)) != 0) {
      const glm::vec2 lookDelta = glm::vec2(User::Cursor::delta.x, -User::Cursor::delta.y) * sensitivity * System::deltaTime;
      camera->transform.position -= camera->transform.rotation * glm::vec3(lookDelta.x, lookDelta.y, 0);

      Renderer::Core::UpdateViewMatrix();
    }
  }
}