#pragma once

#include <Engine/EngineAPI.hpp>

#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/constants.hpp>
#include <SFML/Graphics/Color.hpp>
#include <GL/glew.h>
#include <vector>


enum MeshType : uint8_t {
  LitTriangle,
  Lines,
};
class ZENGINE_API Mesh {
public:
  struct Vertex {
  public:
    glm::vec3 position = {0,0,0};
    glm::vec4 color = {1,1,1,1};
    glm::vec2 uv = {0,0};
  };
  static constexpr uint16_t VERTEX_SIZE = sizeof(Vertex);
  static constexpr uint16_t VERTEX_FLOAT_COUNT = sizeof(Vertex) / sizeof(float);


  MeshType type = MeshType::LitTriangle;

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  GLuint *texture = nullptr;
  

  Mesh(MeshType _type = MeshType::LitTriangle);
  ~Mesh();
};