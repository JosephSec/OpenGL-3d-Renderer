#include <Renderer/Component/Light.hpp>
using namespace Renderer;

#include <Renderer/Core.hpp>


Light::Light(
  LightType _type,
  const glm::vec3 _position,
  const glm::vec3 _color,
  float _brightness
) : type(_type), position(_position), color(_color), brightness(_brightness) {}

void Light::drawGizmoTexture(GLuint _texture) const {
  Mesh textureMesh = MeshHelper::GenerateQuad();
  MeshHelper::SetMeshColor(textureMesh, color);

  MeshRenderer textureRenderer(&textureMesh, Core::GetShader("Unlit"));
  textureRenderer.backFaceCulling = false;
  textureRenderer.material.diffuseTex = _texture;

  const glm::quat rotation = Transform::LookAtRotation(position, Core::camera->transform.position, {0,1,0});
  textureRenderer.draw(Transform(position, rotation).getMatrix());
}


GLuint PointLight::GizmoTexture;

PointLight::PointLight(
  const glm::vec3 _position, 
  const glm::vec3 _color, 
  float _brightness, 
  float _radius
) : Light(LightType::Point, _position, _color, _brightness), radius(_radius) {}

void PointLight::setUniforms(Shader *_shader, unsigned int _index) const {
  const std::string prefix = "lights[" + std::to_string(_index) + "].";
  _shader->SetUniform(prefix+"type", type);
  _shader->SetUniform(prefix+"position", position);
  _shader->SetUniform(prefix+"color", glm::vec4{color, brightness});
  _shader->SetUniform(prefix+"radius", radius);
}
void PointLight::drawGizmos() const {
  Mesh mesh = MeshHelper::GenerateWireSphere(16, radius);
  MeshHelper::SetMeshColor(mesh, color);
  MeshRenderer renderer(&mesh, Core::GetShader("Unlit"));
  renderer.draw(Transform(position).getMatrix());
  
  drawGizmoTexture(GizmoTexture);
}


GLuint DirectionalLight::GizmoTexture;

DirectionalLight::DirectionalLight(const glm::vec3 _position, const glm::vec3 _color, float _brightness, const glm::vec3 _direction)
: Light(LightType::Directional, _position, _color, _brightness), direction(_direction) {}

void DirectionalLight::setUniforms(Shader *_shader, unsigned int _index) const {
  const std::string prefix = "lights[" + std::to_string(_index) + "].";
  _shader->SetUniform(prefix+"type", type);
  _shader->SetUniform(prefix+"color", glm::vec4{color, brightness});
  _shader->SetUniform(prefix+"direction", direction);
}
void DirectionalLight::drawGizmos() const {
  Mesh mesh(GL_LINES);
  mesh.vertices = {
    Mesh::Vertex{{0,0,0}, glm::vec4{color, 1}},
    Mesh::Vertex{glm::normalize(direction), glm::vec4{color, 1}},
  };
  mesh.indices = {0,1};

  MeshRenderer renderer(&mesh, Core::GetShader("Unlit"));
  renderer.draw(Transform(position).getMatrix());
  
  drawGizmoTexture(GizmoTexture);
}

GLuint SpotLight::GizmoTexture;

SpotLight::SpotLight(
  const glm::vec3 _position,
  const glm::vec3 _color,
  float _brightness,
  float _radius,
  const glm::vec3 _direction,
  float _innerCutOff,
  float _outerCutOff
) : Light(LightType::Spot, _position, _color, _brightness), radius(_radius), direction(_direction), innerCutOff(_innerCutOff), outerCutOff(_outerCutOff) {}

void SpotLight::setUniforms(Shader *_shader, unsigned int _index) const {
  const std::string prefix = "lights[" + std::to_string(_index) + "].";
  _shader->SetUniform(prefix+"type", type);
  _shader->SetUniform(prefix+"color", glm::vec4{color, brightness});
  _shader->SetUniform(prefix+"position", position);
  _shader->SetUniform(prefix+"direction", direction);
  _shader->SetUniform(prefix+"radius", radius);
  _shader->SetUniform(prefix+"innerCutOff", glm::cos(glm::radians(innerCutOff)));
  _shader->SetUniform(prefix+"outerCutOff", glm::cos(glm::radians(outerCutOff)));
}
void SpotLight::drawGizmos() const {
  Mesh mesh = MeshHelper::GenerateWireCone(16, 4, outerCutOff, radius);
  MeshHelper::SetMeshColor(mesh, color);
  MeshRenderer renderer(&mesh, Core::GetShader("Unlit"));
  renderer.draw(Transform(position, glm::quatLookAt(glm::normalize(direction), {0,1,0})).getMatrix());
  
  drawGizmoTexture(GizmoTexture);
}
