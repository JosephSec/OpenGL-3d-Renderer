#pragma once

#include <map>

#include <SFML/Graphics/RenderWindow.hpp>

#include <Renderer/API.hpp>
#include <Renderer/Mesh.hpp>
#include <Renderer/MeshRenderer.hpp>
#include <Renderer/MeshHelper.hpp>
#include <Renderer/Shader.hpp>
#include <Renderer/Camera.hpp>
#include <Renderer/Light.hpp>


namespace Renderer {
  enum class State {
    OpenGL,
    UI
  };
  class HORDE3D_API Core {
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


    static void SetClearColor(const glm::vec4 _color);
    static void SetState(State _state);
    static void SetCamera(Camera *_camera);
    static void SetShader(GLuint _program);


    static void ToggleWireframeMode(bool _enable = !WireframeMode);

  private:
    static std::map<std::string, Shader> s_shaders;


    static void initGL();
  };
}