#include <Renderer/Component/Mesh.hpp>
using namespace Renderer;

#include <utility>


//private
uint16_t Mesh::s_nextID = 0;
//private


Mesh::Mesh(GLenum _drawType) : drawType(_drawType), m_id(s_nextID), m_indexCount(0), m_vao(0), m_verticesVBO(0), m_indicesEBO(0), m_instanceCount(0), m_instanceVBO(0) {
  glCreateVertexArrays(1, &m_vao);
  glCreateBuffers(1, &m_verticesVBO);
  glCreateBuffers(1, &m_indicesEBO);

  glVertexArrayVertexBuffer(m_vao, VBO_BINDING_INDEX, m_verticesVBO, 0, sizeof(Vertex));
  glVertexArrayElementBuffer(m_vao, m_indicesEBO);


  glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
  glEnableVertexArrayAttrib(m_vao, 0);
  glVertexArrayAttribBinding(m_vao, 0, 0);

  glVertexArrayAttribFormat(m_vao, 1, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
  glEnableVertexArrayAttrib(m_vao, 1);
  glVertexArrayAttribBinding(m_vao, 1, 0);

  glVertexArrayAttribFormat(m_vao, 2, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
  glEnableVertexArrayAttrib(m_vao, 2);
  glVertexArrayAttribBinding(m_vao, 2, 0);

  glVertexArrayAttribFormat(m_vao, 3, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
  glEnableVertexArrayAttrib(m_vao, 3);
  glVertexArrayAttribBinding(m_vao, 3, 0);

  for(GLuint i = 0; i < 4; i++) {
    glVertexArrayAttribFormat(m_vao, 4 + i, 4, GL_FLOAT, GL_FALSE, i * sizeof(glm::vec4));
    glVertexArrayAttribBinding(m_vao, 4 + i, 1);
  }

  glVertexArrayBindingDivisor(m_vao, 1, 1);


  s_nextID += 1;
}
Mesh::~Mesh() {
  release();
}

Mesh::Mesh(Mesh &&_o) noexcept {
  release();

  drawType = std::exchange(_o.drawType, 0);

  m_id = std::exchange(_o.m_id, 0);

  m_vertices = std::move(_o.m_vertices);
  m_indices  = std::move(_o.m_indices);
  m_indexCount = std::exchange(_o.m_indexCount, 0);

  m_vao = std::exchange(_o.m_vao, 0);
  m_verticesVBO = std::exchange(_o.m_verticesVBO, 0);
  m_indicesEBO = std::exchange(_o.m_indicesEBO, 0);

  m_instances = std::move(_o.m_instances);
  m_instanceCount = std::exchange(_o.m_instanceCount, 0);
  m_instanceVBO = std::exchange(_o.m_instanceVBO, 0);
}
Mesh& Mesh::operator=(Mesh &&_o) noexcept {
  if(this != &_o) {
    release();

    drawType = std::exchange(_o.drawType, 0);

    m_id = std::exchange(_o.m_id, 0);

    m_vertices = std::move(_o.m_vertices);
    m_indices  = std::move(_o.m_indices);
    m_indexCount = std::exchange(_o.m_indexCount, 0);

    m_vao = std::exchange(_o.m_vao, 0);
    m_verticesVBO = std::exchange(_o.m_verticesVBO, 0);
    m_indicesEBO = std::exchange(_o.m_indicesEBO, 0);

    m_instances = std::move(_o.m_instances);
    m_instanceCount = std::exchange(_o.m_instanceCount, 0);
    m_instanceVBO = std::exchange(_o.m_instanceVBO, 0);
  }

  return *this;
}


void Mesh::draw() const noexcept {
  if(isDrawable() == false) return;

  glDrawElements(drawType, m_indexCount, GL_UNSIGNED_INT, nullptr);
}
void Mesh::drawInstanced(GLsizei _instanceCount, GLuint _baseInstance) const noexcept {
  if(isDrawable() == false) return;

  glDrawElementsInstancedBaseInstance(drawType, m_indexCount, GL_UNSIGNED_INT, nullptr, _instanceCount, _baseInstance);
}


void Mesh::setVertices(const std::vector<Vertex> &_vec) {
  m_vertices = std::move(_vec);

  glNamedBufferData(m_verticesVBO, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);
}
void Mesh::setIndices(const std::vector<unsigned int> &_vec) {
  m_indices = std::move(_vec);
  m_indexCount = m_indices.size();

  glNamedBufferData(m_indicesEBO, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
}
void Mesh::setInstances(const std::vector<glm::mat4x4> &_vec) {
  m_instances = std::move(_vec);
  m_instanceCount = m_instances.size();

  if(m_instances.empty() == true) {
    if(m_instanceVBO != 0) {
      glVertexArrayVertexBuffer(m_vao, INSTANCE_VBO_BINDING_INDEX, 0, 0, sizeof(glm::mat4x4));
      glDeleteBuffers(1, &m_instanceVBO);
      m_instanceVBO = 0;

      for(int i = 0; i < 4; i++) glDisableVertexArrayAttrib(m_vao, 4 + i);
    }
    return;
  }

  if(m_instanceVBO == 0) {
    glCreateBuffers(1, &m_instanceVBO);
    glVertexArrayVertexBuffer(m_vao, INSTANCE_VBO_BINDING_INDEX, m_instanceVBO, 0, sizeof(glm::mat4x4));

    for(int i = 0; i < 4; i++) glEnableVertexArrayAttrib(m_vao, 4 + i);
  }

  glNamedBufferData(m_instanceVBO, _vec.size() * sizeof(glm::mat4x4), m_instances.data(), GL_DYNAMIC_DRAW);
}


bool Mesh::isDrawable() const noexcept {
  return (m_indexCount >= 2) && (m_vao != 0) && (m_verticesVBO != 0) && (m_indicesEBO != 0);
}


void Mesh::release() {
  if(m_vao != 0) {
    glDeleteVertexArrays(1, &m_vao);
    m_vao = 0;
  }
  if(m_verticesVBO != 0) {
    glDeleteBuffers(1, &m_verticesVBO);
    m_verticesVBO = 0;
  }
  if(m_indicesEBO != 0) {
    glDeleteBuffers(1, &m_indicesEBO);
    m_indicesEBO = 0;
  }
  if(m_instanceVBO != 0) {
    glDeleteBuffers(1, &m_instanceVBO);
    m_instanceVBO = 0;
  }

  m_indexCount = 0;
  m_instanceCount = 0;
}
