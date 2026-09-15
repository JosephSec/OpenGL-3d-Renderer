#include <Engine/Renderer.hpp>

#include <iostream>


sf::RenderWindow *Renderer::window;
glm::ivec2 Renderer::windowSize;

bool Renderer::WireframeMode = false;

Camera *Renderer::camera;
glm::mat4x4 Renderer::projectionMatrix = glm::mat4x4(1);
glm::mat4x4 Renderer::viewMatrix = glm::mat4x4(1);

glm::vec4 Renderer::ambientLight;
std::vector<Light> Renderer::lights;

//private
std::map<std::string, Shader> Renderer::s_shaders;
//private


void Renderer::init(const sf::Vector2u _windowSize, const std::string _windowName) {
  sf::ContextSettings settings;
  settings.depthBits = 24;
  settings.stencilBits = 8;
  settings.antiAliasingLevel = 4;
  settings.majorVersion = 3;
  settings.minorVersion = 3;
  settings.attributeFlags = sf::ContextSettings::Default;

  window = new sf::RenderWindow(sf::VideoMode{_windowSize}, _windowName, sf::State::Windowed, settings);

  initGL();

  windowSize = glm::ivec2(_windowSize.x, _windowSize.y);

  Renderer::LoadShader("Unlit");
  Renderer::LoadShader("Lit");
}
void Renderer::end() {
  for(const auto &[name, shader] : s_shaders) glDeleteProgram(shader);
  delete Renderer::window;  
}


bool Renderer::LoadShader(const std::string &_name) {
  if(s_shaders.find(_name) != s_shaders.end()) {
    std::cout << "[RENDERER ERROR]: Attempted to load shader that was previously loaded\n";
    return false;
  }

  s_shaders.insert({_name, Shader(_name)});
  return true;
}
Shader *Renderer::GetShader(const std::string &_name) {
  const auto &it = s_shaders.find(_name);

  if(it == s_shaders.end()) {
    std::cout << "[RENDERER ERROR]: Attempted to get shader that has not been loaded\n";
    return nullptr;
  }

  return &it->second;
}


void Renderer::UpdateViewMatrix() {
  if(camera == nullptr) {
    std::cout << "[Renderer Error]: Attempted to update view matrix while camera is nullptr\n";
    return;
  }

  viewMatrix = camera->getViewMatrix();
  for(const auto &[name, shader] : s_shaders) {
    glUseProgram(shader);
    shader.SetUniform("view", viewMatrix);
  }
  glUseProgram(0);
}
void Renderer::UpdateProjectionMatrix() {
  if(camera == nullptr) {
    std::cout << "[Renderer Error]: Attempted to update projection matrix while camera is nullptr\n";
    return;
  }

  projectionMatrix = camera->getProjectionMatrix(windowSize);
  for(const auto &[name, shader] : s_shaders) {
    glUseProgram(shader);
    shader.SetUniform("projection", projectionMatrix);
  }
  glUseProgram(0);
}
void Renderer::UpdateWindowSize() {
  const sf::Vector2u sf_windowSize = window->getSize();
  windowSize = glm::ivec2(sf_windowSize.x, sf_windowSize.y);

  window->setView(sf::View(sf::FloatRect{{0,0}, sf::Vector2f{sf_windowSize}}));
  glViewport(0,0, windowSize.x, windowSize.y);
  UpdateProjectionMatrix();
}
void Renderer::UpdateDynamicLighting() {
  const int lightCount = lights.size();

  if(lightCount == 0) {
    std::cout << "[Renderer Error]: Attempted to update dynamic lighting with no light instances\n";
    return;
  }


  Shader &LitShader = s_shaders["Lit"];
  glUseProgram(LitShader);
  LitShader.SetUniform("ambientLight", ambientLight);
  LitShader.SetUniform("lightCount", lightCount);

  for(int i = 0; i < lightCount; i++) {
    const std::string prefix = "lights[" + std::to_string(i) + "].";
    LitShader.SetUniform(prefix+"position", lights[i].position);
    LitShader.SetUniform(prefix+"color", lights[i].color);
    LitShader.SetUniform(prefix+"radius", lights[i].radius);
  }
  glUseProgram(0);
}


void Renderer::clear() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void Renderer::display() {
  window->display();
}

void Renderer::ClearDepthBuffer() {
  glClear(GL_DEPTH_BUFFER_BIT);
}


void Renderer::SetState(RenderState _state) {
  switch(_state) {
    case RenderState::UI:
      window->resetGLStates();

      ClearDepthBuffer();

      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      glUseProgram(0);
      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      break;

    case RenderState::OpenGL:
      glEnable(GL_DEPTH_TEST);

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      glFrontFace(GL_CW);

      glPolygonMode(GL_FRONT_AND_BACK, WireframeMode? GL_LINE : GL_FILL);
      break;
  }
}
void Renderer::SetCamera(Camera *_camera) {
  camera = _camera;

  if(camera == nullptr) return;

  UpdateViewMatrix();
  UpdateProjectionMatrix();
}
void Renderer::SetShader(GLuint _program) {
  glUseProgram(_program);
}


void Renderer::ToggleWireframeMode(bool _enable) {
  WireframeMode = _enable;
  glPolygonMode(GL_FRONT_AND_BACK, _enable? GL_LINE : GL_FILL);
}


void Renderer::initGL() {
  GLenum err = glewInit();
  if(err != GLEW_OK) std::cout << "GLEW init error: " << glewGetErrorString(err) << '\n';

  Renderer::SetState(RenderState::OpenGL);
  glClearColor(.1,.1,.15, 1);
}