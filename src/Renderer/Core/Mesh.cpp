#include <Renderer/Core/Mesh.hpp>
using namespace Renderer;


Mesh::Mesh() : m_vao(0) {}
Mesh::~Mesh() {
  if(m_vao != 0) glDeleteVertexArrays(1, &m_vao);
}

Mesh::Mesh(Mesh &&_o) {
  if(this != &_o) {
    

    m_vertices = std::move(_o.m_vertices);
    m_indices = std::move(_o.m_indices);

    m_vao = std::exchange(_o.m_vao, 0);
    m_vbo = std::move(m_vbo);
    m_ebo = std::move(m_ebo);
  }
}


void Mesh::operator()() noexcept {
  if(m_vao != 0) glDeleteVertexArrays(1, &m_vao);

  glCreateVertexArrays(1, &m_vao);

  m_vbo();
  m_ebo();

  glVertexArrayVertexBuffer(m_vao, 0, m_vbo.id(), 0, sizeof(Vertex));
  glVertexArrayElementBuffer(m_vao, m_ebo.id());


  glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
  glEnableVertexArrayAttrib(m_vao, 0);
  glVertexArrayAttribBinding(m_vao, 0, 0);

  glVertexArrayAttribFormat(m_vao, 1, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
  glEnableVertexArrayAttrib(m_vao, 1);
  glVertexArrayAttribBinding(m_vao, 1, 0);
}

void Mesh::use() const noexcept {
  glBindVertexArray(m_vao);
}
void Mesh::draw() const noexcept {
  glDrawElements(GL_TRIANGLES, m_indices.size(), GL_INDEX_TYPE, nullptr);
}


void Mesh::setVertices(std::vector<Vertex> &&_vertices, GLenum _usage) noexcept {
  m_vertices = std::move(_vertices);
  m_vbo.upload(m_vertices.size() * sizeof(Vertex), m_vertices.data(), _usage);
}
void Mesh::setIndices(std::vector<Index> &&_indices, GLenum _usage) noexcept {
  m_indices = std::move(_indices);
  m_ebo.upload(m_indices.size() * sizeof(Index), m_indices.data(), _usage);
}
