#pragma once

#include <SFML/Graphics/Color.hpp>
using Color = sf::Color;

#include <Engine/GameObject.hpp>


class Light : public GameObject {
public:
  Color color;
  float range;

  Light(const Vec3& _pos = Vec3(), const Color& _clr = Color::White, float _range = 10);

  inline Vec3 getColorVec() const {
    return Vec3(color.r, color.g, color.b) * 0.003921569f;
  }

  void update();
  void draw() const;
};