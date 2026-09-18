#include <Renderer/MeshRenderer.hpp>
using namespace Renderer;

#include <iostream>


MeshRenderer::MeshRenderer(Mesh *_mesh, Shader *_shader) : m_vao(0), m_vbo(0), m_ebo(0), m_instanceVbo(0) {
  if(glGenBuffers == nullptr) return;

  setMesh(_mesh);
  setShader(_shader);
}
MeshRenderer::MeshRenderer(const MeshRenderer &_meshRenderer) : m_vao(0), m_vbo(0), m_ebo(0), m_instanceVbo(0) {
  // std::cout << "copy constructor (Mesh: " << _meshRenderer.m_mesh << " | " <<
  //               "VAO: " << _meshRenderer.m_vao << " | " <<
  //               "VBO: " << _meshRenderer.m_vbo << " | " <<
  //               "EBO: " << _meshRenderer.m_ebo << ")\n";
  //               "InstanceVBO: " << _meshRenderer.m_instanceVbo << ")\n";

  if(m_vbo != 0) {
    glDeleteBuffers(1, &m_vbo);
    m_vbo = 0;
  }
  if(m_ebo != 0) {
    glDeleteBuffers(1, &m_ebo);
    m_ebo = 0;
  }
  if(m_vao != 0) {
    glDeleteVertexArrays(1, &m_vao);
    m_vao = 0;
  }
  if(m_instanceVbo != 0) {
    glDeleteVertexArrays(1, &m_instanceVbo);
    m_instanceVbo = 0;
  }
  m_mesh = nullptr;

  setMesh(_meshRenderer.m_mesh);
  setShader(_meshRenderer.m_shader);
  backFaceCulling = _meshRenderer.backFaceCulling;
}
MeshRenderer::~MeshRenderer() {
  if(m_vbo != 0) glDeleteBuffers(1, &m_vbo);
  if(m_ebo != 0) glDeleteBuffers(1, &m_ebo);
  if(m_instanceVbo != 0) glDeleteBuffers(1, &m_instanceVbo);
  if(m_vao != 0) glDeleteVertexArrays(1, &m_vao);
}

MeshRenderer &MeshRenderer::operator=(const MeshRenderer &_meshRenderer) {
  // std::cout << "assignment operator (Mesh: " << _meshRenderer.m_mesh << " | " <<
  //               "VAO: " << _meshRenderer.m_vao << " | " <<
  //               "VBO: " << _meshRenderer.m_vbo << " | " <<
  //               "EBO: " << _meshRenderer.m_ebo << ")\n";
  //               "InstanceVBO: " << _meshRenderer.m_instanceVbo << ")\n";

  if(m_vbo != 0) {
    glDeleteBuffers(1, &m_vbo);
    m_vbo = 0;
  }
  if(m_ebo != 0) {
    glDeleteBuffers(1, &m_ebo);
    m_ebo = 0;
  }
  if(m_vao != 0) {
    glDeleteVertexArrays(1, &m_vao);
    m_vao = 0;
  }
  if(m_instanceVbo != 0) {
    glDeleteVertexArrays(1, &m_instanceVbo);
    m_instanceVbo = 0;
  }
  m_mesh = nullptr;

  setMesh(_meshRenderer.m_mesh);
  setShader(_meshRenderer.m_shader);
  backFaceCulling = _meshRenderer.backFaceCulling;

  return *this;
}


