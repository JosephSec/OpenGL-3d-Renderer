#pragma once

#include <Renderer/Core/API.hpp>

#include <Renderer/Core/Shader.hpp>
#include <Renderer/Core/Cubemap.hpp>


namespace Renderer {
  class RENDER3D_API Skybox {
  public:
    Skybox(int _size = 256, GLenum _internalFormat = GL_RGBA8, int _levels = 1);
    ~Skybox();

    void draw(const Shader *_shader) const noexcept;


  private:
    GLuint m_vao;
    Cubemap m_cubemap;
  };
}