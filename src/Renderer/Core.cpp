#include <Renderer/Core.hpp>
using namespace Renderer;


sf::RenderWindow *Core::window;
glm::ivec2 Core::windowSize;

Camera *Core::camera;


void Core::init() {
  window = new sf::RenderWindow(sf::VideoMode{{800,600}}, "3D Rendering Library");
  windowSize = GetWindowSize();


  glewInit();
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CW);
}
void Core::end() {
  delete window;
}


void Core::clear() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void Core::ClearColor() {
  glClear(GL_COLOR_BUFFER_BIT);
}
void Core::ClearDepth() {
  glClear(GL_DEPTH_BUFFER_BIT);
}
void Core::SetClearColor(const glm::vec4 _color) {
  glClearColor(_color.r, _color.g, _color.b, _color.a);
}


void Core::UpdateWindowSize() {
  windowSize = GetWindowSize();
  
  const glm::vec2 windowSize_f = glm::vec2{windowSize};
  for(const ShaderRenderUniforms &_renderUniforms : s_shaderRenderUniforms) {
    _renderUniforms.shader->use();
    _renderUniforms.shader->setUniform(_renderUniforms.windowSizeLocation, windowSize_f);
  }

  Renderer::Core::window->setView(sf::View{sf::FloatRect{{0,0}, sf::Vector2f{windowSize_f.x, windowSize_f.y}}});
  glViewport(0,0, windowSize.x,windowSize.y);
  UpdateProjectionMatrix();
}
void Core::UpdateProjectionMatrix() {
  const glm::mat4x4 projMatrix = camera->getProjectionMatrix(windowSize);
  
  for(const ShaderRenderUniforms &_renderUniform : s_shaderRenderUniforms) {
    _renderUniform.shader->use();
    _renderUniform.shader->setUniform(_renderUniform.projLocation, projMatrix);
  }
}
void Core::UpdateViewMatrix() {
  const glm::mat4x4 viewMatrix = camera->getViewMatrix();
  
  for(const ShaderRenderUniforms &_renderUniform : s_shaderRenderUniforms) {
    _renderUniform.shader->use();
    _renderUniform.shader->setUniform(_renderUniform.viewLocation, viewMatrix);
  }
}

void Core::QueueDraw(const MeshRenderer *_meshRenderer, const Transform &_transform, uint8_t _pass) {
  s_drawCommands.push_back(_meshRenderer->getDrawCommand(_transform, _pass));
}
void Core::ExecuteDrawCommands() {
  if(s_drawCommands.empty() == true) return;

  std::sort(s_drawCommands.begin(), s_drawCommands.end(), [](const DrawCommand &_a, const DrawCommand &_b) {
    return _a.sortKey < _b.sortKey;
  });

  const Mesh *mesh = nullptr;
  const Shader *shader = nullptr;

  for(const DrawCommand &drawCommand : s_drawCommands) {
    if(drawCommand.shader != shader) {
      shader = drawCommand.shader;
      shader->use();
    }
    if(drawCommand.mesh != mesh) {
      mesh = drawCommand.mesh;
      mesh->use();
    }

    shader->setUniform(drawCommand.modelLoc, drawCommand.model);
    mesh->draw();
  }

  s_drawCommands.clear();
}

void Core::CacheRenderUniforms(const Shader *_shader) {
  s_shaderRenderUniforms.push_back(ShaderRenderUniforms{
    _shader,
    _shader->getUniform("uProjection"),
    _shader->getUniform("uView"),
    _shader->getUniform("uWindowSize"),
  });
}

void Core::ToggleWireframeMode(bool _state) {
  s_wireframeMode = _state;
  glPolygonMode(GL_FRONT_AND_BACK, s_wireframeMode? GL_LINE : GL_FILL);
}


//private
std::vector<DrawCommand> Core::s_drawCommands;

std::vector<Core::ShaderRenderUniforms> Core::s_shaderRenderUniforms;

bool Core::s_wireframeMode = false;


glm::ivec2 Core::GetWindowSize() {
  return glm::ivec2{window->getSize().x, window->getSize().y};
}
//private
