#include <Engine/Renderer/Light.hpp>


Light::Light(const glm::vec3 &_position, const glm::vec3 _color, float _radius, float _strength)
: position(_position), color(_color), radius(_radius), strength(_strength) {}