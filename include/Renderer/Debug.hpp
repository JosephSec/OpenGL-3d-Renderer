#pragma once

#include <string>
#include <vector>
#include <map>

#include <Renderer/Core/API.hpp>

#include <GL/glew.h>


namespace Renderer {
  class RENDER3D_API Debug {
  public:
    static void dumpGLOnChange(const char *tag, GLuint postFbo = 0);
  };
}