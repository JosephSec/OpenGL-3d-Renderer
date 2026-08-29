#include <Engine/Renderer/MeshRenderer.hpp>


MeshRenderer::MeshRenderer(Mesh *_mesh, Shader *_shader) {
  glGenVertexArrays(1, &vao);

  setMesh(_mesh);
  setShader(_shader);
}

void MeshRenderer::draw(const glm::mat4x4 &_matrix) const {
  if(mesh == nullptr) return;

  backFaceCulling? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);

  if(shader != nullptr) {
    glUseProgram(shader->program);
    shader->SetUniform("model", _matrix);
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
}

void MeshRenderer::setMesh(Mesh *_mesh) {
  const Mesh *const oldMesh = mesh;
  mesh = _mesh;

  if(_mesh == nullptr) {
    if(oldMesh != nullptr) {
      glDeleteBuffers(1, &vbo);
      glDeleteBuffers(1, &ebo);
    }

    return;

  } else if(_mesh != nullptr && oldMesh == nullptr) {
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
  }

  mesh = _mesh;


  glBindVertexArray(vao);  
  glBindBuffer(GL_ARRAY_BUFFER, vbo);  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);  

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, Mesh::VERTEX_SIZE, (void*)(0 * sizeof(float)));
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Mesh::VERTEX_SIZE, (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

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