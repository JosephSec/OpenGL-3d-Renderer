#include <Renderer/Pipeline/Scene.hpp>
using namespace Renderer;

#include <Renderer/Core.hpp>
#include <Renderer/Pipeline/MeshHelper.hpp>


#include <iostream>
#include <format>
#include <fstream>


void SceneObject::draw() const noexcept {
  if(meshRenderer.isInstanced() == true) meshRenderer.draw();
  else meshRenderer.draw(transform.getModelMatrix());
}


bool SceneObject::SaveToFile(const std::filesystem::path &_path) const {
  std::ofstream file(_path, std::ios::binary);
  if(!file.is_open()) {
    std::cout << "[SCENE OBJECT ERROR]: Failed to open file for writing: " << _path.string() << '\n';
    return false;
  }

  uint64_t meshAddress = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(meshRenderer.getMesh()));
  file.write(reinterpret_cast<const char*>(meshAddress), sizeof(uint64_t));

  const std::string shaderName = meshRenderer.getShader()->getName();
  const size_t shaderNameSize = shaderName.size();

  file.write(reinterpret_cast<const char*>(&shaderNameSize), sizeof(size_t));
  file.write(shaderName.c_str(), shaderNameSize);

  file.write(reinterpret_cast<const char*>(&transform), sizeof(Transform));


  file.close();
  return true;
}


void Scene::draw() const {
  skybox.draw(skyboxShader);

  for(const SceneObject &object : objects) object.draw();
}
void Scene::drawShadows() const {
  for(const SceneObject &object : objects) {
    if(object.meshRenderer.castShadows == false) continue;
    object.draw();
  }
}


bool Scene::SaveToFolder(const std::filesystem::path &_path) const {
  if(std::filesystem::exists(_path)) std::filesystem::remove_all(_path);
  std::filesystem::create_directories(_path);

  std::ofstream file(_path/"scene.data", std::ios::binary);
  if(!file.is_open()) {
    std::cout << "[SCENE ERROR]: Failed to open file for writing: " << _path.string() << '\n';
    return false;
  }

  { //Save Meshes
    const size_t meshCount = meshes.size();
    file.write(reinterpret_cast<const char*>(&meshCount), sizeof(size_t));

    const std::filesystem::path meshesFolder = _path/"meshes";
    if(std::filesystem::exists(meshesFolder) == false) {
      std::filesystem::create_directories(meshesFolder);
    }

    for(size_t i = 0; i < meshCount; i++) {
      const std::string addressStr = std::format("{}", static_cast<const void*>(&meshes[i]));
      MeshHelper::SaveMesh(meshes[i], meshesFolder/(addressStr + ".mesh"));
    }
  } //Save Meshes
  { //Save Scene Objects
    const size_t objectCount = meshes.size();
    file.write(reinterpret_cast<const char*>(&objectCount), sizeof(size_t));

    const std::filesystem::path objectsFolder = _path/"objects";
    if(std::filesystem::exists(objectsFolder) == false) {
      std::filesystem::create_directories(objectsFolder);
    }

    file << '\n';
    for(size_t i = 0; i < objectCount; i++) {
      objects[i].SaveToFile(objectsFolder/(objects[i].name + ".object"));
      file << objects[i].name << '\n';
    }
  } //Save Scene Objects

  file.close();
  return true;
}