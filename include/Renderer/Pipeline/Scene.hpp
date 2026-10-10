#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include <Renderer/Core/API.hpp>

#include <Renderer/Component/Transform.hpp>
#include <Renderer/Component/MeshRenderer.hpp>
#include <Renderer/Component/Light.hpp>


namespace Renderer {
  struct RENDER3D_API SceneObject {
  public:
    void draw() const noexcept;

    bool SaveToFile(const std::filesystem::path &_path) const;


    std::string name;

    MeshRenderer meshRenderer;
    Transform transform;
  };

  class RENDER3D_API Scene {
  public:
    void draw() const;

    bool SaveToFolder(const std::filesystem::path &_path) const;


    std::vector<Mesh> meshes;
    std::vector<SceneObject> objects;
    std::vector<Light*> lights;
  };
}