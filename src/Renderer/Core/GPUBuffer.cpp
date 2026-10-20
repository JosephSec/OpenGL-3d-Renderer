#include <Renderer/Core/GPUBuffer.hpp>
using namespace Renderer;

#include <Renderer/Hardware/GPU.hpp>


GPUBuffer::GPUBuffer() : m_id(0), m_size(0), m_capacity(0) {}
GPUBuffer::~GPUBuffer() {
  if(m_id != 0) glDeleteBuffers(1, &m_id);
  if(m_data != nullptr) std::free(m_data);
  m_size = m_capacity = 0;
}

GPUBuffer::GPUBuffer(GPUBuffer &&_o) :
  m_id(std::exchange(_o.m_id, 0)),
  m_size(std::exchange(_o.m_size, 0)),
  m_capacity(std::exchange(_o.m_capacity, 0)),
  m_data(std::exchange(_o.m_data, nullptr)) {}
GPUBuffer &GPUBuffer::operator=(GPUBuffer &&_o) {
  if(this != &_o) {
    if(m_id != 0) glDeleteBuffers(1, &m_id);

    m_id       = std::exchange(_o.m_id, 0);
    m_size     = std::exchange(_o.m_size, 0);
    m_capacity = std::exchange(_o.m_capacity, 0);
    m_data     = std::exchange(_o.m_data, nullptr);
  }
  return *this;
}

void GPUBuffer::operator()() noexcept {
  if(m_id != 0) glDeleteBuffers(1, &m_id);
  if(m_data != nullptr) {
    GPU::VRAMInUse -= m_capacity;
    std::free(m_data);
    m_data = nullptr;
  }

  m_id = m_size = m_capacity = 0;
  glCreateBuffers(1, &m_id);
}


void GPUBuffer::upload(GLsizeiptr _size, const void *_data, GLenum _usage) noexcept {
  m_size = _size;
  
  if(m_size > m_capacity) {
    m_capacity = m_size;
    m_data = realloc(m_data, m_capacity);

    if(m_data != nullptr && _data != nullptr) memcpy(m_data, _data, m_size);
    glNamedBufferData(m_id, m_size, m_data, _usage);
  }
  else {
    if(m_data != nullptr && _data != nullptr) memcpy(m_data, _data, m_size);
    glNamedBufferSubData(m_id, 0, m_size, m_data);
  }

  GPU::VRAMInUse += m_capacity;
}
