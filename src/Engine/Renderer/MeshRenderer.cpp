#include <Engine/Renderer/MeshRenderer.hpp>

#include <iostream>


MeshRenderer::MeshRenderer(Mesh *_mesh, Shader *_shader) : vao(0), vbo(0), ebo(0) {
  if(glGenBuffers == nullptr) return;

  setMesh(_mesh);
  setShader(_shader);
}
MeshRenderer::MeshRenderer(const MeshRenderer &_meshRenderer) : vao(0), vbo(0), ebo(0) {
  // std::cout << "copy constructor (Mesh: " << _meshRenderer.mesh << " | " <<
  //               "VAO: " << _meshRenderer.vao << " | " <<
  //               "VBO: " << _meshRenderer.vbo << " | " <<
  //               "EBO: " << _meshRenderer.ebo << ")\n";

  if(vbo != 0) {
    glDeleteBuffers(1, &vbo);
    vbo = 0;
  }
  if(ebo != 0) {
    glDeleteBuffers(1, &ebo);
    ebo = 0;
  }
  if(vao != 0) {
    glDeleteVertexArrays(1, &vao);
    vao = 0;
  }
  mesh = nullptr;

  setMesh(_meshRenderer.mesh);
  setShader(_meshRenderer.shader);
  backFaceCulling = _meshRenderer.backFaceCulling;
}
MeshRenderer::~MeshRenderer() {
  if(vbo != 0) glDeleteBuffers(1, &vbo);
  if(ebo != 0) glDeleteBuffers(1, &ebo);
  if(vao != 0) glDeleteVertexArrays(1, &vao);
}

MeshRenderer &MeshRenderer::operator=(const MeshRenderer &_meshRenderer) {
  // std::cout << "assignment operator (Mesh: " << _meshRenderer.mesh << " | " <<
  //               "VAO: " << _meshRenderer.vao << " | " <<
  //               "VBO: " << _meshRenderer.vbo << " | " <<
  //               "EBO: " << _meshRenderer.ebo << ")\n";

  if(vbo != 0) {
    glDeleteBuffers(1, &vbo);
    vbo = 0;
  }
  if(ebo != 0) {
    glDeleteBuffers(1, &ebo);
    ebo = 0;
  }
  if(vao != 0) {
    glDeleteVertexArrays(1, &vao);
    vao = 0;
  }
  mesh = nullptr;

  setMesh(_meshRenderer.mesh);
  setShader(_meshRenderer.shader);
  backFaceCulling = _meshRenderer.backFaceCulling;

  return *this;
}


void MeshRenderer::update() {
  if(mesh == nullptr || (vao + vbo + ebo) < 3) return;

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * Mesh::VERTEX_SIZE, mesh->vertices.data(), GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(unsigned int), mesh->indices.data(), GL_DYNAMIC_DRAW);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void MeshRenderer::draw(const glm::mat4x4 &_matrix) const {
  if(mesh == nullptr) return;
  
  backFaceCulling? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
  
  if(shader != nullptr) {
    glUseProgram(shader->program);
    shader->SetUniform("model", _matrix);

    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, *mesh->texture);

  } else glUseProgram(0);

  glBindVertexArray(vao);

  int drawType = GL_TRIANGLES;
  switch(mesh->type) {
    case MeshType::LitTriangle:
      drawType = GL_TRIANGLES;
      break;
    case MeshType::Lines:
      drawType = GL_LINES;
      break;
  }

  glDrawElements(drawType, mesh->indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}

void MeshRenderer::setMesh(Mesh *_mesh) {
  const Mesh *const oldMesh = mesh;
  mesh = _mesh;

  if(_mesh == nullptr) {
    if(oldMesh != nullptr) {
      glDeleteBuffers(1, &vbo);
      glDeleteBuffers(1, &ebo);
      glDeleteVertexArrays(1, &vao);
    }

    return;

  } else if(_mesh != nullptr && oldMesh == nullptr) {
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);
  }


  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, Mesh::VERTEX_SIZE, (void*)offsetof(Mesh::Vertex, position));
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Mesh::VERTEX_SIZE, (void*)offsetof(Mesh::Vertex, color));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, Mesh::VERTEX_SIZE, (void*)offsetof(Mesh::Vertex, uv));
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * Mesh::VERTEX_SIZE, mesh->vertices.data(), GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(unsigned int), mesh->indices.data(), GL_DYNAMIC_DRAW);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void MeshRenderer::setShader(Shader *_shader) {
  shader = _shader;

  //Update vao attributes, and vbo data to match shader type
}