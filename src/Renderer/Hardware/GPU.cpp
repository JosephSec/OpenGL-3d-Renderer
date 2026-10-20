#include <Renderer/Hardware/GPU.hpp>
using namespace Renderer;

#include <Renderer/Core.hpp>

#include <windows.h>

#ifdef _WIN32
  #include <dxgi.h>
  #include <wrl/client.h>
#endif


uint64_t GPU::VRAMCapacity = 0;
uint64_t GPU::VRAMInUse = 0;



std::string GPU::GetGPUName() {
  const GLubyte *renderer = glGetString(GL_RENDERER);
  if(renderer != nullptr) return std::string(reinterpret_cast<const char*>(renderer));
  return "Unknown GPU";
}

int64_t GPU::GetVRAMCapacity() {
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
float GPU::GetUsedVRAM() {
  return 100.0f * VRAMInUse / VRAMCapacity;
}

uint64_t GPU::GetShaderVRAMFootprint(GLuint _program) {
  if(_program == 0) return 0;

  GLint linkStatus = 0;
  glGetProgramiv(_program, GL_LINK_STATUS, &linkStatus);
  if (linkStatus == GL_FALSE) return 0;

  GLint binaryLength = 0;
  glGetProgramiv(_program, GL_PROGRAM_BINARY_LENGTH, &binaryLength);

  return (binaryLength + 1024 - 1) & ~(1024 - 1);  
}
uint64_t GPU::GetMeshVRAMFootprint(const Mesh *_mesh, size_t _allignment) {
  uint64_t byteCount = _mesh->GetVRAMFootprint();
  if(_allignment == 0) return byteCount;
  return (byteCount + _allignment - 1) & ~(_allignment - 1);
}