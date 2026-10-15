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
#include <map>


int main(int argc, char *argv[]) {
  System::init();
  Renderer::Core::init();
  Renderer::Core::window->setVerticalSyncEnabled(true);

  Renderer::Camera *camera = new Renderer::Camera();
  Renderer::Core::camera = camera;
  camera->transform = Renderer::Transform({-2,2,-2}, Renderer::Transform::LookAtRotation({-2,2,-2}, {0,0,0}, {0,1,0}));

  Renderer::Shader unlitShader("Unlit");
  Renderer::Core::CacheRenderUniforms(&unlitShader);

  Renderer::Shader litShader("Lit");
  Renderer::Core::CacheRenderUniforms(&litShader);
  
  Renderer::Core::UpdateViewMatrix();
  Renderer::Core::UpdateWindowSize();


  Renderer::Shader skyboxShader("Skybox");
  Renderer::Skybox skybox;
  Renderer::Skybox::SplitToFaces(System::PATH+"/assets/skyboxs/OvercastSunset");
  skybox.loadFromFolder(System::PATH+"/assets/skyboxs/OvercastSunset");

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


  static bool doPost = false;
  while(Renderer::Core::window->isOpen()) {
    while(const auto &eventOpt = Renderer::Core::window->pollEvent()) {
      const auto &event = *eventOpt;

      if(event.is<sf::Event::Closed>()) Renderer::Core::window->close();
      else if(const auto *resized = event.getIf<sf::Event::Resized>()) Renderer::Core::UpdateWindowSize();
      else if(event.is<sf::Event::FocusLost>()) User::HandleFocusLost();

      else if(const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        User::HandleKeyPressed(keyPressed);
        if(keyPressed->code == sf::Keyboard::Key::Space) doPost = !doPost;
      }
      else if(const auto *mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) User::HandleMouseButtonPressed(mouseButtonPressed);
      else if(const auto *mouseButtonReleased = event.getIf<sf::Event::MouseButtonReleased>()) User::HandleMouseButtonReleased(mouseButtonReleased);
    }

    { //Update
      System::update();
      User::update();

      static float timeSinceStart = 0;
      timeSinceStart += System::deltaTime;

      postShader.use();            
      postShader.setUniform("uProj", Renderer::Core::camera->getProjectionMatrix(Renderer::Core::windowSize));
      postShader.setUniform("uView", Renderer::Core::camera->getViewMatrix());
      postShader.setUniform("uViewSize", glm::vec2{Renderer::Core::windowSize});
      postShader.setUniform("uTime", timeSinceStart);
      postShader.setUniform("uCameraWorldPos", Renderer::Core::camera->transform.position);
    } //Update

    { //Render      
      Renderer::Core::SetState(Renderer::StateType::Scene); {
        Renderer::Core::clear();

        Renderer::StateCache::PushStates();
        if(doPost) post.begin(Renderer::Core::windowSize.x, Renderer::Core::windowSize.y);

        skybox.draw(&skyboxShader);

        activeScene.draw();

        if(activeScene.lights.empty() == false) {
          Renderer::StateCache::PushStates();
          Renderer::Core::SetDepthFunc(GL_ALWAYS);
          Renderer::StateCache::Invalidate();

          Renderer::Mesh sphereMesh = std::move(Renderer::MeshHelper::GenerateUVSphere(8,8, .25f));
          Renderer::MeshRenderer meshRenderer(&sphereMesh, &unlitShader);

          for(const Renderer::Light *light : activeScene.lights) {
            sphereMesh.editVertices([&](std::vector<Renderer::Vertex> &_vertices) {
              for(Renderer::Vertex &vertex : _vertices) vertex.color = glm::vec4{glm::vec3{light->color}, 1};
            });

            meshRenderer.draw(Renderer::Transform(light->position));
          }

          Renderer::Core::SetDepthFunc(GL_LEQUAL);
          Renderer::StateCache::PullStates();
        }

        Renderer::Core::ExecuteDrawCommands();

        if(doPost) post.end(&postShader, Renderer::Core::windowSize.x,Renderer::Core::windowSize.y);      
        Renderer::StateCache::PullStates();
      }

      // Renderer::Core::SetState(Renderer::StateType::UI); {
      //   Renderer::StateCache::PushStates();
      //   Renderer::Core::window->resetGLStates();
      //   Renderer::StateCache::Invalidate();

      //   sf::RectangleShape shape({25,25});
      //   shape.setPosition({25,25});
      //   shape.setFillColor(sf::Color::Red);
      //   Renderer::Core::window->draw(shape);

      //   Renderer::StateCache::PullStates();
      // }

      Renderer::Core::display();
    } //Render
  }

  Renderer::Core::end();

  return 0;
}