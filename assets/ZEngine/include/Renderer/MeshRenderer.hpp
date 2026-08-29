#pragma once

#include <GL/glew.h>
#include <Engine/Renderer/Mesh.hpp>
#include <Engine/Renderer/Shader.hpp>


class MeshRenderer {
public:
  MeshRenderer(Mesh *_mesh = nullptr, Shader *_shader = nullptr);

  void draw(const glm::mat4x4 &_matrix) const;
  //Attemp gpu instancing

  void setMesh(Mesh *_mesh);
  void setShader(Shader *_shader);


  GLuint vao, vbo, ebo;
  Mesh *mesh = nullptr;
  Shader *shader = nullptr;
  bool backFaceCulling;
};