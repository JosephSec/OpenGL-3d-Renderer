#pragma once

#include <cstdint>

#include <Renderer/API.hpp>

#include <Renderer/Core/Mesh.hpp>


namespace Renderer {
  class RENDER3D_API GPU {
  public:
    static uint64_t VRAMCapacity;
    static uint64_t VRAMInUse;


    static std::string GetGPUName();

    static int64_t GetVRAMCapacity();
    static float GetUsedVRAM();

    static uint64_t GetShaderVRAMFootprint(GLuint _program);
    static uint64_t GetMeshVRAMFootprint(const Mesh *_mesh, size_t _allignment = 64);
  };
}