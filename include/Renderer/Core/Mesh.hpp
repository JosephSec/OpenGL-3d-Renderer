#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <Renderer/API.hpp>

#include <Renderer/Core/GPUBuffer.hpp>

#include <GL/glew.h>


namespace Renderer {
  class RENDER3D_API Mesh {
  public:
    struct RENDER3D_API Vertex {
    public:
      glm::vec3 position;
      glm::vec4 color;
    };

    using Index = uint32_t;
    static constexpr GLenum GL_INDEX_TYPE = GL_UNSIGNED_INT;



    Mesh();
    ~Mesh();

    Mesh(Mesh &&_o);
    Mesh &operator=(Mesh &&_o);

    Mesh(Mesh &) = delete;
    Mesh &operator=(Mesh &) = delete;


    void operator()() noexcept;

    void use() const noexcept;
    void draw() const noexcept;


    void setVertices(std::vector<Vertex> &&_vertices, GLenum _usage = GL_DYNAMIC_DRAW) noexcept;
    void setIndices(std::vector<Index> &&_indices, GLenum _usage = GL_DYNAMIC_DRAW) noexcept;

    inline void setVertices(std::vector<Vertex> &_vertices, GLenum _usage = GL_DYNAMIC_DRAW) noexcept {
      setVertices(std::move(_vertices), _usage);
    }
    inline void setIndices(std::vector<Index> &_indices, GLenum _usage = GL_DYNAMIC_DRAW) noexcept {
      setIndices(std::move(_indices), _usage);
    }


    inline uint64_t GetVRAMFootprint() const noexcept {
      return m_vbo.capacity() + m_ebo.capacity();
    }


  private:
    std::vector<Vertex> m_vertices;
    std::vector<Index> m_indices;

    GLuint m_vao = 0;
    GPUBuffer m_vbo;
    GPUBuffer m_ebo;
  };
}