#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include <Renderer/Core/API.hpp>

#include <Renderer/Component/Transform.hpp>
#include <Renderer/Component/MeshRenderer.hpp>
#include <Renderer/Component/Light.hpp>

#include <Renderer/Core/Shader.hpp>


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
    Scene(int _shadowSize = 1024);

    void draw() const;
    void drawShadows() const;


    bool SaveToFolder(const std::filesystem::path &_path) const;


    std::vector<Mesh> meshes;
    std::vector<SceneObject> objects;

    glm::vec4 ambientLighting = glm::vec4(0);
    std::vector<Light*> lights;
  };
}