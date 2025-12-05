#pragma once

#include <string>
using string = std::string;

#include <System/Transform.hpp>
#include <Renderer/Mesh.hpp>


class GameObject {
public:
  string name;

  Transform transform;
  Mesh* mesh;


  GameObject(Transform _transform, Mesh* _mesh = nullptr);

  void update();
  void draw() const;
};