#pragma once

#include <SFML/Window/Event.hpp>

#include <glm/vec2.hpp>


class User {
public:
  class Mouse {
  public:
    static glm::ivec2 position;
    static glm::ivec2 delta;

    static void init();
    static void update();
  };


  static void init();

  static void HandleEvent(const sf::Event &_event);
};