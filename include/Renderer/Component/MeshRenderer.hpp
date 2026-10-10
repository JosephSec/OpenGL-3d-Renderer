#pragma once

#include <Renderer/Core/API.hpp>

#include <Renderer/Component/Mesh.hpp>
#include <Renderer/Shader.hpp>
#include <Renderer/Component/Transform.hpp>


namespace Renderer {
  struct RENDER3D_API DrawCommand {
  public:
    uint64_t sortKey = 0;
    
    const Mesh *mesh = nullptr;
    const Shader *shader = nullptr;

    Shader::UniformLocation modelLoc = {-1};
    glm::mat4x4 model = glm::mat4x4(1);
  };

  struct RENDER3D_API Material {
  public:
    glm::vec3 albedo = glm::vec3(1);
    float roughness = 0.5f;
    float metallic  = 0.0f;

    GLuint albedoTexture = 0;
  };

  class RENDER3D_API MeshRenderer {
  public:
    explicit MeshRenderer(Mesh *_mesh = nullptr, const Shader *_shader = nullptr);


    void draw(const glm::mat4x4 &_model = glm::mat4x4(1)) const noexcept;
    inline void draw(const Transform &_transform) const noexcept {
      draw(_transform.getModelMatrix());
    }


    DrawCommand getDrawCommand(const Transform &_transform, uint8_t _pass) const noexcept;


    void setMesh(Mesh *_mesh) noexcept;
    void setShader(const Shader *_shader) noexcept;

    void setInstancingData(const std::vector<glm::mat4x4> &_instances);
    void setInstancingData(const std::vector<Transform> &_instances);


    inline Mesh* getMesh() const noexcept {
      return m_mesh;
    }
    inline const Shader* getShader() const noexcept {
      return m_shader;
    }

    bool isDrawable() const noexcept;


    bool backFaceCulling = true;
    Material material;



  private:
    static uint64_t GetDrawCommandKey(uint8_t _pass, uint16_t _shaderID, uint16_t _meshID, uint32_t _depth);


    Mesh *m_mesh = nullptr;
    
    const Shader *m_shader = nullptr;
    Shader::UniformLocation m_modelUniformLocation;
    Shader::UniformLocation m_isInstancedUniformLocation;

    bool m_isInstanced = false;
  };
}