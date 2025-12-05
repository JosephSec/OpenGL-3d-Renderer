#include <User.hpp>
#include <System.hpp>
#include <Renderer.hpp>


std::vector<Keyboard::Key> User::curKeys;
std::vector<Keyboard::Key> User::prevKeys;

std::vector<Mouse::Button> User::curMButtons;
std::vector<Mouse::Button> User::prevMButtons;

Vec2 User::mousePos;


void User::update() {
  mousePos = Mouse::getPosition(*Renderer::window);
  
  prevKeys = curKeys;
  curKeys.clear();

  for(int i = 0; i < Keyboard::KeyCount; i++) {
    const Keyboard::Key key = static_cast<Keyboard::Key>(i);
    if(Keyboard::isKeyPressed(key)) curKeys.emplace_back(key);
  }

  prevMButtons = curMButtons;
  curMButtons.clear();

  for(int i = 0; i < Mouse::ButtonCount; i++) {
    const Mouse::Button button = static_cast<Mouse::Button>(i);
    if(Mouse::isButtonPressed(button)) curMButtons.emplace_back(button);
  }
}

void User::handle(Event event) {
  if(const auto* key = event.getIf<Event::KeyPressed>()) {
    if(key->code == Keyboard::Key::Escape) Renderer::window->close();
    
    // else if(key->code == Keyboard::Key::X)
  }

  // else if(const auto* mouseButton = event.getIf<Event::MouseButtonPressed>()) {
  //        if(mouseButton->button == Mouse::Button::Left)
  //   else if(mouseButton->button == Mouse::Button::Right)
  // }
}