#include <Renderer/Core.hpp>
using namespace Renderer;

#include <Renderer/Debug/GPU.hpp>

#include <cstdint>
#include <string>
#include <algorithm>

#ifdef _WIN32
  #include <dxgi.h>
  #include <wrl/client.h>
#endif


sf::RenderWindow *Core::window;
glm::ivec2 Core::windowSize;


void Core::init(bool _enableVSync) {
  window = new sf::RenderWindow(sf::VideoMode({800,600}), "3D Rendering Library");
  window->setVerticalSyncEnabled(_enableVSync);
  windowSize = glm::ivec2{window->getSize().x, window->getSize().y};

  glewInit();

  GPU::VRAMCapacity = GetVRAMCapacity();
  GPU::VRAMInUse = 0;

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CW);
  glCullFace(GL_BACK);
}
void Core::clear() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void Core::display() {
  window->display();
}
void Core::end() {}

void Core::UpdateWindowSize() {
  windowSize = glm::vec2{window->getSize().x, window->getSize().y};

  window->setView(sf::View(sf::FloatRect{{0,0}, {windowSize.x,windowSize.y}}));

  glViewport(0,0, windowSize.x,windowSize.y);
}

int64_t Core::GetVRAMCapacity() {
#ifdef _WIN32
  using Microsoft::WRL::ComPtr;

  ComPtr<IDXGIFactory1> factory;
  if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(factory.GetAddressOf())))) {
    const std::string glName = GetGPUName();
    int64_t largest = -1;

    ComPtr<IDXGIAdapter1> adapter;
    for (UINT i = 0;
         factory->EnumAdapters1(i, adapter.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND;
         ++i) {
      DXGI_ADAPTER_DESC1 desc;
      if (FAILED(adapter->GetDesc1(&desc))) continue;
      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue; // skip WARP

      const int64_t dedicated = static_cast<int64_t>(desc.DedicatedVideoMemory);

      // Convert the wide adapter name to a narrow string (GPU names are ASCII)
      std::wstring wname(desc.Description);
      std::string name(wname.begin(), wname.end());

      // Prefer the adapter that matches the GL renderer
      if (!name.empty() && glName.find(name.substr(0, 10)) != std::string::npos)
        return dedicated;

      largest = std::max(largest, dedicated);
    }
    if (largest > 0) return largest; // fallback: biggest dedicated pool
  }
#endif

  // NVIDIA on non-Windows (or if DXGI failed)
  if (glewIsSupported("GL_NVX_gpu_memory_info")) {
    GLint totalKB = 0;
    glGetIntegerv(0x9047, &totalKB);
    if (glGetError() == GL_NO_ERROR && totalKB > 0)
      return static_cast<int64_t>(totalKB) * 1024;
  }

  return -1; // unknown
}
std::string Core::GetGPUName() {
  const GLubyte *renderer = glGetString(GL_RENDERER);
  if(renderer != nullptr) return std::string(reinterpret_cast<const char*>(renderer));
  return "Unknown GPU";
}
