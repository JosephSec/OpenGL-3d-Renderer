#pragma once

#include <cstdint>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/constants.hpp>
#include <SFML/Graphics/Color.hpp>
#include <GL/glew.h>

#include <Renderer/Core/API.hpp>


namespace Renderer {
  class HORDE3D_API Mesh {
  public:
    struct Vertex {
    public:
      glm::vec3 position = {0,0,0};
      glm::vec4 color = {1,1,1,1};
      glm::vec3 normal = {0,0,0};
      glm::vec2 uv = {0,0};
    };
    static constexpr uint16_t VERTEX_SIZE = sizeof(Vertex);


    Mesh(GLenum _type = GL_TRIANGLES);
    ~Mesh();

    template <typename F>
    void editVertices(F &&fn) {
      fn(vertices);
    }


    GLenum type = GL_TRIANGLES;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
  };
}