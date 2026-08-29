#include <Engine/Renderer/Mesh.hpp>

#include <cstring>


Mesh::Mesh(MeshType _type) : type(_type) {
  if(glGenBuffers == nullptr) return;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);  
  glBindBuffer(GL_ARRAY_BUFFER, vbo);  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);  

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (void*)(0 * sizeof(float)));
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
Mesh::~Mesh() {
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
}