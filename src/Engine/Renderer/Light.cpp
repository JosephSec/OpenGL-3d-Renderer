#include <Engine/Renderer/Light.hpp>


Light::Light(const glm::vec3 &_position, const glm::vec4 _color, float _radius)
: position(_position), color(_color), radius(_radius) {}