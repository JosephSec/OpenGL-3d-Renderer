#pragma once

#include <filesystem>

#include <Engine/Renderer/Camera.hpp>
#include <Engine/Renderer/MeshRenderer.hpp>


class Editor {
public:
  static Camera *camera;
  static float sensitivity;
  static float slowModeSpeed;
  static float fastModeSpeed;

  static MeshRenderer worldGridRenderer;
  static Mesh worldGridMesh;

  static std::vector<Camera*> cameras;


  static void init();
  static void update();
  static void draw();
  static void end();

  static void SaveMeshPrimitive(const Mesh &_mesh, const std::filesystem::path &_path);
  static bool LoadMeshPrimitive(Mesh &_mesh, const std::filesystem::path &_path);
};