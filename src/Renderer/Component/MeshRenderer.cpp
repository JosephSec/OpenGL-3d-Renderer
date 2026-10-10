#include <Renderer/Component/MeshRenderer.hpp>
using namespace Renderer;


MeshRenderer::MeshRenderer(Mesh *_mesh, const Shader *_shader) {
  setMesh(_mesh);
  setShader(_shader);
}


void MeshRenderer::draw(const glm::mat4x4 &_model) const noexcept {
  if(isDrawable() == false) return;

  m_shader->use();
  m_shader->setUniform(m_isInstancedUniformLocation, m_isInstanced);

  m_mesh->use();
  if(m_isInstanced == false) {
    m_shader->setUniform(m_modelUniformLocation, _model);
    m_mesh->draw();
  }
  else m_mesh->drawInstanced();
}


DrawCommand MeshRenderer::getDrawCommand(const Transform &_transform, uint8_t _pass) const noexcept {
  return DrawCommand{
    GetDrawCommandKey(_pass, m_shader->getID(), m_mesh->getID(), 0),
    m_mesh,
    m_shader,
    m_modelUniformLocation,
    _transform.getModelMatrix()
  };
}


void MeshRenderer::setMesh(Mesh *_mesh) noexcept {
  m_mesh = _mesh;
}
void MeshRenderer::setShader(const Shader *_shader) noexcept {
  m_shader = _shader;
  m_modelUniformLocation = _shader->getUniform("uModel");
  m_isInstancedUniformLocation = _shader->getUniform("uIsInstanced");
}

void MeshRenderer::setInstancingData(const std::vector<glm::mat4x4> &_instances) {
  if(m_mesh == nullptr) return;

  m_mesh->setInstances(_instances);
  m_isInstanced = _instances.empty() == false;
}
void MeshRenderer::setInstancingData(const std::vector<Transform> &_instances) {
  if(m_mesh == nullptr) return;

  std::vector<glm::mat4x4> modelMatrices;
  modelMatrices.reserve(_instances.size());
  for(const Transform &transform : _instances) {
    modelMatrices.push_back(transform.getModelMatrix());
  }

  setInstancingData(modelMatrices);
}


bool MeshRenderer::isDrawable() const noexcept {
  return (m_mesh != nullptr) && (m_shader != nullptr);
}


uint64_t MeshRenderer::GetDrawCommandKey(uint8_t _pass, uint16_t _shaderID, uint16_t _meshID, uint32_t _depth) {
  return (
    static_cast<uint64_t>(_pass)     << 56 |
    static_cast<uint64_t>(_shaderID) << 40 |
    static_cast<uint64_t>(_meshID)   << 24 |
    static_cast<uint64_t>(_depth & 0xFFFFF)
  );
}
