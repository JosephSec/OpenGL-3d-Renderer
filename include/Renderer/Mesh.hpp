#pragma once

#include <vector>
#include <string>
using string = std::string;

#include <System/Vec3.hpp>
#include <SFML/Graphics/Color.hpp>
using Color = sf::Color;

#include <GL/glew.h>



class Mesh {
public:
  static GLuint VAO;
  static GLuint VBO;

  static Mesh Triangle;
  static Mesh Circle;
  static Mesh Pyramid;
  static Mesh Quad;
  static Mesh Cube;

  static void InitPrimitives();
  static void SaveAsPrimitive(const string& name, const Mesh& mesh);
  static bool LoadPrimitive(const string& name, Mesh& mesh);


  std::vector<Vec3> vertices;
  std::vector<uint32_t> indecies;
  std::vector<Color> colors;


  Mesh();

  void update();
  void draw() const;
};