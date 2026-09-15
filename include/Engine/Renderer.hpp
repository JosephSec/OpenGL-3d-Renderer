#pragma once

#include <map>

#include <SFML/Graphics/RenderWindow.hpp>

#include <Engine/API.hpp>
#include <Engine/Renderer/Shader.hpp>
#include <Engine/Renderer/Camera.hpp>
#include <Engine/Renderer/Light.hpp>


enum class RenderState {
  OpenGL,
  UI
};
class ZENGINE_API Renderer {
public:
  static sf::RenderWindow *window;
  static glm::ivec2 windowSize;

  static bool WireframeMode;

  static Camera *camera;
  static glm::mat4x4 viewMatrix;
  static glm::mat4x4 projectionMatrix;

  static glm::vec4 ambientLight;
  static std::vector<Light> lights;


  static void init(const sf::Vector2u _windowSize = sf::Vector2u{800,600}, const std::string _windowName = "OpenGL 3d Renderer");
  static void end();


  static bool LoadShader(const std::string &_name);
  static Shader *GetShader(const std::string &_name);


  static void UpdateViewMatrix();
  static void UpdateProjectionMatrix();
  static void UpdateWindowSize();
  static void UpdateDynamicLighting();


  static void clear();
  static void display();
  
  static void ClearDepthBuffer();


  static void SetState(RenderState _state);
  static void SetCamera(Camera *_camera);
  static void SetShader(GLuint _program);


  static void ToggleWireframeMode(bool _enable = !WireframeMode);

private:
  static std::map<std::string, Shader> s_shaders;


  static void initGL();
};