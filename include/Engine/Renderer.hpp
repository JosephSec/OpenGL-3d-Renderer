#pragma once

#include <Engine/EngineAPI.hpp>

#include <SFML/Graphics/RenderWindow.hpp>

#include <Engine/Renderer/Shader.hpp>
#include <Engine/Renderer/Mesh.hpp>
#include <Engine/Renderer/MeshRenderer.hpp>
#include <Engine/Renderer/Camera.hpp>


enum class RenderState {
  OpenGL,
  UI
};
class ZENGINE_API Renderer {
public:
  static sf::RenderWindow *window;
  static glm::ivec2 windowSize;

  static bool WireframeMode;
  static Shader UnlitShader;

  static Camera *camera;
  static glm::mat4x4 viewMatrix;
  static glm::mat4x4 projectionMatrix;


  static void init(const sf::Vector2u _windowSize = sf::Vector2u{800,600}, const std::string _windowName = "OpenGL 3d Renderer");
  static void update();
  static void end();

  static void clear();
  static void ClearDepthBuffer();
  static void SetShader(GLuint _program);
  static void display();


  static void SetState(RenderState _state);
  static void SetCamera(Camera *_camera);

  static void UpdateProjectionMatrix();
  static void UpdateViewMatrix();
  static void HandleResize();

  static void ToggleWireframeMode(bool _enable = !WireframeMode);
};