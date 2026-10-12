#include <Renderer/Core/Cubemap.hpp>
using namespace Renderer;

#include <utility>
#include <algorithm>


Cubemap::Cubemap(int _size, GLenum _internalFormat, int _levels) : m_size(_size) {
  glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_id);
  glTextureStorage2D(m_id, _levels, _internalFormat, _size,_size);

  glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, _levels > 1? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
  glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTextureParameteri(m_id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}
Cubemap::~Cubemap() {
  if(m_id != 0) glDeleteTextures(1, &m_id);
  m_size = 0;
}

Cubemap::Cubemap(Cubemap&& o) noexcept {
  m_id = std::exchange(o.m_id, 0);
  m_size = std::exchange(o.m_size, 0);
}
Cubemap& Cubemap::operator=(Cubemap&& o) noexcept {
  if(this != &o) {
    if(m_id != 0) glDeleteTextures(1, &m_id);
    
    m_id = std::exchange(o.m_id, 0);
    m_size = std::exchange(o.m_size, 0);
  }
  return *this;
}


void Cubemap::generateMipmaps() const noexcept {
  glGenerateTextureMipmap(m_id);
}

void Cubemap::uploadFace(int _face, const void *_pixels, GLenum _format, GLenum _type, int _level) {
  const int size = std::max(m_size >> _level, 1);
  glTextureSubImage3D(m_id, _level, 0,0,_face, size,size, 1, _format, _type, _pixels);
}

void Cubemap::bind(GLuint _unit) const {
  glBindTextureUnit(_unit, m_id);
}