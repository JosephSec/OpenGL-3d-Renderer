#pragma once

#include <cstdint>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <SFML/Graphics/Color.hpp>
#include <GL/glew.h>
#include <vector>

#ifndef M_PI
#define M_PI 3.141592653589
#define M_PI_2 M_PI * 2
#endif


enum MeshType : uint8_t {
  LitTriangle,
  Lines,
};
class Mesh {
public:
  struct Vertex {
  public:
    glm::vec3 position;
    glm::vec4 color;
  };
  static constexpr uint16_t VERTEX_SIZE = sizeof(Vertex);
  static constexpr uint16_t VERTEX_FLOAT_COUNT = sizeof(Vertex) / sizeof(float);


  MeshType type = MeshType::LitTriangle;

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  GLuint vao;
  GLuint vbo;
  GLuint ebo;


  Mesh(MeshType _type = MeshType::LitTriangle);
  ~Mesh();
};