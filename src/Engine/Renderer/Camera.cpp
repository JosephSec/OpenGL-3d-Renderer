#include <Engine/Renderer/Camera.hpp>


Camera::Camera(const glm::vec3 _pos, const glm::quat _rotation)
: transform(_pos, _rotation, glm::vec3(1)) {}

glm::mat4x4 Camera::getViewMatrix() const {
  const glm::mat4x4 rotation = glm::mat4_cast(glm::conjugate(transform.rotation));
  const glm::mat4x4 translation = glm::translate(glm::mat4x4(1), -transform.position);
  return rotation * translation;
}
glm::mat4x4 Camera::getProjectionMatrix(const glm::vec2 _windowSize) const {
  const float aspectRatio = static_cast<float>(_windowSize.x) / static_cast<float>(_windowSize.y);
  return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}