#pragma once

#include <glm/vec2.hpp>

#include <SFML/Graphics.hpp>

#include <Renderer/Core/API.hpp>

#include <Renderer/Component/Camera.hpp>
#include <Renderer/Component/MeshRenderer.hpp>


namespace Renderer {
  enum State {
    World,
    UI
  };

  class RENDER3D_API Core {
  public:
    static sf::RenderWindow *window;
    static glm::ivec2 windowSize;

    static Camera *camera;


    static void init();
    static void end();


    static void clear();
    static void ClearColor();
    static void ClearDepth();
    static void SetClearColor(const glm::vec4 _color);


    static inline void display() {
      window->display();
    }


    static void UpdateWindowSize();
    static void UpdateProjectionMatrix();
    static void UpdateViewMatrix();

    static void QueueDraw(const MeshRenderer *_meshRenderer, const Transform &_transform, uint8_t _pass);
    static void ExecuteDrawCommands();

    static void CacheRenderUniforms(const Shader *_shader);

    static void ToggleWireframeMode(bool _state = !s_wireframeMode);


  private:
    struct ShaderRenderUniforms {
    public:
      const Shader *shader;
      Shader::UniformLocation projLocation;
      Shader::UniformLocation viewLocation;
      Shader::UniformLocation windowSizeLocation;
    };


    static std::vector<DrawCommand> s_drawCommands;

    static std::vector<ShaderRenderUniforms> s_shaderRenderUniforms;

    static bool s_wireframeMode;


    static glm::ivec2 GetWindowSize();
  };
}