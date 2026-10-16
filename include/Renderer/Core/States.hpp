#pragma once

#include <vector>

#include <Renderer/Core/API.hpp>

#include <GL/glew.h>


namespace Renderer {
  enum class StateType {
    Scene = 0,
    UI = 1
  };

  struct RENDER3D_API StateBuffer {
  public:
    GLenum polygonMode = GL_TRIANGLES;

    bool   depthTest  = true;
    bool   depthWrite = true;
    GLenum depthFunc  = GL_LEQUAL;

    bool   cull      = true;
    GLenum cullFace  = GL_BACK;
    GLenum frontFace = GL_CW;

    bool blend      = false;
    GLenum srcRGB   = GL_SRC_ALPHA, dstRGB   = GL_ONE_MINUS_SRC_ALPHA;
    GLenum srcAlpha = GL_ONE,       dstAlpha = GL_ONE_MINUS_SRC_ALPHA;

    bool scissor = false;
    bool multisample = true;

    GLuint framebuffer = 0;
    GLuint program = 0;
    GLuint vao = 0;

    GLuint viewX = 0, viewY = 0, viewW = 0, viewH = 0;


    static StateBuffer Scene();
    static StateBuffer UI();
  };

  class RENDER3D_API StateCache {
  public:
    static void Apply(const StateBuffer &_stateBuf);
    static inline void Invalidate() {
      s_valid = false;
    }

    static void PushStates(); //add current states to vector to be set again with PullStates
    static void PullStates();


  private:
    static void SetCap(GLenum _cap, bool _active);
    static void ForceApply(const StateBuffer &_stateBuf);

    static StateBuffer m_curState;
    static bool s_valid;

    static std::vector<StateBuffer> s_pushedStates;
  };
}