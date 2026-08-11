#pragma once

#include <vector>
#include <functional>
#include <Engine/GameObject.hpp>
#include <Renderer/Line.hpp>
#include <Renderer/Mesh.hpp>


class Gizmo {
public:
  Transform transform;
  Mesh mesh;


  Gizmo() : mesh(Mesh::Circle) {}
  Gizmo(const Transform& _transform, const Mesh& _mesh, const Color& _color) : transform(_transform), mesh(_mesh) {
    for(Color& color : mesh.colors) color = _color;
  }

  void draw() const;
};

struct DebugRadius {
  static void (*DebugDrawRadius)(const Vec3&, float, const Color&);
  Vec3 center;
  float radius;
  Color color;

  inline void operator()() {
    DebugDrawRadius(center, radius, color);
  }
};
class Gizmos {
public:
  static Line circle;
  static Mesh arrow;

  static std::vector<DebugRadius> debugRadiusCalls;
  static std::vector<Gizmo> gizmos;


  static void init();
  static void update();
  static void draw();


  static void DrawRotateTool(const GameObject* object);
  

  static inline void DrawRadius(const Vec3& center, float radius, const Color& color = Color::Green) {
    debugRadiusCalls.push_back(DebugRadius{center, radius, color});
  }
  static inline void DrawCircle(const Vec3& center, float radius, const Color& color = Color::Green) {
    gizmos.push_back(Gizmo(Transform(center, Quaternion(), Vec3one * radius), Mesh::Circle, color));
  }


private:
  static void DebugDrawRadius(const Vec3& center, float radius, const Color& color = Color::Green);
};