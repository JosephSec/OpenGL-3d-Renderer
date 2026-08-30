#pragma once

#include <Engine/EngineAPI.hpp>

#include <SFML/Graphics/RenderWindow.hpp>

#include <Engine/Renderer/Shader.hpp>
#include <Engine/Renderer/Mesh.hpp>
#include <Engine/Renderer/MeshRenderer.hpp>
#include <Engine/Renderer/Camera.hpp>


class ZENGINE_API Renderer {
public:
  static sf::RenderWindow *window;
  static glm::ivec2 windowSize;

  static bool wireframeMode;
  static Shader UnlitShader;

  static Camera *camera;
  static glm::mat4x4 viewMatrix;
  static glm::mat4x4 projectionMatrix;


  static void init();
  static void update();
  static void end();

  static void clear();
  static void SetShader(GLuint _program);
  static void display();


  static void UpdateProjectionMatrix();
  static void UpdateViewMatrix();
  static void HandleResize();

  static void ToggleWireframeMode(bool _enable = !wireframeMode);
};