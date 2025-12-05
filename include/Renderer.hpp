#pragma once

#include <SFML/Graphics.hpp>
#include <System/Vec2.hpp>
using Color = sf::Color;

#include <System/Transform.hpp>
#include <Renderer/Shader.hpp>
#include <Renderer/Mesh.hpp>
#include <Renderer/Light.hpp>
#include <Renderer/Line.hpp>
#include <Engine/GameObject.hpp>


static inline void SetMatrixVal(float out[16], float in[16]) {
  for(int i = 0; i < 16; i++) out[i] = in[i];
}


struct DebugRadius {
  static void (*DebugDrawRadius)(const Vec3&, float, const Color&);
  Vec3 center;
  float radius;
  Color color;

  void operator()() {
    DebugDrawRadius(center, radius, color);
  }
};
class Renderer {
public:
  static sf::RenderWindow* window;
  static Vec2 windowSize;
  static Vec2 windowCenter;

  static sf::Font font;
  static sf::Text TextPrefab;


  static float aspectRatio;
  static float nearPlane;
  static float farPlane;
  static float fov;
  static float f;

  static float projection[16];
  static float view[16];

  static Transform camera;

  
  static Shader LitTriangleShader;
  static Shader UnlitTriangleShader;
  static Shader LineShader;

  static std::vector<GameObject> objects;
  static std::vector<Mesh> meshs;

  static std::vector<Light> lights;

  static GameObject* selected;
  static Line line;

  static std::vector<DebugRadius> debugRadiusCalls;


  static void init();
  static void initGL();

  static void update();
  static void draw();
  static inline void draw(const sf::Drawable& drawable) {
    window->draw(drawable);
  }

  static void clear();

  static void UpdateViewMatrix() {
    SetMatrixVal(view, identityMatrix);
    float rotMat[16];
    Quaternion::Inverse(camera.rotation).getMatrix(rotMat);
    float transMat[16];
    SetMatrixVal(transMat, identityMatrix);
    for(int i = 0; i < 16; i++) view[i] = (i % 5 == 0) ? 1 : 0;

    for(int row = 0; row < 3; row++)
      for (int col = 0; col < 3; col++)
        view[col + row * 4] = rotMat[col + row * 4];

    Vec3 p = -camera.position;
    view[12] = p.x * view[0] + p.y * view[4] + p.z * view[8];
    view[13] = p.x * view[1] + p.y * view[5] + p.z * view[9];
    view[14] = p.x * view[2] + p.y * view[6] + p.z * view[10];
    view[15] = 1;
  }
  static void HandleResize() {
    const Vec2 windowSize = window->getSize();
    
    window->setView(sf::View(sf::FloatRect(Vec2(), windowSize)));
    glViewport(0,0, windowSize.x, windowSize.y);

    aspectRatio = windowSize.x / windowSize.y;

    float newProjection[16] = {
      f/aspectRatio, 0, 0, 0,
      0, f, 0, 0,
      0, 0, (farPlane+nearPlane)/(farPlane-nearPlane), 1,
      0, 0, (-2*farPlane*nearPlane)/(farPlane-nearPlane), 0
    };

    SetMatrixVal(projection, newProjection);
  }


  static void DrawRadius(const Vec3& center, float radius, const Color& color = Color::Green) {
    debugRadiusCalls.push_back(DebugRadius{center, radius, color});
  }
  static void DebugDrawRadius(const Vec3& center, float radius, const Color& color = Color::Green) {
    static float model[16];

    Transform transform(center, Quaternion(), Vec3one*radius);
    transform.getMatrix(model);
    LineShader.SetUniform("model", glm::make_mat4(model));
    line.colors = std::vector<Color>(line.indecies.size(), color);
    line.draw();

    transform.rotation = Quaternion::Euler(90,0,0);
    transform.getMatrix(model);
    LineShader.SetUniform("model", glm::make_mat4(model));    
    line.draw();

    transform.rotation = Quaternion::Euler(0,90,0);
    transform.getMatrix(model);
    LineShader.SetUniform("model", glm::make_mat4(model));    
    line.draw();
  }
};


#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

static Vec3 ScreenToWorld(const Vec2& screen, float depth = 0) {
  glm::vec4 ndc = glm::vec4(0,0,0,1);
  ndc.x = 2.0f * screen.x / Renderer::windowSize.x - 1.0f;
  ndc.y = 1.0f - 2.0f * screen.y / Renderer::windowSize.y;
  ndc.z = 2.0f * depth - 1.0f;

  glm::vec4 eye = glm::inverse(glm::make_mat4(Renderer::projection)) * ndc;
  eye /= eye.w;

  glm::vec4 world = glm::inverse(glm::make_mat4(Renderer::view)) * eye;
  return Vec3(world.x, world.y, world.z);
}


static inline Vec3 ApplyTransform(const Vec3& vec, const Transform& transform) {
  float mat4[16];
  transform.getMatrix(mat4);
  glm::vec3 result = glm::make_mat4(mat4) * glm::vec4(vec.x, vec.y, vec.z, 0);
  return Vec3(result.x, result.y, result.z);
}
static inline Vec3 CalculateNormal(const Vec3& a, const Vec3& b, const Vec3& c) {
  return Vec3::cross(b-a, c-a).normalize();
}

static inline float GetRaycastRadius(const Mesh* mesh) {
  float value = -INFINITY;
  for(const Vec3& vert : mesh->vertices) {
    const float dist = vert.magnitude();
    if(dist > value) value = dist;
  }
  return value;
}