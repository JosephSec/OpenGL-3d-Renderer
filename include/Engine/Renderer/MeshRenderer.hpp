#pragma once

#include <GL/glew.h>

#include <Engine/API.hpp>
#include <Engine/Renderer/Mesh.hpp>
#include <Engine/Renderer/Shader.hpp>


namespace Renderer {
  class ZENGINE_API MeshRenderer {
  public:
    MeshRenderer() {}
    MeshRenderer(Mesh *_mesh, Shader *_shader);
    MeshRenderer(const MeshRenderer &_meshRenderer);
    ~MeshRenderer();

    MeshRenderer &operator=(const MeshRenderer &_meshRenderer);


    void update();
    void draw(const glm::mat4x4 &_matrix) const;
    //Attemp gpu instancing

    void setMesh(Mesh *_mesh);
    void setShader(Shader *_shader);


    GLuint vao, vbo, ebo;
    Mesh *mesh = nullptr;
    Shader *shader = nullptr;
    bool backFaceCulling = true;
  };
}
