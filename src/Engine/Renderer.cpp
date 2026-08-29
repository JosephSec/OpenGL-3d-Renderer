#include <Engine/Renderer.hpp>
#include <System.hpp>

#include <format>


sf::RenderWindow Renderer::window;
glm::ivec2 Renderer::windowSize;

Shader Renderer::UnlitShader;

Camera *Renderer::camera;
glm::mat4x4 Renderer::viewMatrix;
glm::mat4x4 Renderer::projectionMatrix;


static void initGL() {
  GLenum err = glewInit();
  if(err != GLEW_OK) std::cout << "GLEW init error: " << glewGetErrorString(err) << '\n';

  glEnable(GL_DEPTH_TEST);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CW);

  glClearColor(.1,.1,.15, 1);
}
void Renderer::init() {
  sf::ContextSettings settings;
  settings.depthBits = 24;
  settings.stencilBits = 8;
  settings.antiAliasingLevel = 4;
  settings.majorVersion = 3;
  settings.minorVersion = 3;
  settings.attributeFlags = sf::ContextSettings::Default;

  window = sf::RenderWindow(sf::VideoMode{{800,600}}, "OpenGL 3d Renderer", sf::State::Windowed, settings);
  window.setVerticalSyncEnabled(true);
  // window.setFramerateLimit(60);

  initGL();

  UnlitShader = Shader("Unlit");
}
void Renderer::update() {}
void Renderer::draw() {}


void Renderer::UpdateProjectionMatrix() {
  if(camera == nullptr) {
    std::cout << "[Renderer Error] Cannot update projection matrix: Camera is nullptr\n";
    return;
  }

  glUseProgram(UnlitShader);
  UnlitShader.SetUniform("projection", camera->getProjectionMatrix(windowSize));
  glUseProgram(0);
}
void Renderer::UpdateViewMatrix() {
  if(camera == nullptr) {
    std::cout << "[Renderer Error] Cannot update view matrix: Camera is nullptr\n";
    return;
  }

  glUseProgram(UnlitShader);
  UnlitShader.SetUniform("view", camera->getViewMatrix());
  glUseProgram(0);
}
void Renderer::HandleResize() {
  const sf::Vector2u sf_windowSize = window.getSize();
  windowSize = glm::ivec2(sf_windowSize.x, sf_windowSize.y);

  window.setView(sf::View(sf::FloatRect{{0,0}, sf::Vector2f{sf_windowSize}}));
  glViewport(0,0, windowSize.x, windowSize.y);
  UpdateProjectionMatrix();
}