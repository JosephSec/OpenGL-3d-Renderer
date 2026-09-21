#include <User.hpp>

#include <System.hpp>
#include <Renderer.hpp>

#include <iostream>
#include <sstream>
#include <format>


glm::ivec2 User::Mouse::position;
glm::ivec2 User::Mouse::delta;

void User::Mouse::init() {
  const sf::Vector2i curMousePos = sf::Mouse::getPosition(*Renderer::Core::window);
  position = glm::ivec2(curMousePos.x, curMousePos.y);
  delta = glm::ivec2(0,0);
}
void User::Mouse::update() {
  const sf::Vector2i curMousePos = sf::Mouse::getPosition(*Renderer::Core::window);
  delta = glm::ivec2(curMousePos.x, curMousePos.y) - position;
  position += delta;
}
void User::Mouse::update(const glm::ivec2 &_lockedPosition) {
  const sf::Vector2i curMousePos = sf::Mouse::getPosition(*Renderer::Core::window);
  delta = glm::ivec2(curMousePos.x, curMousePos.y) - position;
  position = _lockedPosition;

  sf::Mouse::setPosition(sf::Vector2i{position.x,position.y}, *Renderer::Core::window);
}


void User::init() {
  Mouse::init();
}

void User::HandleKeyPressed(const sf::Event::KeyPressed *_keyPressed) {
  if(_keyPressed->code == sf::Keyboard::Key::Escape) Renderer::Core::window->close();

  else if(_keyPressed->code == sf::Keyboard::Key::F1) Renderer::Core::ToggleWireframeMode();
  else if(_keyPressed->code == sf::Keyboard::Key::F2) System::ShowMeshNormals = !System::ShowMeshNormals;
  else if(_keyPressed->code == sf::Keyboard::Key::F3) System::ShowLightGizmos = !System::ShowLightGizmos;

  else if(_keyPressed->code == sf::Keyboard::Key::X) {
    std::stringstream ss;

    ss << "Delta Time: " << System::deltaTime << '\n' <<
          "Frames Per Second: " << (1 / System::deltaTime) << '\n';

    std::cout << ss.str();
  }
}
void User::HandleMouseButtonPressed(const sf::Event::MouseButtonPressed *_mouseButtonPressed) {
  if(_mouseButtonPressed->button == sf::Mouse::Button::Left) {
    std::cout << "Left Mouse Button Pressed\n";
  }
}