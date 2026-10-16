#include <Renderer/Core/States.hpp>
using namespace Renderer;

#include <Renderer/Core.hpp>


StateBuffer StateBuffer::Scene() {
  StateBuffer scene;
  scene.polygonMode = Core::GetScenePolygonMode();
  return scene;
}
StateBuffer StateBuffer::UI() {
  StateBuffer buf;
  buf.depthTest   = false;
  buf.depthWrite  = false;
  buf.cull        = false;
  buf.blend       = true;
  buf.multisample = false;
  return buf;
}



//private
StateBuffer StateCache::m_curState;
bool StateCache::s_valid = false;

std::vector<StateBuffer> StateCache::s_pushedStates;
//private


void StateCache::Apply(const StateBuffer &_stateBuf) {
  if(!s_valid) {
    ForceApply(_stateBuf);
    return;
  }

  StateBuffer &c = m_curState;

  if(_stateBuf.polygonMode != c.polygonMode) glPolygonMode(GL_FRONT_AND_BACK, _stateBuf.polygonMode);

  if(_stateBuf.depthTest != c.depthTest)   SetCap(GL_DEPTH_TEST, _stateBuf.depthTest);
  if(_stateBuf.depthWrite != c.depthWrite) glDepthMask(_stateBuf.depthWrite);
  if(_stateBuf.depthFunc != c.depthFunc)   glDepthFunc(_stateBuf.depthFunc);

  if(_stateBuf.cull != c.cull)           SetCap(GL_CULL_FACE, _stateBuf.cull);
  if(_stateBuf.cullFace != c.cullFace)   glCullFace(_stateBuf.cullFace);
  if(_stateBuf.frontFace != c.frontFace) glFrontFace(_stateBuf.frontFace);

  if(_stateBuf.blend != c.blend) SetCap(GL_BLEND, _stateBuf.blend);
  if(_stateBuf.srcRGB != c.srcRGB || _stateBuf.dstRGB != c.dstRGB || _stateBuf.srcAlpha != c.srcAlpha || _stateBuf.dstAlpha != c.dstAlpha) {
    glBlendFuncSeparate(_stateBuf.srcRGB, _stateBuf.dstRGB, _stateBuf.srcAlpha, _stateBuf.dstAlpha);
  }

  if(_stateBuf.scissor != c.scissor)         SetCap(GL_SCISSOR_TEST, _stateBuf.scissor);
  if(_stateBuf.multisample != c.multisample) SetCap(GL_MULTISAMPLE, _stateBuf.multisample);

  c = _stateBuf;
}

void StateCache::PushStates() {
  s_pushedStates.push_back(m_curState);
}
void StateCache::PullStates() {
  Apply(s_pushedStates.back());
  s_pushedStates.pop_back();
}


void StateCache::SetCap(GLenum _cap, bool _active) {
  _active? glEnable(_cap) : glDisable(_cap);
}
void StateCache::ForceApply(const StateBuffer &_stateBuf) {
  s_valid = false;
  m_curState = StateBuffer{};

  glPolygonMode(GL_FRONT_AND_BACK, _stateBuf.polygonMode);

  SetCap(GL_DEPTH_TEST, _stateBuf.depthTest);
  glDepthMask(_stateBuf.depthWrite);
  glDepthFunc(_stateBuf.depthFunc);

  SetCap(GL_CULL_FACE, _stateBuf.cull);
  glCullFace(_stateBuf.cullFace);
  glFrontFace(_stateBuf.frontFace);

  SetCap(GL_BLEND, _stateBuf.blend);
  glBlendFuncSeparate(_stateBuf.srcRGB, _stateBuf.dstRGB, _stateBuf.srcAlpha, _stateBuf.dstAlpha);

  SetCap(GL_SCISSOR_TEST, _stateBuf.scissor);
  SetCap(GL_MULTISAMPLE, _stateBuf.multisample);

  m_curState = _stateBuf;
  s_valid = true;
}