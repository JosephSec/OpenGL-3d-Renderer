#include <System.hpp>
#include <User.hpp>

#include <Renderer/Core.hpp>
#include <Renderer/Component/MeshRenderer.hpp>
#include <Renderer/Component/Camera.hpp>
#include <Renderer/Pipeline/Scene.hpp>
#include <Renderer/Pipeline/MeshHelper.hpp>

#include <random>
#include <chrono>
#include <sstream>
#include <iostream>



int main(int argc, char *argv[]) {
  System::init();
  Renderer::Core::init();
  Renderer::Core::window->setVerticalSyncEnabled(true);

  Renderer::Camera *camera = new Renderer::Camera();
  Renderer::Core::camera = camera;

  Renderer::Shader unlitShader("Unlit");
  Renderer::Core::CacheRenderUniforms(&unlitShader);
  
  Renderer::Core::UpdateViewMatrix();
  Renderer::Core::UpdateWindowSize();


  Renderer::Scene scene;

  scene.meshes; {
    { //Triangle Mesh
      Renderer::Mesh triangleMesh; {
        triangleMesh.setVertices({
          Renderer::Vertex{{-.5f,-.5f,0}, {1,0,0,1}},
          Renderer::Vertex{{0.0f, .5f,0}, {0,1,0,1}},
          Renderer::Vertex{{ .5f,-.5f,0}, {0,0,1,1}},
        });
        triangleMesh.setIndices({0,1,2});
      }

      scene.meshes.push_back(std::move(triangleMesh));
    } //Triangle Mesh
    scene.meshes.push_back(std::move(Renderer::MeshHelper::GenerateCircle()));
    scene.meshes.push_back(std::move(Renderer::MeshHelper::GenerateCube()));
    scene.meshes.push_back(std::move(Renderer::MeshHelper::GenerateCylinder()));
    scene.meshes.push_back(std::move(Renderer::MeshHelper::GenerateQuad()));
    scene.meshes.push_back(std::move(Renderer::MeshHelper::GeneratePlane(glm::vec2(1), glm::ivec2(10))));
    scene.meshes.push_back(std::move(Renderer::MeshHelper::GeneratePyramid()));
    scene.meshes.push_back(std::move(Renderer::MeshHelper::GenerateUVSphere()));
  }
  scene.objects = {
    Renderer::SceneObject{
      "Triangle",
      Renderer::MeshRenderer(&scene.meshes[0], &unlitShader),
      Renderer::Transform({0,0,0})
    }
  };
  for(int i = 1; i < scene.meshes.size(); i++) {
    scene.objects.push_back(Renderer::SceneObject{
      std::to_string(i),
      Renderer::MeshRenderer(&scene.meshes[i], &unlitShader),
      Renderer::Transform({i * 1.5, 0, 0})
    });
  }

  scene.SaveToFolder(System::PATH+"/assets/scenes/mesh_instancing");


  {
    static constexpr size_t instanceCount = 1000;

    std::mt19937 gen(std::chrono::high_resolution_clock().now().time_since_epoch().count());
    std::uniform_real_distribution<float> randPos(-50,50);
    std::uniform_real_distribution<float> randSpin(-180, 180);
    std::uniform_real_distribution<float> randSize(.25f, 5);
    
    for(size_t i = 0; i < scene.objects.size(); i++) {
      Renderer::SceneObject &object = scene.objects[i];

      std::vector<glm::mat4x4> instances(instanceCount);
      for(int j = 0; j < instanceCount; j++) {
        const glm::quat randomRot =
          glm::angleAxis(glm::radians<float>(randSpin(gen)), glm::vec3(0,1,0)) *
          glm::angleAxis(glm::radians<float>(randSpin(gen)), glm::vec3(1,0,0));

        instances[j] = Renderer::Transform(
          glm::vec3{randPos(gen), randPos(gen), randPos(gen)},
          randomRot,
          glm::vec3{randSize(gen), randSize(gen), randSize(gen)}
        ).getModelMatrix();
      }

      object.meshRenderer.setInstancingData(instances);
    }
  }


  while(Renderer::Core::window->isOpen()) {
    while(const auto &eventOpt = Renderer::Core::window->pollEvent()) {
      const auto &event = *eventOpt;

      if(event.is<sf::Event::Closed>()) Renderer::Core::window->close();
      else if(const auto *resized = event.getIf<sf::Event::Resized>()) Renderer::Core::UpdateWindowSize();
      else if(event.is<sf::Event::FocusLost>()) User::HandleFocusLost();

      else if(const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) User::HandleKeyPressed(keyPressed);
      else if(const auto *mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) User::HandleMouseButtonPressed(mouseButtonPressed);
      else if(const auto *mouseButtonReleased = event.getIf<sf::Event::MouseButtonReleased>()) User::HandleMouseButtonReleased(mouseButtonReleased);
    }

    { //Update
      System::update();
      User::update();
    } //Update

    { //Render
      Renderer::Core::clear();
      
      scene.draw();

      Renderer::Core::ExecuteDrawCommands();

      Renderer::Core::display();
    } //Render
  }

  Renderer::Core::end();

  return 0;
}