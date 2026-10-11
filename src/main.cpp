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
  Renderer::Core::window->setVerticalSyncEnabled(false);
  Renderer::Core::window->setFramerateLimit(60);
  Renderer::Core::SetClearColor({0,0,0,0});

  Renderer::Camera *camera = new Renderer::Camera();
  Renderer::Core::camera = camera;

  Renderer::Shader unlitShader("Unlit");
  Renderer::Core::CacheRenderUniforms(&unlitShader);

  Renderer::Shader litShader("Lit");
  Renderer::Core::CacheRenderUniforms(&litShader);
  
  Renderer::Core::UpdateViewMatrix();
  Renderer::Core::UpdateWindowSize();


  Renderer::Scene meshShowcase; {
    meshShowcase.ambientLighting = glm::vec4(1);

    meshShowcase.meshes; {
      { //Triangle Mesh
        Renderer::Mesh triangleMesh; {
          triangleMesh.setVertices({
            Renderer::Vertex{{-.5f,-.5f,0}, {1,0,0,1}, {0,0,1}, { 0, 0}},
            Renderer::Vertex{{0.0f, .5f,0}, {0,1,0,1}, {0,0,1}, {.5, 1}},
            Renderer::Vertex{{ .5f,-.5f,0}, {0,0,1,1}, {0,0,1}, { 1, 0}},
          });
          triangleMesh.setIndices({0,1,2});
        }

        meshShowcase.meshes.push_back(std::move(triangleMesh));
      } //Triangle Mesh
      meshShowcase.meshes.push_back(std::move(Renderer::MeshHelper::GenerateCircle()));
      meshShowcase.meshes.push_back(std::move(Renderer::MeshHelper::GenerateCube()));
      meshShowcase.meshes.push_back(std::move(Renderer::MeshHelper::GenerateCylinder()));
      meshShowcase.meshes.push_back(std::move(Renderer::MeshHelper::GenerateQuad()));
      meshShowcase.meshes.push_back(std::move(Renderer::MeshHelper::GeneratePlane(glm::vec2(1), glm::ivec2(10))));
      meshShowcase.meshes.push_back(std::move(Renderer::MeshHelper::GeneratePyramid()));
      meshShowcase.meshes.push_back(std::move(Renderer::MeshHelper::GenerateUVSphere()));
    }
    meshShowcase.objects = {
      Renderer::SceneObject{
        "Triangle",
        Renderer::MeshRenderer(&meshShowcase.meshes[0], &litShader),
        Renderer::Transform({0,0,0})
      }
    };
    for(int i = 1; i < meshShowcase.meshes.size(); i++) {
      meshShowcase.objects.push_back(Renderer::SceneObject{
        std::to_string(i),
        Renderer::MeshRenderer(&meshShowcase.meshes[i], &litShader),
        Renderer::Transform({i * 1.5, 0, 0})
      });
    }
    meshShowcase.lights = {
      new Renderer::PointLight({ 0,0,2}, {1,1,1,1}, 15),
      new Renderer::PointLight({ 6,0,2}, {1,1,1,1}, 15),
      new Renderer::PointLight({12,0,2}, {1,1,1,1}, 15),
    };

    meshShowcase.objects[0].meshRenderer.backFaceCulling = false;
  }
  Renderer::Scene instancingShowcase; {
    instancingShowcase.meshes.push_back(std::move(Renderer::MeshHelper::GenerateUVSphere(8,8,.5f)));    
    // instancingShowcase.meshes[0].editVertices([&](std::vector<Renderer::Vertex> &_vertices) {
    //   const std::vector<glm::vec4> colors = {
    //     {1,0,0,1},
    //     {0,1,0,1},
    //     {0,0,1,1},

    //     {1,0,0,1},
    //     {0,0,1,1},
    //     {1,1,1,1},
    //   };

    //   for(int i = 0; i < _vertices.size(); i++) _vertices[i].color = colors[i % 6];
    // });
    
    instancingShowcase.objects.push_back(Renderer::SceneObject{
      "Cube",
      Renderer::MeshRenderer(&instancingShowcase.meshes[0], &litShader),
      Renderer::Transform()
    });


    instancingShowcase.ambientLighting = {1,1,1,.01f};
    instancingShowcase.lights = {
      new Renderer::PointLight({0,0,0}, {1,0,0,1}, 15),
      new Renderer::PointLight({0,0,0}, {0,1,0,1}, 15),
      new Renderer::PointLight({0,0,0}, {0,0,1,1}, 15),
    };
  }

  Renderer::Core::UpdateDynamicLighting(&litShader, instancingShowcase);


  if constexpr(false) { //Generate Random Instances
    static constexpr size_t instanceCount = 1000;

    std::mt19937 gen(std::chrono::high_resolution_clock().now().time_since_epoch().count());
    std::uniform_real_distribution<float> randPos(-25,25);
    std::uniform_real_distribution<float> randSpin(-180, 180);
    std::uniform_real_distribution<float> randSize(.25f, 5);
    
    for(size_t i = 0; i < meshShowcase.objects.size(); i++) {
      Renderer::SceneObject &object = meshShowcase.objects[i];

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
  else if constexpr(true) { //Generate Uniform Instances
    static constexpr size_t BOX_SIZE = 50;
    static constexpr glm::vec3 BOX_START = glm::vec3(-.5f) * static_cast<float>(BOX_SIZE);

    std::vector<glm::mat4x4> instances(BOX_SIZE * BOX_SIZE * BOX_SIZE);
    for(int z = 0; z < BOX_SIZE; z++) {
      for(int y = 0; y < BOX_SIZE; y++) {
        for(int x = 0; x < BOX_SIZE; x++) {
          instances[x + (y * BOX_SIZE) + (z * BOX_SIZE * BOX_SIZE)] = 
          Renderer::Transform(
            BOX_START + glm::vec3{x,y,z},
            glm::quat(),
            glm::vec3(.75f)
          ).getModelMatrix();
        }
      }
    }

    instancingShowcase.objects[0].meshRenderer.setInstancingData(instances);
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


      static float t = 0;
      t += System::deltaTime;

      static constexpr float LIGHT_MOVE_RADIUS = 15.0f;
      for(int i = 0; i < instancingShowcase.lights.size(); i++) {
        const float phiDif = i * glm::two_pi<float>() / static_cast<float>(instancingShowcase.lights.size());

        instancingShowcase.lights[i]->position = {
          glm::cos(t + phiDif) * LIGHT_MOVE_RADIUS,
          0, // glm::sin(t + phiDif) * LIGHT_MOVE_RADIUS,
          glm::sin(t + phiDif) * LIGHT_MOVE_RADIUS
        };
      }
      Renderer::Core::UpdateDynamicLighting(&litShader, instancingShowcase);

      // meshShowcase.lights[0]->position = {(1 - (glm::cos(t) + 1) / 2.0f) * 12.0f, 2,2};
      // Renderer::Core::UpdateDynamicLighting(&litShader, meshShowcase);

      if constexpr(true) { //Update Instances
        static constexpr int BOX_SIZE = 50;
        static constexpr glm::vec3 BOX_START = glm::vec3(-.5f) * static_cast<float>(BOX_SIZE);

        static constexpr float INSTANCE_SIZE_MIN = 0.1f;
        static constexpr float INSTANCE_SIZE_MAX = 1.0f;

        static float t = 0;
        t += System::deltaTime;

        static float waves[BOX_SIZE];
        for(int i = 0; i < BOX_SIZE; i++) waves[i] = (glm::cos(i + t) + 1) / 2.0f;

        static std::vector<glm::mat4x4> instances(BOX_SIZE * BOX_SIZE * BOX_SIZE);
        for(int z = 0; z < BOX_SIZE; z++) {
          for(int y = 0; y < BOX_SIZE; y++) {
            for(int x = 0; x < BOX_SIZE; x++) {
              const float sizeScalar = INSTANCE_SIZE_MIN + (INSTANCE_SIZE_MAX - INSTANCE_SIZE_MIN) *
                waves[x] * waves[y] * waves[z];

              instances[x + (y * BOX_SIZE) + (z * BOX_SIZE * BOX_SIZE)] = 
              Renderer::Transform(
                BOX_START + glm::vec3{x,y,z},
                glm::quat(),
                glm::vec3(sizeScalar)
              ).getModelMatrix();
            }
          }
        }

        instancingShowcase.objects[0].meshRenderer.setInstancingData(instances);
      } //Update Instances
    } //Update

    { //Render
      Renderer::Core::clear();

      instancingShowcase.draw();

      if(instancingShowcase.lights.empty() == false) {
        Renderer::Core::ClearDepth();

        Renderer::Mesh sphereMesh = std::move(Renderer::MeshHelper::GenerateUVSphere(8,8, .25f));
        Renderer::MeshRenderer meshRenderer(&sphereMesh, &unlitShader);

        for(const Renderer::Light *light : instancingShowcase.lights) {
          sphereMesh.editVertices([&](std::vector<Renderer::Vertex> &_vertices) {
            for(Renderer::Vertex &vertex : _vertices) vertex.color = glm::vec4{glm::vec3{light->color}, 1};
          });

          meshRenderer.draw(Renderer::Transform(light->position));
        }
      }

      Renderer::Core::ExecuteDrawCommands();

      Renderer::Core::display();
    } //Render
  }

  Renderer::Core::end();

  return 0;
}