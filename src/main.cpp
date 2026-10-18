#include <System.hpp>
#include <User.hpp>

#include <Renderer/Core.hpp>
#include <Renderer/Core/ShadowMap.hpp>
#include <Renderer/Component/MeshRenderer.hpp>
#include <Renderer/Component/Camera.hpp>
#include <Renderer/Pipeline/Scene.hpp>
#include <Renderer/Pipeline/MeshHelper.hpp>
#include <Renderer/Pipeline/Skybox.hpp>
#include <Renderer/Pipeline/PostProcess.hpp>

#include <CyberScene.inl>

#include <map>
#include <optional>
#include <iostream>



static Renderer::Camera *camera = nullptr;
static Renderer::Shader unlitShader;
static Renderer::Shader litShader;
static Renderer::Shader shadowShader;

static Renderer::Shader postShader;
static Renderer::Shader skyboxShader;


static std::optional<Renderer::Scene> scene;
static std::optional<Renderer::PostProcess> postProcess;
static std::optional<Renderer::ShadowMap> shadows;


static void init() {
  System::init();
  Renderer::Core::init("3D Rendering Library", true);

  camera = new Renderer::Camera();
  Renderer::Core::camera = camera;


  unlitShader = Renderer::Shader("Unlit");
  litShader = Renderer::Shader("Lit");
  shadowShader = Renderer::Shader("ShadowDepth");

  Renderer::Core::CacheRenderUniforms(&unlitShader);
  Renderer::Core::CacheRenderUniforms(&litShader);
  Renderer::Core::CacheRenderUniforms(&shadowShader);

  postShader = Renderer::Shader("PostProcess");
  skyboxShader = Renderer::Shader("Skybox");
  
  Renderer::Core::UpdateViewMatrix();
  Renderer::Core::UpdateWindowSize();

  
  postProcess.emplace(1920,1080);
  shadows.emplace();

  CreateCyberScene();
}
int main(int argc, char *argv[]) {
  init();


  static bool doPost = true;
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

      const glm::vec3 dronePos = glm::vec3{
        glm::cos(timeSinceStart * .45f) * 3.f, glm::sin(timeSinceStart * 1.1f) * .25f, glm::sin(timeSinceStart * .45f) * 3.f};
      scene->objects[droneObject].transform.position = dronePos;
      scene->lights[0]->position = dronePos - glm::vec3{0, .4f, 0};

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

        shadows->drawShadows(*scene, &shadowShader);
        shadows->loadToShader(scene->lights[0]->position, &litShader);

        Renderer::StateCache::PushStates();
        if(doPost) postProcess->begin(Renderer::Core::windowSize.x, Renderer::Core::windowSize.y);

        scene->draw();

        { //Draw Gizmos
          Renderer::StateCache::PushStates();
          Renderer::Core::SetDepthMask(false);
          Renderer::Core::SetDepthTest(false);
          Renderer::StateCache::Invalidate();
          

          if(scene->lights.empty() == false) {
            Renderer::Mesh sphereMesh = std::move(Renderer::MeshHelper::GenerateUVSphere(8,8, .25f));
            Renderer::MeshRenderer meshRenderer(&sphereMesh, &unlitShader);

            for(const Renderer::Light *light : scene->lights) {
              sphereMesh.editVertices([&](std::vector<Renderer::Vertex> &_vertices) {
                for(Renderer::Vertex &vertex : _vertices) vertex.color = glm::vec4{glm::vec3{light->color}, 1};
              });

              meshRenderer.draw(Renderer::Transform(light->position));
            }
          }

          Renderer::Core::SetDepthFunc(GL_LEQUAL);
          Renderer::StateCache::PullStates();
        } //Draw Gizmos
          
          Renderer::Core::ExecuteDrawCommands();

        if(doPost) postProcess->end(&postShader, Renderer::Core::windowSize.x,Renderer::Core::windowSize.y);      
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