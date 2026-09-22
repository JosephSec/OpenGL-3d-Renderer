#pragma once

#include <GL/glew.h>

#include <Renderer/API.hpp>
#include <Renderer/Mesh.hpp>
#include <Renderer/Shader.hpp>
#include <Renderer/Transform.hpp>


namespace Renderer {
  struct HORDE3D_API Material {
  public:
    glm::vec3 baseColor = {1,1,1};
    glm::vec2 surface = {.5f,.1f}; //x: roughness | y: metallic
  };

  class HORDE3D_API MeshRenderer {
  public:
    MeshRenderer() {}
    MeshRenderer(Mesh *_mesh, Shader *_shader);
    MeshRenderer(const MeshRenderer &_meshRenderer);
    ~MeshRenderer();

    MeshRenderer &operator=(const MeshRenderer &_meshRenderer);


    void updateMeshData();
    void updateInstancingData(const std::vector<glm::mat4x4> &_matrices);
    void updateInstancingData(const std::vector<Transform> &_transforms);
    void clearInstancingData();

    void draw(const glm::mat4x4 &_matrix) const;
    inline void draw(const Transform &_transform) const {
      draw(_transform.getMatrix());
    }
    void drawInstanced() const;

    void setMesh(Mesh *_mesh);
    void setShader(Shader *_shader);


    bool backFaceCulling = true;
    Material material;

  private:
    GLuint m_vao, m_vbo, m_ebo, m_instanceVbo;
    Mesh *m_mesh = nullptr;
    Shader *m_shader = nullptr;
    uint64_t m_instanceCount = 0;
  };
}
