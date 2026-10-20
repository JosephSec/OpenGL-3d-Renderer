#pragma once

#include <utility>
#include <memory>

#include <Renderer/API.hpp>

#include <GL/glew.h>


namespace Renderer {
  struct RENDER3D_API GPUBuffer {
  public:
    GPUBuffer();
    ~GPUBuffer();

    GPUBuffer(GPUBuffer&) = delete;
    GPUBuffer operator=(GPUBuffer&) = delete;

    GPUBuffer(GPUBuffer &&_o);
    GPUBuffer &operator=(GPUBuffer &&_o);

    void operator()() noexcept;


    void upload(GLsizeiptr _size, const void *_data, GLenum _usage) noexcept;


    inline GLuint id() const noexcept {
      return m_id;
    }
    inline GLsizeiptr size() const noexcept {
      return m_size;
    }
    inline GLsizeiptr capacity() const noexcept { //returns VRAM usage in bytes
      return m_capacity;
    }
    inline const void *data() const noexcept {
      return m_data;
    }


  private:
    GLuint m_id = 0;
    GLsizeiptr m_size = 0, m_capacity = 0;
    void *m_data = nullptr;
  };
}