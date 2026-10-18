#pragma once

#include <glm/vec2.hpp>

#include <SFML/Graphics.hpp>

#include <Renderer/Core/API.hpp>

#include <Renderer/Core/States.hpp>

#include <Renderer/Component/Camera.hpp>
#include <Renderer/Component/MeshRenderer.hpp>
#include <Renderer/Component/Light.hpp>

#include <Renderer/Pipeline/Scene.hpp>


namespace Renderer {
  class RENDER3D_API Core {
  public:
    static sf::RenderWindow *window;
    static glm::ivec2 windowSize;

    static Camera *camera;


    static void init(const std::string &_winName = "3D Rendering Library", bool _enableVSync = true);
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

    static void UpdateDynamicLighting(const Shader *_shader, const Scene &_scene);


    static void QueueDraw(const MeshRenderer *_meshRenderer, const Transform &_transform, uint8_t _pass);
    static void ExecuteDrawCommands();
    static void ClearDrawCommands();

    static void CacheRenderUniforms(const Shader *_shader);

    static void SetScenePolygonMode(GLenum _mode);
    static GLenum GetScenePolygonMode();

    static void SetPolygonMode(GLenum _mode);
    static GLenum GetPolygonMode();

    static void SetDepthMask(bool _mask);
    static bool GetDepthMask();

    static void SetDepthTest(bool _state);
    static bool GetDepthTest();

    static void SetDepthFunc(GLenum _func);
    static GLenum GetDepthFunc();

    static void SetState(StateType _state);
    static void SetState(const StateBuffer &_stateBuf);


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

    static GLenum s_scenePolygonMode;


    static glm::ivec2 GetWindowSize();
  };
}