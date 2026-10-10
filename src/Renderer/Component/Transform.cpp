#include <Renderer/Component/Transform.hpp>
using namespace Renderer;

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>


glm::quat Transform::LookAtRotation(const glm::vec3 _start, const glm::vec3 _end, const glm::vec3 _up) {
  const glm::vec3 direction = _end - _start;

  if(glm::length2(direction) < 1e-4f) return glm::quat();

  return glm::quatLookAt(glm::normalize(direction), _up);
}



Transform::Transform(
  const glm::vec3 _pos,
  const glm::quat _rotation,
  const glm::vec3 _scale
) : position(_pos), rotation(_rotation), scale(_scale) {}

glm::mat4x4 Transform::getModelMatrix() const noexcept {
  return
    glm::translate(glm::mat4x4(1), position) *
    glm::mat4_cast(rotation) *
    glm::scale(glm::mat4x4(1), scale);
}
glm::mat4x4 Transform::getViewMatrix() const noexcept {
  const glm::mat4x4 rotationMatrix = glm::mat4_cast(glm::conjugate(rotation));
  const glm::mat4x4 translationMatrix = glm::translate(glm::mat4x4(1), -position);
  return rotationMatrix * translationMatrix;
}
glm::mat4x4 Transform::getProjectionMatrix(const glm::ivec2 _windowSize, float _fov, float _near, float _far) const noexcept {
  const float aspectRatio = static_cast<float>(_windowSize.x) / static_cast<float>(_windowSize.y);
  return glm::perspective(glm::radians(_fov), aspectRatio, _near, _far);
}

glm::vec3 Transform::right() const noexcept {
  return rotation * glm::vec3(1,0,0);
}
glm::vec3 Transform::up() const noexcept {
  return rotation * glm::vec3(0,1,0);
}
glm::vec3 Transform::forward() const noexcept {
  return rotation * glm::vec3(0,0,-1);
}