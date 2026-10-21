#include <Renderer/Pipeline/GPUMemoryManager.hpp>
using namespace Renderer;


//private
std::vector<GPUBuffer> GPUMemoryManager::m_buffers;
std::vector<Mesh> GPUMemoryManager::m_meshes;
std::vector<Shader> GPUMemoryManager::m_shaders;
//private


GPUBuffer *GPUMemoryManager::CreateBuffer() {
  m_buffers.push_back(GPUBuffer());
  GPUBuffer *buffer = &m_buffers.back();
  (*buffer)();
  return buffer;
}
Mesh *GPUMemoryManager::CreateMesh() {
  m_meshes.push_back(Mesh());
  Mesh *mesh = &m_meshes.back();
  (*mesh)();
  return mesh;
}
Shader *GPUMemoryManager::CreateShader(const std::string &_name) {
  m_shaders.push_back(Shader(_name));
  return &m_shaders.back();
}