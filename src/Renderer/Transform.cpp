#include <Renderer/Transform.hpp>
using namespace Renderer;


Transform::Transform(
  const glm::vec3 _pos,
  const glm::quat _rotation,
  const glm::vec3 _scale
) : position(_pos), rotation(_rotation), scale(_scale) {}

glm::mat4x4 Transform::getMatrix() const noexcept {
  return
    glm::translate(glm::mat4x4(1), position) *
    glm::mat4_cast(rotation) *
    glm::scale(glm::mat4x4(1), scale);
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