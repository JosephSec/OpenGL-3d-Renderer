#pragma once

#include <SFML/Graphics/RenderWindow.hpp>

#include <Engine/Renderer/Shader.hpp>
#include <Engine/Renderer/Mesh.hpp>
#include <Engine/Renderer/MeshRenderer.hpp>
#include <Engine/Renderer/Camera.hpp>


class Renderer {
public:
  static sf::RenderWindow window;
  static glm::ivec2 windowSize;

  static Shader UnlitShader;

  static Camera *camera;
  static glm::mat4x4 viewMatrix;
  static glm::mat4x4 projectionMatrix;


  static void init();
  static void update();
  static void draw();


  static void UpdateProjectionMatrix();
  static void UpdateViewMatrix();
  static void HandleResize();
};