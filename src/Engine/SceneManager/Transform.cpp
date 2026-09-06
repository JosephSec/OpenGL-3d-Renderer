#include <Engine/SceneManager/Transform.hpp>


Transform::Transform(
  const glm::vec3 _pos,
  const glm::quat _rotation,
  const glm::vec3 _scale
) : position(_pos), rotation(_rotation), scale(_scale) {}

glm::mat4x4 Transform::getMatrix() const {
  return
    glm::translate(glm::mat4x4(1), position) *
    glm::mat4_cast(rotation) *
    glm::scale(glm::mat4x4(1), scale);
}