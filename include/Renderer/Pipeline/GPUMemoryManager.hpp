#pragma once

#include <string>
#include <vector>

#include <Renderer/API.hpp>

#include <Renderer/Core/GPUBuffer.hpp>
#include <Renderer/Core/Mesh.hpp>
#include <Renderer/Core/Shader.hpp>


namespace Renderer {
  class RENDER3D_API GPUMemoryManager {
  public:
    static GPUBuffer *CreateBuffer();
    static Mesh *CreateMesh();
    static Shader *CreateShader(const std::string &_name);

  private:
    static std::vector<GPUBuffer> m_buffers;
    static std::vector<Mesh> m_meshes;
    static std::vector<Shader> m_shaders;
  };
}