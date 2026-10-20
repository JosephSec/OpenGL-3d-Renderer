#pragma once

#include <Renderer/API.hpp>


namespace Renderer {
  class RENDER3D_API Monitor {
  public:
    static int RefreshRate;

    static int GetCurrentMonitorRefreshRate();
  };
}