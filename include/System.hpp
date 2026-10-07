#pragma once

#include <string>

#include <SFML/System/Clock.hpp>

#include <SFML/Graphics/RenderWindow.hpp>


class System {
public:
  static std::string PATH;

  static sf::Clock timeClock;
  static float deltaTime;


  static void init();
  static void update();
};