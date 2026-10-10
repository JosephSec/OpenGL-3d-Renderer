#include <Renderer/Component/Light.hpp>
using namespace Renderer;

#include <Renderer/Core.hpp>


Light::Light(
  LightType _type,
  const glm::vec3 _position,
  const glm::vec4 _color
) : type(_type), position(_position), color(_color) {}

// void Light::drawGizmoTexture(GLuint _texture, const Shader *_shader) const {
//   Mesh textureMesh = MeshHelper::GenerateQuad();
//   MeshHelper::RandomizeMeshColors(textureMesh, {glm::vec4{glm::vec3{color},1}});

//   MeshRenderer textureRenderer(&textureMesh, _shader);
//   textureRenderer.backFaceCulling = false;
//   textureRenderer.material.albedoTexture = _texture;

//   const glm::quat rotation = Transform::LookAtRotation(position, Core::camera->transform.position, {0,1,0});
//   textureRenderer.draw(Transform(position, rotation).getModelMatrix());
// }


// GLuint PointLight::GizmoTexture;

PointLight::PointLight(
  const glm::vec3 _position,
  const glm::vec4 _color,
  float _radius
) : Light(LightType::Point, _position, _color), radius(_radius) {}

void PointLight::setUniforms(Shader *_shader, unsigned int _index) const {
  const std::string prefix = "lights[" + std::to_string(_index) + "].";
  _shader->setUniform(prefix+"type", static_cast<int>(type));
  _shader->setUniform(prefix+"position", position);
  _shader->setUniform(prefix+"color", color);
  _shader->setUniform(prefix+"radius", radius);
}
// void PointLight::drawGizmos(const Shader *_shader) const {
//   Mesh mesh = MeshHelper::GenerateWireSphere(16, radius);
//   MeshHelper::RandomizeMeshColors(mesh, {glm::vec4{color,1}});
//   MeshRenderer renderer(&mesh, _shader);
//   renderer.draw(Transform(position).getModelMatrix());
  
//   drawGizmoTexture(GizmoTexture);
// }


// GLuint DirectionalLight::GizmoTexture;

DirectionalLight::DirectionalLight(
  const glm::vec3 _position,
  const glm::vec4 _color,
  const glm::vec3 _direction
) : Light(LightType::Directional, _position, _color), direction(_direction) {}

void DirectionalLight::setUniforms(Shader *_shader, unsigned int _index) const {
  const std::string prefix = "lights[" + std::to_string(_index) + "].";
  _shader->setUniform(prefix+"type", static_cast<int>(type));
  _shader->setUniform(prefix+"color", color);
  _shader->setUniform(prefix+"direction", direction);
}
// void DirectionalLight::drawGizmos(const Shader *_shader) const {
//   Mesh mesh(GL_LINES);
//   mesh.setVertices({
//     Vertex{{0,0,0}, glm::vec4{glm::vec3{color}, 1}},
//     Vertex{glm::normalize(direction), glm::vec4{glm::vec3{color}, 1}},
//   });
//   mesh.setIndices({0,1});

//   MeshRenderer renderer(&mesh, _shader);
//   renderer.draw(Transform(position).getModelMatrix());
  
//   drawGizmoTexture(GizmoTexture, _shader);
// }

// GLuint SpotLight::GizmoTexture;

SpotLight::SpotLight(
  const glm::vec3 _position,
  const glm::vec4 _color,
  float _radius,
  const glm::vec3 _direction,
  float _innerCutOff,
  float _outerCutOff
) : Light(LightType::Spot, _position, _color), radius(_radius), direction(_direction), innerCutOff(_innerCutOff), outerCutOff(_outerCutOff) {}

void SpotLight::setUniforms(Shader *_shader, unsigned int _index) const {
  const std::string prefix = "lights[" + std::to_string(_index) + "].";
  _shader->setUniform(prefix+"type", static_cast<int>(type));
  _shader->setUniform(prefix+"color", color);
  _shader->setUniform(prefix+"position", position);
  _shader->setUniform(prefix+"direction", direction);
  _shader->setUniform(prefix+"radius", radius);
  _shader->setUniform(prefix+"innerCutOff", glm::cos(glm::radians(innerCutOff)));
  _shader->setUniform(prefix+"outerCutOff", glm::cos(glm::radians(outerCutOff)));
}
// void SpotLight::drawGizmos(const Shader *_shader) const {
//   Mesh mesh = MeshHelper::GenerateWireCone(16, 4, outerCutOff, radius);
//   MeshHelper::RandomizeMeshColors(mesh, {glm::vec4{color,1}});
//   MeshRenderer renderer(&mesh, _shader);
//   renderer.draw(Transform(position, glm::quatLookAt(glm::normalize(direction), {0,1,0})).getModelMatrix());

//   drawGizmoTexture(GizmoTexture, _shader);
// }