#include <User.hpp>

#include <System.hpp>
#include <Engine/Renderer.hpp>
#include <Engine/Editor.hpp>
#include <Engine/SceneManager.hpp>

#include <iostream>
#include <sstream>
#include <format>


glm::ivec2 User::Mouse::position;
glm::ivec2 User::Mouse::delta;

void User::Mouse::init() {
  const sf::Vector2i curMousePos = sf::Mouse::getPosition(*Renderer::window);
  position = glm::ivec2(curMousePos.x, curMousePos.y);
  delta = glm::ivec2(0,0);
}
void User::Mouse::update() {
  const sf::Vector2i curMousePos = sf::Mouse::getPosition(*Renderer::window);
  delta = glm::ivec2(curMousePos.x, curMousePos.y) - position;
  
  if(Editor::PlayModePaused == false && Editor::IsActiveCamera() == false) {
    const sf::Vector2i lockedPosition = sf::Vector2i{Renderer::window->getSize()} / 2;

    sf::Mouse::setPosition(lockedPosition, *Renderer::window);
    position = glm::ivec2(lockedPosition.x, lockedPosition.y);
  }
  else position += delta;
}


void User::init() {
  Mouse::init();
}

void User::HandleEvent(const sf::Event &_event) {
  if(const auto *keyPressed = _event.getIf<sf::Event::KeyPressed>()) {
    if(keyPressed->code == sf::Keyboard::Key::Escape) Renderer::window->close();
    else if(keyPressed->code == sf::Keyboard::Key::F1) Editor::TogglePlayModePaused();
    else if(keyPressed->code == sf::Keyboard::Key::F2) Editor::TogglePlayModeFocused();
    else if(keyPressed->code == sf::Keyboard::Key::F3) Editor::TogglePlayModeGizmos();
    
    else if(keyPressed->code == sf::Keyboard::Key::X) {
      std::stringstream ss;

      ss << "Delta Time: " << System::deltaTime << '\n' <<
            "Frames Per Second: " << (1 / System::deltaTime) << '\n';

      // for(const Rigidbody &rigidbody : SceneManager::scene.rigidbodys) {
      //   ss << std::format("Velocity: ({},{},{})\n", rigidbody.velocity.x,rigidbody.velocity.y,rigidbody.velocity.z);
      //   ss << std::format("Position: ({},{},{})\n\n", rigidbody.transform.position.x,rigidbody.transform.position.y,rigidbody.transform.position.z);
      // }

      std::cout << ss.str();
    }
    else if(keyPressed->code == sf::Keyboard::Key::Z) Renderer::ToggleWireframeMode();

    else if(keyPressed->code == sf::Keyboard::Key::Space) {
      Editor::cameras.push_back(new Camera(*Editor::camera));
    }
    
    else if(keyPressed->code == sf::Keyboard::Key::Left) {
      for(int i = 0; i < Editor::cameras.size(); i++) {
        if(Editor::cameras[i] != Renderer::camera) continue;

        if(i == 0) Renderer::SetCamera(Editor::cameras.back());
        else Renderer::SetCamera(Editor::cameras[i - 1]);

        break;
      }
    }
    else if(keyPressed->code == sf::Keyboard::Key::Right) {
      for(int i = 0; i < Editor::cameras.size(); i++) {
        if(Editor::cameras[i] != Renderer::camera) continue;

        if(i == (static_cast<int>(Editor::cameras.size()) - 1)) Renderer::SetCamera(Editor::cameras[0]);
        else Renderer::SetCamera(Editor::cameras[i + 1]);

        break;
      }
    }
  }
}