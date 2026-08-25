#include <SFML/Graphics.hpp>

int main(int argc, char *argv[]) {
  sf::RenderWindow window = sf::RenderWindow(sf::VideoMode({800,600}), "Window");

  while(window.isOpen() == true) {
    while(const auto &eventOpt = window.pollEvent()) {
      const auto &event = *eventOpt;

      if(event.is<sf::Event::Closed>()) window.close();
      else if(const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if(keyPressed->code == sf::Keyboard::Key::Escape) window.close();
      }
    }

    window.clear(sf::Color::Black);
    window.display();
  }

  return 0;
}