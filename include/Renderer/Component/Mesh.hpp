#pragma once

#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <GL/glew.h>

#include <Renderer/Core/API.hpp>


namespace Renderer {
  struct RENDER3D_API Vertex {
  public:
    glm::vec3 position;
    glm::vec4 color;
    glm::vec3 normal;
    glm::vec2 uv;
  };


  class RENDER3D_API Mesh {
  public:
    Mesh(GLenum _drawType = GL_TRIANGLES);
    ~Mesh();

    Mesh(Mesh &&_o) noexcept;
    Mesh& operator=(Mesh &&_o) noexcept;

    Mesh(const Mesh &_o) = delete;
    Mesh& operator=(const Mesh &_o) = delete;


    inline void use() const noexcept {
      glBindVertexArray(m_vao);
    }
    void draw() const noexcept;
    void drawInstanced(GLsizei _instanceCount, GLuint _baseInstance) const noexcept;
    inline void drawInstanced() const noexcept {
      drawInstanced(m_instanceCount, 0);
    }

    template <typename F>
    void editVertices(F &&fn) {
      fn(m_vertices);
      setVertices(m_vertices);
    }


    void setVertices(const std::vector<Vertex> &_vec); //uses std::move, _vec will be made invalid
    void setIndices(const std::vector<unsigned int> &_vec); //uses std::move, _vec will be made invalid
    void setInstances(const std::vector<glm::mat4x4> &_vec); //uses std::move, _vec will be made invalid


    inline uint16_t getID() const noexcept {
      return m_id;
    }

    inline std::vector<Vertex> getVertices() const noexcept {
      return m_vertices;
    }
    inline std::vector<unsigned int> getIndices() const noexcept {
      return m_indices;
    }


    bool isDrawable() const noexcept;


    GLenum drawType = GL_TRIANGLES;


  private:
    static constexpr GLuint VBO_BINDING_INDEX = 0;
    static constexpr GLuint INSTANCE_VBO_BINDING_INDEX = 1;
    static uint16_t s_nextID;

    void release();

    uint16_t m_id;

    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    GLsizei m_indexCount = 0;

    GLuint m_vao = 0;
    GLuint m_verticesVBO = 0;
    GLuint m_indicesEBO  = 0;

    std::vector<glm::mat4x4> m_instances;
    GLsizei m_instanceCount = 0;
    GLuint m_instanceVBO = 0;
  };
}