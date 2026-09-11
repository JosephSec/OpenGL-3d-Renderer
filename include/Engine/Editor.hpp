#pragma once

#include <Engine/EngineAPI.hpp>

#include <Engine/Renderer/Camera.hpp>
#include <Engine/Renderer/MeshRenderer.hpp>

#include <filesystem>
#include <map>


class ZENGINE_API Editor {
public:
  static std::filesystem::path PATH;
  static std::filesystem::path primitiveMeshFolder;
  static std::map<std::string, std::filesystem::path> primitiveMeshPaths;

  static float deltaTime;
  static glm::ivec2 mouseDelta;

  static bool PlayModePaused;
  static bool PlayModeFocused;
  static bool PlayModeGizmos;

  static std::vector<Camera*> cameras;

  static Camera *camera;
  static float sensitivity;
  static float slowModeSpeed;
  static float fastModeSpeed;

  static Mesh worldGridMesh;
  static MeshRenderer worldGridRenderer;


  static void init(const std::filesystem::path &_PATH);
  static void update(float _deltaTime, const glm::ivec2 _mouseDelta);
  static void draw();
  static void end();


  static void TogglePlayModePaused(bool _enable = !PlayModePaused);
  static void TogglePlayModeFocused(bool _enable = !PlayModeFocused);
  static void TogglePlayModeGizmos(bool _enable = !PlayModeGizmos);


  static bool IsActiveCamera(); //is the Editor::camera the active render target


  static bool SaveMeshPrimitive(const Mesh &_mesh, const std::string &_name);
  static bool LoadMeshPrimitive(Mesh &_mesh, const std::string &_name);

  static void RandomizeMeshColors(Mesh &_mesh, const std::vector<glm::vec4> &_colors);
};