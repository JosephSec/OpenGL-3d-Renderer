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
#include <Engine/Gizmos.hpp>


static inline void SetMatrixVal(float out[16], float in[16]) {
  for(int i = 0; i < 16; i++) out[i] = in[i];
}

///UPDATE ASPECT RATIO TO FIT VIEWPORT AREA
///UPDATE ASPECT RATIO TO FIT VIEWPORT AREA
///UPDATE ASPECT RATIO TO FIT VIEWPORT AREA
///UPDATE ASPECT RATIO TO FIT VIEWPORT AREA
///UPDATE ASPECT RATIO TO FIT VIEWPORT AREA
///UPDATE ASPECT RATIO TO FIT VIEWPORT AREA
///UPDATE ASPECT RATIO TO FIT VIEWPORT AREA
///UPDATE ASPECT RATIO TO FIT VIEWPORT AREA


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

  static std::vector<Gizmo*> gizmos;

  static std::vector<GameObject> objects;
  static std::vector<Mesh> meshs;

  static bool DrawLightRange;
  static std::vector<Light> lights;

  static unsigned int coneFaceCount;


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
    windowSize = window->getSize();
    windowCenter = windowSize/2.0f;

    window->setView(sf::View(sf::FloatRect(Vec2zero, windowSize)));
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


static Vec3 ApplyTransform(const Vec3& vec, const Transform& transform, bool isDirection = true) {
  float mat4[16];
  transform.getMatrix(mat4);
  glm::vec3 result = glm::make_mat4(mat4) * glm::vec4(vec.x, vec.y, vec.z, !isDirection);
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