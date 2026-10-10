#include <User.hpp>

#include <System.hpp>
#include <Renderer/Core.hpp>

#include <iostream>
#include <sstream>


glm::ivec2 User::Cursor::position;
glm::ivec2 User::Cursor::delta;

User::CursorLockMode User::Cursor::lockState = User::CursorLockMode::None;


void User::Cursor::init() {
  const sf::Vector2i curMousePos = sf::Mouse::getPosition(*Renderer::Core::window);
  position = glm::ivec2(curMousePos.x, curMousePos.y);
  delta = glm::ivec2(0,0);

  lockState = CursorLockMode::Confined;
}
void User::Cursor::update() {
  const sf::Vector2i curMousePos = sf::Mouse::getPosition(*Renderer::Core::window);
  delta = glm::ivec2(curMousePos.x, curMousePos.y) - position;
  
  switch(lockState) {
    case CursorLockMode::None: {
      position += delta;
      break;
    }

    case CursorLockMode::Locked: {
      const sf::Vector2i windowCenter = sf::Vector2i{Renderer::Core::windowSize.x / 2, Renderer::Core::windowSize.y / 2};
      sf::Mouse::setPosition(windowCenter, *Renderer::Core::window);
      position = glm::ivec2(windowCenter.x, windowCenter.y);

      break;
    }

    case CursorLockMode::Confined: {
      const sf::Vector2i clamped = sf::Vector2i{
        std::min(Renderer::Core::windowSize.x - 1, std::max(0, curMousePos.x)),
        std::min(Renderer::Core::windowSize.y - 1, std::max(0, curMousePos.y))
      };
      sf::Mouse::setPosition(clamped, *Renderer::Core::window);
      position = glm::ivec2(clamped.x, clamped.y);

      break;
    }
  }
}


bool User::panningControl = false;
bool User::flyThroughControl = false;


void User::init() {
  Cursor::init();
}
void User::update() {
  User::Cursor::update();

  Renderer::Camera *camera = Renderer::Core::camera;

  if(flyThroughControl == true) {
    glm::vec3 moveDir = glm::vec3(0);
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) moveDir -= glm::vec3(0,0,1);
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) moveDir -= glm::vec3(1,0,0);
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) moveDir += glm::vec3(0,0,1);
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) moveDir += glm::vec3(1,0,0);
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) moveDir -= glm::vec3(0,1,0);
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) moveDir += glm::vec3(0,1,0);
    if(glm::length(moveDir) != 0) {
      camera->transform.position += (camera->transform.rotation * glm::normalize(moveDir)) * 8.0f * System::deltaTime;
    }

    if(glm::length(glm::vec2(User::Cursor::delta)) != 0) {
      const glm::vec2 lookDelta = -glm::vec2(User::Cursor::delta) * System::deltaTime;

      const glm::quat pitch = glm::angleAxis(lookDelta.y, glm::vec3(1,0,0));
      const glm::quat yaw = glm::angleAxis(lookDelta.x, glm::vec3(0,1,0));
      camera->transform.rotation = glm::normalize(yaw * camera->transform.rotation * pitch);
    }

    Renderer::Core::UpdateViewMatrix();
  }
  else if(panningControl == true) {
    if(glm::length(glm::vec2(User::Cursor::delta)) != 0) {
      const glm::vec2 lookDelta = glm::vec2(User::Cursor::delta.x, -User::Cursor::delta.y) * System::deltaTime;
      camera->transform.position -= camera->transform.rotation * glm::vec3(lookDelta.x, lookDelta.y, 0);

      Renderer::Core::UpdateViewMatrix();
    }
  }
}

void User::HandleFocusLost() {
  Cursor::lockState = CursorLockMode::None;

  panningControl = false;
  flyThroughControl = false;
}
void User::HandleKeyPressed(const sf::Event::KeyPressed *_keyPressed) {
  if(_keyPressed->code == sf::Keyboard::Key::Escape) {
    if(Renderer::Core::window->hasFocus() == true) {
      Cursor::lockState = CursorLockMode::None;
    }
  }

  else if(_keyPressed->code == sf::Keyboard::Key::F1) Renderer::Core::ToggleWireframeMode();
  
  else if(_keyPressed->code == sf::Keyboard::Key::X) {
    std::stringstream ss;

    ss << "Delta Time: " << System::deltaTime << '\n' <<
          "Frames Per Second: " << (1 / System::deltaTime) << '\n';

    std::cout << ss.str();
  }
}
void User::HandleMouseButtonPressed(const sf::Event::MouseButtonPressed *_mouseButtonPressed) {
  if(Renderer::Core::window->hasFocus() == true) Cursor::lockState = CursorLockMode::Confined;

  if(_mouseButtonPressed->button == sf::Mouse::Button::Right) {
    flyThroughControl = true;
    panningControl = false;
  }
  else if(_mouseButtonPressed->button == sf::Mouse::Button::Middle) {
    flyThroughControl = false;
    panningControl = true;
  }
}
void User::HandleMouseButtonReleased(const sf::Event::MouseButtonReleased *_mouseButtonReleased) {
  if(_mouseButtonReleased->button == sf::Mouse::Button::Right) {
    flyThroughControl = false;
  }
  else if(_mouseButtonReleased->button == sf::Mouse::Button::Middle) {
    panningControl = false;
  }
}