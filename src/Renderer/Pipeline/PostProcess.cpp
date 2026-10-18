#include <Renderer/Pipeline/PostProcess.hpp>
using namespace Renderer;

#include <Renderer/Core.hpp>

#include <iostream>


PostProcess::PostProcess(int _w, int _h)
  : m_w(_w), m_h(_h), m_color(_w, _h, GL_RGBA8), m_depth(_w, _h, GL_DEPTH_COMPONENT24) {
  glCreateFramebuffers(1, &m_fbo);
  glCreateVertexArrays(1, &m_vao);
  attach();
}
PostProcess::~PostProcess() {
  if(m_fbo != 0) glDeleteFramebuffers(1, &m_fbo);
  if(m_vao != 0) glDeleteVertexArrays(1, &m_vao);
}

PostProcess::PostProcess(PostProcess &&_o) noexcept
: m_color(std::move(_o.m_color)),
  m_depth(std::move(_o.m_depth)),
  m_fbo(std::move(_o.m_fbo)),
  m_vao(std::move(_o.m_vao)),
  m_w(std::exchange(_o.m_w, 0)),
  m_h(std::exchange(_o.m_h, 0)) {}
PostProcess &PostProcess::operator=(PostProcess &&_o) noexcept {
  if(this != &_o) {
    if(m_fbo != 0) glDeleteFramebuffers(1, &m_fbo);
    if(m_vao != 0) glDeleteVertexArrays(1, &m_vao);

    m_color = std::move(_o.m_color);
    m_depth = std::move(_o.m_depth);
    m_fbo = std::move(_o.m_fbo);
    m_vao = std::move(_o.m_vao);
    m_w = std::exchange(_o.m_w, 0);
    m_h = std::exchange(_o.m_h, 0);
  }
  return *this;
}


void PostProcess::resize(int _w, int _h) {
  m_color = Texture(_w, _h, GL_RGBA8);
  m_depth = Texture(_w, _h, GL_DEPTH_COMPONENT24);
  attach();
  m_w = _w; m_h = _h;
}

void PostProcess::begin(int _w, int _h) {
  if(_w != m_w || _h != m_h) resize(_w, _h);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glViewport(0, 0, _w, _h);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void PostProcess::end(const Shader* _post, int _screenW, int _screenH) {
  Renderer::StateCache::PushStates();
  Renderer::Core::SetPolygonMode(GL_FILL);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, _screenW, _screenH);
  glDisable(GL_DEPTH_TEST);

  _post->use();

  m_color.bind(0);
  m_depth.bind(1);
  _post->setUniform("uColor", 0);
  _post->setUniform("uDepth", 1);

  glBindVertexArray(m_vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);

  glEnable(GL_DEPTH_TEST);
  Renderer::StateCache::PullStates();
}


void PostProcess::attach() {
  glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_color.id(), 0);
  glNamedFramebufferTexture(m_fbo, GL_DEPTH_ATTACHMENT,  m_depth.id(), 0);
  if(glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "[POST PROCESS ERROR]: Post Process frame buffer is incomplete\n";
  }
}
