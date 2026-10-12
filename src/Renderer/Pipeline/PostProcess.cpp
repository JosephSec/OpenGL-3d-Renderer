#include <Renderer/Pipeline/PostProcess.hpp>
using namespace Renderer;

#include <Renderer/Core.hpp>

#include <iostream>


PostProcess::PostProcess(int _w, int _h)
  : m_color(_w, _h, GL_RGBA8), m_depth(_w, _h, GL_DEPTH_COMPONENT24) {
  glCreateFramebuffers(1, &m_fbo);
  glCreateVertexArrays(1, &m_vao);
  attach();
}
PostProcess::~PostProcess() {
  if(m_fbo != 0) glDeleteFramebuffers(1, &m_fbo);
  if(m_vao != 0) glDeleteVertexArrays(1, &m_vao);
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
  glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void PostProcess::end(const Shader* _post, int _screenW, int _screenH) {
  const bool wireframe_state = Renderer::Core::GetWireframeMode();
  Renderer::Core::ToggleWireframeMode(false);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, _screenW, _screenH);
  glDisable(GL_DEPTH_TEST);

  _post->use();
  _post->setUniform("uFrame", 0);
  m_color.bind(0);
  glBindVertexArray(m_vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);

  glEnable(GL_DEPTH_TEST);
  Renderer::Core::ToggleWireframeMode(wireframe_state);
}


void PostProcess::attach() {
  glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_color.id(), 0);
  glNamedFramebufferTexture(m_fbo, GL_DEPTH_ATTACHMENT,  m_depth.id(), 0);
  if(glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "[POST PROCESS ERROR]: Post Process frame buffer is incomplete\n";
  }
}
