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
  Renderer::Skybox::SplitToFaces(System::PATH+"/assets/skyboxs/DimTwilight/cubemap.png");
  skybox.loadFromFolder(System::PATH+"/assets/skyboxs/DimTwilight");

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
      new Renderer::PointLight({2,2,-2}, {1,0,0,1}, 15),
    };
  }

  Renderer::Core::UpdateDynamicLighting(&litShader, activeScene);


  // ---- Point light shadows ----
  constexpr int   SHADOW_SIZE = 1024;
  constexpr float SHADOW_FAR  = 30.0f;     // must cover the farthest surface the light reaches
  constexpr GLuint SHADOW_UNIT = 5;        // free unit (skybox uses 0, post uses 0 and 1)

  Renderer::Shader shadowDepthShader("ShadowDepth");
  Renderer::Core::CacheRenderUniforms(&shadowDepthShader);

  Renderer::Cubemap shadowCube(SHADOW_SIZE, GL_DEPTH_COMPONENT32F, 1);
  glTextureParameteri(shadowCube.id(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);   // we compare distances ourselves
  glTextureParameteri(shadowCube.id(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTextureParameteri(shadowCube.id(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(shadowCube.id(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTextureParameteri(shadowCube.id(), GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTextureParameteri(shadowCube.id(), GL_TEXTURE_COMPARE_MODE, GL_NONE);

  GLuint shadowFbo = 0;
  glCreateFramebuffers(1, &shadowFbo);
  glNamedFramebufferDrawBuffer(shadowFbo, GL_NONE);
  glNamedFramebufferReadBuffer(shadowFbo, GL_NONE);

  // Same meshes/transforms as the scene objects, but drawn with the depth shader
  struct ShadowCaster { Renderer::MeshRenderer renderer; Renderer::Transform transform; };
  std::vector<ShadowCaster> shadowCasters;
  shadowCasters.push_back({Renderer::MeshRenderer(&activeScene.meshes[0], &shadowDepthShader), Renderer::Transform({0,0,0})});
  shadowCasters.push_back({Renderer::MeshRenderer(&activeScene.meshes[1], &shadowDepthShader), Renderer::Transform({0,0,0}, glm::quat(), {10,10,10})});

  // Standard cubemap face orientations (+X, -X, +Y, -Y, +Z, -Z)
  const glm::vec3 faceDir[6] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
  const glm::vec3 faceUp [6] = {{0,-1,0},{0,-1,0},{0,0,1},{0,0,-1},{0,-1,0},{0,-1,0}};
  

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

      activeScene.lights[0]->position = {glm::cos(timeSinceStart * .5f) * 2, glm::cos(timeSinceStart * .33f) * 2, glm::sin(timeSinceStart * .5f) * 2};
      Renderer::Core::UpdateDynamicLighting(&litShader, activeScene);

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

        // ---- Shadow pass (before post.begin, because that binds the post FBO) ----
        if(!activeScene.lights.empty()) {
          const glm::vec3 lightPos = activeScene.lights[0]->position;
          const glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.05f, SHADOW_FAR);

          Renderer::StateCache::PushStates();

          glBindFramebuffer(GL_FRAMEBUFFER, shadowFbo);
          glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);
          glDisable(GL_SCISSOR_TEST);
          glEnable(GL_DEPTH_TEST);
          glDepthFunc(GL_LESS);
          glDepthMask(GL_TRUE);
          Renderer::StateCache::Invalidate();      // raw GL above bypassed the cache

          shadowDepthShader.use();
          shadowDepthShader.setUniform("uLightPos", lightPos);
          shadowDepthShader.setUniform("uShadowFar", SHADOW_FAR);

          for(int f = 0; f < 6; f++) {
            glNamedFramebufferTextureLayer(shadowFbo, GL_DEPTH_ATTACHMENT, shadowCube.id(), 0, f);
            glClear(GL_DEPTH_BUFFER_BIT);

            shadowDepthShader.setUniform("uLightSpace", proj * glm::lookAt(lightPos, lightPos + faceDir[f], faceUp[f]));
            for(ShadowCaster &c : shadowCasters) c.renderer.draw(c.transform);
            Renderer::Core::ExecuteDrawCommands();
          }

          glBindFramebuffer(GL_FRAMEBUFFER, 0);
          glViewport(0, 0, Renderer::Core::windowSize.x, Renderer::Core::windowSize.y);
          Renderer::StateCache::Invalidate();
          Renderer::StateCache::PullStates();

          // Feed the lit shader
          shadowCube.bind(SHADOW_UNIT);
          litShader.use();
          litShader.setUniform("uShadowCube", (int)SHADOW_UNIT);
          litShader.setUniform("uShadowLightPos", lightPos);
          litShader.setUniform("uShadowFar", SHADOW_FAR);
          litShader.setUniform("uHasShadow", true);
        }

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

      Renderer::Core::SetState(Renderer::StateType::UI); {
        Renderer::StateCache::PushStates();
        Renderer::Core::window->resetGLStates();
        Renderer::StateCache::Invalidate();

        sf::RectangleShape shape({25,25});
        shape.setPosition({25,25});
        shape.setFillColor(sf::Color::Red);
        Renderer::Core::window->draw(shape);

        Renderer::StateCache::PullStates();
      }

      Renderer::Core::display();
    } //Render
  }

  Renderer::Core::end();

  return 0;
}