#pragma once

#include <vector>

#include <System/Vec3.hpp>
#include <SFML/Graphics/Color.hpp>
using Color = sf::Color;

#include <GL/glew.h>


class Line {
public:
  static GLuint VAO;
  static GLuint VBO;


  std::vector<Vec3> vertices;
  std::vector<uint32_t> indecies;
  std::vector<Color> colors;


  Line();

  void update();
  void draw() const;
};