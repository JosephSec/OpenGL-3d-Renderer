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


#include <sstream>
#include <glm/mat4x4.hpp>
static std::string PrintMatrix(const glm::mat4x4 &_matrix) {
  std::stringstream ss;

  for(int y = 0; y < 4; y++) {
    ss << "{";
    for(int x = 0; x < 3; x++) ss << _matrix[y][x] << ", ";
    ss << _matrix[y][3] << "}\n";
  }

  return ss.str();
}