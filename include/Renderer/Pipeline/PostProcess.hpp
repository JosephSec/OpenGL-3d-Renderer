#pragma once

#include <Renderer/Core/API.hpp>

#include <GL/glew.h>
#include <Renderer/Core/Shader.hpp>
#include <Renderer/Core/Texture.hpp>


namespace Renderer {
  class RENDER3D_API PostProcess {
  public:
    PostProcess(int _w, int _h);
    ~PostProcess();


    void resize(int _w, int _h);

    void begin(int _w, int _h);
    void end(const Shader* _post, int _screenW, int _screenH);

  // private:
    void attach();

    Texture m_color, m_depth;
    GLuint m_fbo = 0, m_vao = 0;
    int m_w = 0, m_h = 0;
  };
}
