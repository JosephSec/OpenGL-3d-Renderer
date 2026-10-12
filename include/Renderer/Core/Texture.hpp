#pragma once

#include <Renderer/Core/API.hpp>

#include <GL/glew.h>


namespace Renderer {
  class RENDER3D_API Texture {
  public:
    Texture(int _width, int _height, GLenum _internalFormat = GL_RGBA8, int _levels = 1);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& o) noexcept;
    Texture& operator=(Texture&& o) noexcept;

    void upload(const void* _pixels, GLenum _format = GL_RGBA, GLenum _type = GL_UNSIGNED_BYTE, int _level = 0);
    void generateMipmaps() const noexcept;
    void bind(GLuint _unit) const noexcept;

    inline GLuint id() const noexcept {
      return m_id;
    }
    inline int width() const noexcept {
      return m_width;
    }
    inline int height() const noexcept {
      return m_height;
    }

  private:
    int m_width = 0;
    int m_height = 0;
    int m_levels = 0;
    
    GLuint m_id = 0;
    GLenum m_internalFormat = GL_RGBA8;
  };
}