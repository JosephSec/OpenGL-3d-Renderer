#pragma once

#include <filesystem>
#include <map>

#include <Engine/Renderer/Camera.hpp>
#include <Engine/Renderer/MeshRenderer.hpp>


class Editor {
public:
  static std::filesystem::path primitiveMeshFolder;
  static std::map<std::string, std::filesystem::path> primitiveMeshPaths;

  static std::vector<Camera*> cameras;

  static Camera *camera;
  static float sensitivity;
  static float slowModeSpeed;
  static float fastModeSpeed;

  static Mesh worldGridMesh;
  static MeshRenderer worldGridRenderer;

  static Camera *playModeCamera;
  // static bool playMode;


  static void init();
  static void update();
  static void draw();
  static void end();


  static void TogglePlayMode();


  static bool SaveMeshPrimitive(const Mesh &_mesh, const std::string &_name);
  static bool LoadMeshPrimitive(Mesh &_mesh, const std::string &_name);

  static void RandomizeMeshColors(Mesh &_mesh, const std::vector<glm::vec4> &_colors);
};