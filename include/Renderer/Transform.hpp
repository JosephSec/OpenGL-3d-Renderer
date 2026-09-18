#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Renderer/API.hpp>


namespace Renderer {
  class HORDE3D_API Transform {
  public:
    Transform(
      const glm::vec3 _pos = glm::vec3(0),
      const glm::quat _rotation = glm::quat(),
      const glm::vec3 _scale = glm::vec3(1)
    );

    glm::mat4x4 getMatrix() const noexcept;

    glm::vec3 right() const noexcept;
    glm::vec3 up() const noexcept;
    glm::vec3 forward() const noexcept;


    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale = glm::vec3(1);
  };
}