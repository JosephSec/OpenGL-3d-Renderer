#include <User.hpp>

#include <System.hpp>
#include <Renderer/Core.hpp>
#include <TestScene.hpp>
#include <EditorCamera.hpp>

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


void User::init() {
  Cursor::init();
}

void User::HandleFocusLost() {
  Cursor::lockState = CursorLockMode::None;

  EditorCamera::panningControl = false;
  EditorCamera::flyThroughControl = false;
}
void User::HandleKeyPressed(const sf::Event::KeyPressed *_keyPressed) {
  if(_keyPressed->code == sf::Keyboard::Key::Escape) {
    if(Renderer::Core::window->hasFocus() == true) {
      Cursor::lockState = CursorLockMode::None;
    }
  }
  
  else if(_keyPressed->code == sf::Keyboard::Key::X) {
    std::stringstream ss;

    ss << "Delta Time: " << System::deltaTime << '\n' <<
          "Frames Per Second: " << (1 / System::deltaTime) << '\n' <<
          "Particle Count: " << TestScene::particleSystem.getParticleCount() << '\n';

    std::cout << ss.str();
  }

  else if(_keyPressed->code == sf::Keyboard::Key::F1) Renderer::Core::ToggleWireframeMode();
  else if(_keyPressed->code == sf::Keyboard::Key::F2) System::ShowMeshNormals = !System::ShowMeshNormals;
  else if(_keyPressed->code == sf::Keyboard::Key::F3) System::ShowLightGizmos = !System::ShowLightGizmos;
  else if(_keyPressed->code == sf::Keyboard::Key::F4) System::ShowParticleGizmos = !System::ShowParticleGizmos;
}
void User::HandleMouseButtonPressed(const sf::Event::MouseButtonPressed *_mouseButtonPressed) {
  if(Renderer::Core::window->hasFocus() == true) Cursor::lockState = CursorLockMode::Confined;

  if(_mouseButtonPressed->button == sf::Mouse::Button::Right) {
    EditorCamera::flyThroughControl = true;
    EditorCamera::panningControl = false;
  }
  else if(_mouseButtonPressed->button == sf::Mouse::Button::Middle) {
    EditorCamera::flyThroughControl = false;
    EditorCamera::panningControl = true;
  }
}
void User::HandleMouseButtonReleased(const sf::Event::MouseButtonReleased *_mouseButtonReleased) {
  if(_mouseButtonReleased->button == sf::Mouse::Button::Right) {
    EditorCamera::flyThroughControl = false;
  }
  else if(_mouseButtonReleased->button == sf::Mouse::Button::Middle) {
    EditorCamera::panningControl = false;
  }
}