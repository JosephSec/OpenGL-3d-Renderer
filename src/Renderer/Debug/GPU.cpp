#include <Renderer/Debug/GPU.hpp>
using namespace Renderer;


uint64_t GPU::VRAMCapacity = 0;
uint64_t GPU::VRAMInUse = 0;


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