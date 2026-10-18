#include <Renderer/Core.hpp>
using namespace Renderer;

#include <Renderer/Pipeline/Scene.hpp>


sf::RenderWindow *Core::window;
glm::ivec2 Core::windowSize;

Camera *Core::camera;


void Core::init(const std::string &_winName, bool _enableVSync) {
  window = new sf::RenderWindow(sf::VideoMode{{800,600}}, _winName);
  window->setVerticalSyncEnabled(_enableVSync);
  windowSize = GetWindowSize();


  glewInit();

  glEnable(GL_DEPTH_TEST);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CW);

  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
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

void Core::UpdateDynamicLighting(const Shader *_shader, const Scene &_scene) {
  const size_t lightCount = _scene.lights.size();

  _shader->use();
  _shader->setUniform("uAmbientLight", _scene.ambientLighting);
  
  _shader->setUniform("uLightCount", static_cast<int>(lightCount));
  for(size_t i = 0; i < lightCount; i++) {
    _scene.lights[i]->setUniforms(_shader, i);
  }

  _shader->setUniform("uViewPoint", camera->transform.position);

  _shader->setUniform("uMaterial.diffuseTex", 0);
  _shader->setUniform("uMaterial.useDiffuseTex", false);
  _shader->setUniform("uMaterial.baseColor", glm::vec3(1));
  _shader->setUniform("uMaterial.surface", glm::vec2{0.5f,0.0f});
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
}
void Core::ClearDrawCommands() {
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

void Core::SetScenePolygonMode(GLenum _mode) {
  s_scenePolygonMode = _mode;
}
GLenum Core::GetScenePolygonMode() {
  return s_scenePolygonMode;
}

void Core::SetPolygonMode(GLenum _mode) {
  glPolygonMode(GL_FRONT_AND_BACK, _mode);
  StateCache::Invalidate();
}
GLenum Core::GetPolygonMode() {
  GLint mode[2];
  glGetIntegerv(GL_POLYGON_MODE, mode);
  return mode[0];
}

void Core::SetDepthMask(bool _mask) {
  glDepthMask(_mask? GL_TRUE : GL_FALSE);
  StateCache::Invalidate();
}
bool Core::GetDepthMask() {
  GLboolean output;
  glGetBooleanv(GL_DEPTH_WRITEMASK, &output);
  return output == GL_TRUE;
}

void Core::SetDepthTest(bool _state) {
  if(_state) glEnable(GL_DEPTH_TEST);
  else       glDisable(GL_DEPTH_TEST);
  StateCache::Invalidate();
}
bool Core::GetDepthTest() {
  GLboolean output;
  glGetBooleanv(GL_DEPTH_TEST, &output);
  return output == GL_TRUE;
}

void Core::SetDepthFunc(GLenum _func) {
  glDepthFunc(_func);
  StateCache::Invalidate();
}
GLenum Core::GetDepthFunc() {
  GLint output;
  glGetIntegerv(GL_DEPTH_FUNC, &output);
  return static_cast<GLenum>(output);
}

void Core::SetState(StateType _state) {
  glUseProgram(0);
  glBindVertexArray(0);

  switch(_state) {
    default:
    case StateType::Scene: {
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_COLOR_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      SetState(StateBuffer::Scene());
      break;
    }
    case StateType::UI: {
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_COLOR_ARRAY);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);

      SetState(StateBuffer::UI());
      break;
    }
  }
}
void Core::SetState(const StateBuffer &_stateBuf) {
  StateCache::Apply(_stateBuf);
}


//private
std::vector<DrawCommand> Core::s_drawCommands;

std::vector<Core::ShaderRenderUniforms> Core::s_shaderRenderUniforms;

GLenum Core::s_scenePolygonMode = GL_FILL;


glm::ivec2 Core::GetWindowSize() {
  return glm::ivec2{window->getSize().x, window->getSize().y};
}
//private
