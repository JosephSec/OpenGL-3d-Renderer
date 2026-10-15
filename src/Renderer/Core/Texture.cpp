#include <Renderer/Core/Texture.hpp>
using namespace Renderer;

#include <utility>
#include <algorithm>


Texture::Texture(int _width, int _height, GLenum _internalFormat, int _levels)
: m_width(_width), m_height(_height), m_levels(_levels), m_internalFormat(_internalFormat) {
  glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
  glTextureStorage2D(m_id, _levels, _internalFormat, _width, _height);

  glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, _levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
  glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
Texture::~Texture() {
  if(m_id != 0) glDeleteTextures(1, &m_id);
}

Texture::Texture(Texture&& o) noexcept {
  m_width = std::exchange(o.m_width, 0);
  m_height = std::exchange(o.m_height, 0);
  m_levels = std::exchange(o.m_levels, 0);

  m_id = std::exchange(o.m_id, 0);
  m_internalFormat = std::exchange(o.m_internalFormat, 0);
}
Texture& Texture::operator=(Texture&& o) noexcept {
  if(this != &o) {
    if(m_id != 0) glDeleteTextures(1, &m_id);

    m_width = std::exchange(o.m_width, 0);
    m_height = std::exchange(o.m_height, 0);
    m_levels = std::exchange(o.m_levels, 0);

    m_id = std::exchange(o.m_id, 0);
    m_internalFormat = std::exchange(o.m_internalFormat, 0);
  }
  return *this;
}

void Texture::upload(const void *_pixels, GLenum _format, GLenum _type, int _level) {
  const int w = std::max(m_width >> _level, 1);
  const int h = std::max(m_height >> _level, 1);
  glTextureSubImage2D(m_id, _level, 0,0, w,h, _format, _type, _pixels);
}
void Texture::generateMipmaps() const noexcept {
  glGenerateTextureMipmap(m_id);
}

void Texture::bind(GLuint _unit) const noexcept {
  glBindTextureUnit(_unit, m_id);
}