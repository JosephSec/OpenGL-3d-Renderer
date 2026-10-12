#include <System.hpp>
#include <User.hpp>

#include <Renderer/Core.hpp>
#include <Renderer/Component/MeshRenderer.hpp>
#include <Renderer/Component/Camera.hpp>
#include <Renderer/Pipeline/Scene.hpp>
#include <Renderer/Pipeline/MeshHelper.hpp>
#include <Renderer/Pipeline/Skybox.hpp>
#include <Renderer/Pipeline/PostProcess.hpp>

#include <random>
#include <chrono>
#include <sstream>
#include <iostream>



int main(int argc, char *argv[]) {
  System::init();
  Renderer::Core::init();
  Renderer::Core::window->setVerticalSyncEnabled(true);
  // Renderer::Core::window->setFramerateLimit(60);
  // Renderer::Core::SetClearColor({0,0,0,0});

  Renderer::Camera *camera = new Renderer::Camera();
  Renderer::Core::camera = camera;

  Renderer::Shader unlitShader("Unlit");
  Renderer::Core::CacheRenderUniforms(&unlitShader);

  Renderer::Shader litShader("Lit");
  Renderer::Core::CacheRenderUniforms(&litShader);
  
  Renderer::Core::UpdateViewMatrix();
  Renderer::Core::UpdateWindowSize();


  Renderer::Shader skyboxShader("Skybox");
  Renderer::Skybox skybox(256);

  Renderer::PostProcess post(1920,1080);
  Renderer::Shader postShader("PostProcess");


  Renderer::Scene activeScene; {
    activeScene.meshes.push_back(std::move(Renderer::MeshHelper::GenerateCube()));
    {
      Renderer::Mesh boxMesh = std::move(Renderer::MeshHelper::GenerateCube());
      std::vector<unsigned int> indices = boxMesh.getIndices();
      for(size_t i = 0; i < indices.size() / 3; i++) {
        const size_t start = i * 3;

        std::swap(indices[start + 0], indices[start + 1]);
      }
      boxMesh.setIndices(indices);
      boxMesh.editVertices([](std::vector<Renderer::Vertex> &_vertices) {
        for(Renderer::Vertex &vertex : _vertices) vertex.normal = -vertex.normal;
      });

      activeScene.meshes.push_back(std::move(boxMesh));
    }

    activeScene.objects.push_back(Renderer::SceneObject{
      "Cube",
      Renderer::MeshRenderer(&activeScene.meshes[0], &litShader),
      Renderer::Transform({0,0,0})
    });
    activeScene.objects.push_back(Renderer::SceneObject{
      "Box",
      Renderer::MeshRenderer(&activeScene.meshes[1], &litShader),
      Renderer::Transform({0,0,0}, glm::quat(), {10,10,10})
    });

    activeScene.ambientLighting = {1,1,1, .01f};
    activeScene.lights = {
      new Renderer::PointLight({2,2,-2}, {1,1,1,1}, 15),
    };
  }

  Renderer::Core::UpdateDynamicLighting(&litShader, activeScene);


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

      static float timeSinceStart = 0;
      timeSinceStart += System::deltaTime;

      postShader.use();
      postShader.setUniform("uTime", timeSinceStart);
      postShader.setUniform("uView", Renderer::Core::camera->getViewMatrix());
      postShader.setUniform("uProj", Renderer::Core::camera->getProjectionMatrix(Renderer::Core::windowSize));
      postShader.setUniform("uCameraWorldPos", Renderer::Core::camera->transform.position);
      postShader.setUniform("uViewSize", glm::vec2{Renderer::Core::windowSize});
    } //Update

    { //Render
      Renderer::Core::clear();
      post.begin(Renderer::Core::windowSize.x, Renderer::Core::windowSize.y);

      skybox.draw(&skyboxShader);

      activeScene.draw();

      if(activeScene.lights.empty() == false) {
        Renderer::Core::ClearDepth();

        Renderer::Mesh sphereMesh = std::move(Renderer::MeshHelper::GenerateUVSphere(8,8, .25f));
        Renderer::MeshRenderer meshRenderer(&sphereMesh, &unlitShader);

        for(const Renderer::Light *light : activeScene.lights) {
          sphereMesh.editVertices([&](std::vector<Renderer::Vertex> &_vertices) {
            for(Renderer::Vertex &vertex : _vertices) vertex.color = glm::vec4{glm::vec3{light->color}, 1};
          });

          meshRenderer.draw(Renderer::Transform(light->position));
        }
      }

      Renderer::Core::ExecuteDrawCommands();

      post.end(&postShader, Renderer::Core::windowSize.x,Renderer::Core::windowSize.y);
      Renderer::Core::display();
    } //Render
  }

  Renderer::Core::end();

  return 0;
}