#pragma once

#include <Renderer/API.hpp>

#include <Renderer/Core/Mesh.hpp>
#include <Renderer/Core/Shader.hpp>


namespace Renderer {
  struct RENDER3D_API RenderCommand {
  public:
    #define ENABLE(_dst, _flag)  _dst |= _flag
    #define DISABLE(_dst, _flag) _dst &= ~_flag


    Mesh *mesh = nullptr;
    Shader *shader = nullptr;
    uint64_t flags = 0;
  };

  class RENDER3D_API MeshRenderer {
  public:
    MeshRenderer(Mesh *_mesh, Shader *_shader) {
      m_renderCommand.mesh = _mesh;
      m_renderCommand.shader = _shader;
    }

  private:
    RenderCommand m_renderCommand;
  };
}