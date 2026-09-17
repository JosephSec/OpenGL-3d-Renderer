#pragma once

#include <GL/glew.h>

#include <Renderer/API.hpp>
#include <Renderer/Mesh.hpp>
#include <Renderer/Shader.hpp>
#include <Renderer/Transform.hpp>


namespace Renderer {
  //see if theres benefits to switching the buffer data type to dynamic in update
  //has any benefits. the thought being that, if update is never called the mesh
  //is probably static. after update is called, the mesh is likely dynamic

  class HORDE3D_API MeshRenderer {
  public:
    MeshRenderer() {}
    MeshRenderer(Mesh *_mesh, Shader *_shader);
    MeshRenderer(const MeshRenderer &_meshRenderer);
    ~MeshRenderer();

    MeshRenderer &operator=(const MeshRenderer &_meshRenderer);


    void update();

    void draw(const glm::mat4x4 &_matrix) const;
    inline void draw(const Transform &_transform) const;
    void draw(const std::vector<glm::mat4x4> &_matrices) const;
    //Attemp gpu instancing

    void setMesh(Mesh *_mesh);
    void setShader(Shader *_shader);


    GLuint vao, vbo, ebo;
    Mesh *mesh = nullptr;
    Shader *shader = nullptr;
    bool backFaceCulling = true;
  };
}