void MeshRenderer::updateMeshData() {
  if(m_mesh == nullptr || (m_vao + m_vbo + m_ebo) < 3) return;

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, m_mesh->vertices.size() * Mesh::VERTEX_SIZE, m_mesh->vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_mesh->indices.size() * sizeof(unsigned int), m_mesh->indices.data(), GL_STATIC_DRAW);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void MeshRenderer::updateInstancingData(const std::vector<glm::mat4x4> &_matrices) {
  const uint64_t newInstanceCount = _matrices.size();
  const uint64_t dataByteCount = newInstanceCount * sizeof(glm::mat4x4);

  glBindVertexArray(m_vao);

  if(m_instanceVbo == 0) { //instance buffer doesnt exist, create new
    glGenBuffers(1, &m_instanceVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVbo);
    glBufferData(GL_ARRAY_BUFFER, dataByteCount, _matrices.data(), GL_DYNAMIC_DRAW);
    
    std::size_t vec4Size = sizeof(glm::vec4);
    for(unsigned int i = 0; i < 4; i++) {
      unsigned int attribLocation = 4 + i;
      glEnableVertexAttribArray(attribLocation);
      glVertexAttribPointer(attribLocation, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * vec4Size));
      glVertexAttribDivisor(attribLocation, 1); 
    }
  }
  else {    
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVbo);
    glBufferData(GL_ARRAY_BUFFER, dataByteCount, nullptr, GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, dataByteCount, _matrices.data(), GL_DYNAMIC_DRAW);
  }

  m_instanceCount = newInstanceCount;
  glBindVertexArray(0);
}
void MeshRenderer::updateInstancingData(const std::vector<Transform> &_transforms) {
  const size_t transformCount = _transforms.size();

  std::vector<glm::mat4x4> matrices(transformCount);
  for(int i = 0; i < transformCount; i++) matrices[i] = _transforms[i].getMatrix();

  updateInstancingData(matrices);
}
void MeshRenderer::clearInstancingData() {
  if(m_instanceVbo != 0) glDeleteBuffers(1, &m_instanceVbo);

  m_instanceVbo = 0;
  m_instanceCount = 0;
}

void MeshRenderer::draw(const glm::mat4x4 &_matrix) const {
  if(m_mesh == nullptr) return;

  backFaceCulling? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);

  if(m_shader != nullptr) {
    glUseProgram(m_shader->program);
    m_shader->SetUniform("uModel", _matrix);
    m_shader->SetUniform("uIsInstanced", false);
    
  } else glUseProgram(0);

  glBindVertexArray(m_vao);

  int drawType = GL_TRIANGLES;
  switch(m_mesh->type) {
    case MeshType::LitTriangle:
      drawType = GL_TRIANGLES;
      break;
    case MeshType::Lines:
      drawType = GL_LINES;
      break;
  }

  glDrawElements(drawType, m_mesh->indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}
inline void MeshRenderer::draw(const Transform &_transform) const {
  draw(_transform.getMatrix());
}
void MeshRenderer::drawInstanced() const {
  if(m_mesh == nullptr) return;

  backFaceCulling? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);

  if(m_shader != nullptr) {
    glUseProgram(m_shader->program);
    m_shader->SetUniform("uIsInstanced", true);

  } else glUseProgram(0);

  glBindVertexArray(m_vao);

  int drawType = GL_TRIANGLES;
  switch(m_mesh->type) {
    case MeshType::LitTriangle:
      drawType = GL_TRIANGLES;
      break;
    case MeshType::Lines:
      drawType = GL_LINES;
      break;
  }

  glDrawElementsInstanced(drawType, m_mesh->indices.size(), GL_UNSIGNED_INT, 0, m_instanceCount);
  glBindVertexArray(0);
  glUseProgram(0);
}

void MeshRenderer::setMesh(Mesh *_mesh) {
  const Mesh *const oldMesh = m_mesh;
  m_mesh = _mesh;

  if(_mesh == nullptr) {
    if(oldMesh != nullptr) {
      glDeleteBuffers(1, &m_vbo);
      glDeleteBuffers(1, &m_ebo);
      glDeleteVertexArrays(1, &m_vao);
    }

    return;

  } else if(_mesh != nullptr && oldMesh == nullptr) {
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
    glGenVertexArrays(1, &m_vao);
  }


  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, Mesh::VERTEX_SIZE, (void*)offsetof(Mesh::Vertex, position));
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, Mesh::VERTEX_SIZE, (void*)offsetof(Mesh::Vertex, normal));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Mesh::VERTEX_SIZE, (void*)offsetof(Mesh::Vertex, color));
  glEnableVertexAttribArray(2);

  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, Mesh::VERTEX_SIZE, (void*)offsetof(Mesh::Vertex, uv));
  glEnableVertexAttribArray(3);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, m_mesh->vertices.size() * Mesh::VERTEX_SIZE, m_mesh->vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_mesh->indices.size() * sizeof(unsigned int), m_mesh->indices.data(), GL_STATIC_DRAW);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void MeshRenderer::setShader(Shader *_shader) {
  m_shader = _shader;
}
