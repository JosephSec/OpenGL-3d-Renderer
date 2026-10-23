#include <System.hpp>
#include <User.hpp>

#include <Renderer/Core.hpp>
#include <Renderer/Core/Shader.hpp>
#include <Renderer/Core/Mesh.hpp>
#include <Renderer/Component/Camera.hpp>
#include <Renderer/Debug/Graph.hpp>
#include <Renderer/Hardware/GPU.hpp>
#include <Renderer/Hardware/Monitor.hpp>

#include <Editor/UI/PerformanceOverlay.hpp>
#include <Editor/UI/Button.hpp>

#include <random>
#include <chrono>

#include <iostream>


static bool wireframeMode = false;
static bool paused = false;
int main(int argc, char *argv[]) {
  System::init();
  Renderer::Core::init(true);
  Renderer::Core::window->setActive(true);

  Editor::UI::PerformanceOverlay::init();

  Renderer::Shader unlitShader("Unlit");

  sf::Font font(System::PATH+"/assets/Roboto.ttf");

  Renderer::Camera *camera = new Renderer::Camera();


  Renderer::Shader waterShader("Water");
  Renderer::Mesh water; {
    water();

    const glm::ivec2 grid = glm::ivec2{1200,1200};
    const float scale = .1f;

    std::vector<Renderer::Mesh::Vertex> vertices((grid.x + 1) * (grid.y + 1));
    for(int y = 0; y <= grid.y; y++) {
      for(int x = 0; x <= grid.x; x++) {
        vertices[x + y * (grid.x + 1)] = Renderer::Mesh::Vertex{
          glm::vec3{x - static_cast<float>(grid.x + 1) / 2.0f, 0, -(y - static_cast<float>(grid.y + 1) / 2.0f)} * scale,
          {0,0,1,1},
        };
      }
    }
    water.setVertices(vertices);

    std::vector<Renderer::Mesh::Index> indices(grid.x * grid.y * 6);
    for(int y = 0; y < grid.y; y++) {
      for(int x = 0; x < grid.x; x++) {
        const uint64_t start = (static_cast<uint64_t>(x) + static_cast<uint64_t>(y) * grid.x) * 6;

        const Renderer::Mesh::Index a = x + y * (grid.x + 1);
        const Renderer::Mesh::Index b = a + grid.x + 1;
        const Renderer::Mesh::Index c = b + 1;
        const Renderer::Mesh::Index d = a + 1;

        indices[start + 0] = a;
        indices[start + 1] = b;
        indices[start + 2] = c;

        indices[start + 3] = a;
        indices[start + 4] = c;
        indices[start + 5] = d;
      }
    }
    water.setIndices(indices);
  }


  std::cout << "Active Monitor Refresh Rate: " << Renderer::Monitor::GetCurrentMonitorRefreshRate() << '\n';
  std::cout << "GPU: " << Renderer::GPU::GetGPUName() << " | Available VRAM: " << Renderer::GPU::VRAMCapacity / (1024 * 1024) << "MB\n";
  std::cout << "Used VRAM: " << Renderer::GPU::VRAMInUse / 1000 << "KB | " << Renderer::GPU::GetUsedVRAM() << "%\n";
  while(Renderer::Core::window->isOpen()) {
    while(const auto &eventOpt = Renderer::Core::window->pollEvent()) {
      const auto &event = *eventOpt;

      if(event.is<sf::Event::Closed>()) Renderer::Core::window->close();
      else if(const auto *resized = event.getIf<sf::Event::Resized>()) Renderer::Core::UpdateWindowSize();
      else if(event.is<sf::Event::FocusLost>()) User::HandleFocusLost();

      else if(const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        User::HandleKeyPressed(keyPressed);
        if(keyPressed->code == sf::Keyboard::Key::Space) paused = !paused;
        else if(keyPressed->code == sf::Keyboard::Key::F1) Editor::UI::PerformanceOverlay::ToggleEnabled();
        else if(keyPressed->code == sf::Keyboard::Key::F2) {
          wireframeMode = !wireframeMode;
          glPolygonMode(GL_FRONT_AND_BACK, wireframeMode? GL_LINE : GL_FILL);
        }
      }
      else if(const auto *mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) User::HandleMouseButtonPressed(mouseButtonPressed);
      else if(const auto *mouseButtonReleased = event.getIf<sf::Event::MouseButtonReleased>()) User::HandleMouseButtonReleased(mouseButtonReleased);
    }

    { //Update
      System::update();

      { //Check if window moved
        static sf::Vector2i windowLastPosition = Renderer::Core::window->getPosition();
        const sf::Vector2i windowCurrentPosition = Renderer::Core::window->getPosition();
        if(windowCurrentPosition != windowLastPosition) {
          windowLastPosition = windowCurrentPosition;
          Renderer::Monitor::RefreshRate = Renderer::Monitor::GetCurrentMonitorRefreshRate();
          Editor::UI::PerformanceOverlay::fpsGraph.max = Renderer::Monitor::RefreshRate;
        }
      } //Check if window moved

      if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) { //Camera Control
        static constexpr float slowModeSpeed = 8;
        static constexpr float fastModeSpeed = 15;

        glm::vec3 moveDir = glm::vec3(0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) moveDir -= glm::vec3(0,0,1);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) moveDir -= glm::vec3(1,0,0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) moveDir += glm::vec3(0,0,1);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) moveDir += glm::vec3(1,0,0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) moveDir -= glm::vec3(0,1,0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) moveDir += glm::vec3(0,1,0);
        if(glm::length(moveDir) != 0) {
          const float speed = System::deltaTime * (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)? fastModeSpeed : slowModeSpeed);
          camera->transform.position += (camera->transform.rotation * glm::normalize(moveDir)) * speed;
        }

        if(glm::length(glm::vec2(User::Cursor::delta)) != 0) {
          const glm::vec2 lookDelta = -glm::vec2(User::Cursor::delta) * System::deltaTime;

          const glm::quat pitch = glm::angleAxis(lookDelta.y, glm::vec3(1,0,0));
          const glm::quat yaw = glm::angleAxis(lookDelta.x, glm::vec3(0,1,0));
          camera->transform.rotation = glm::normalize(yaw * camera->transform.rotation * pitch);
        }
      } //Camera Control

      Editor::UI::PerformanceOverlay::update(System::deltaTime);

      User::update();
    } //Update

    { //Render
      Renderer::Core::clear();

      { //Scene
        { //GL Init
          glEnable(GL_DEPTH_TEST);
          glDepthMask(GL_TRUE);
          glDepthFunc(GL_LESS);
          glEnable(GL_CULL_FACE);
          glCullFace(GL_BACK);
          glFrontFace(GL_CW);
          glDisable(GL_BLEND);

          glPolygonMode(GL_FRONT_AND_BACK, wireframeMode? GL_LINE : GL_FILL);
        } //GL Init


        { //Scene
          static Renderer::Mesh mesh;
          static std::vector<Renderer::Transform> meshInstances;
          static bool sceneGenerated = false;

          if(sceneGenerated == false) {
            sceneGenerated = true;

            { //Cube
              mesh();
              
              std::vector<Renderer::Mesh::Vertex> vertices;
              vertices = {
                Renderer::Mesh::Vertex{{-1,-1, 1}, {1,0,0,1}},
                Renderer::Mesh::Vertex{{-1, 1, 1}, {0,1,0,1}},
                Renderer::Mesh::Vertex{{ 1, 1, 1}, {0,0,1,1}},
                Renderer::Mesh::Vertex{{ 1,-1, 1}, {1,1,1,1}},
                
                Renderer::Mesh::Vertex{{-1,-1,-1}, {1,0,0,1}},
                Renderer::Mesh::Vertex{{-1, 1,-1}, {0,1,0,1}},
                Renderer::Mesh::Vertex{{ 1, 1,-1}, {0,0,1,1}},
                Renderer::Mesh::Vertex{{ 1,-1,-1}, {1,1,1,1}},
              };

              std::vector<Renderer::Mesh::Index> indices;
              indices = {
                0,4,5, 0,5,1, //-x
                6,7,3, 6,3,2, //+x

                0,3,7, 0,7,4, //-y
                1,5,6, 1,6,2, //+y

                7,6,5, 7,5,4, //-z
                0,1,2, 0,2,3, //+z
              };

              mesh.setVertices(vertices);
              mesh.setIndices(indices);
            } //Cube

            std::mt19937 gen(std::chrono::high_resolution_clock().now().time_since_epoch().count());
            std::uniform_real_distribution<float> rand(0,1);

            auto randPos = [&]() {
              return glm::vec3{
                ((rand(gen) * 2) - 1) * 50.0f,
                ((rand(gen) * 2) - 1) * 50.0f,
                ((rand(gen) * 2) - 1) * 50.0f
              };
            };
            auto randRot = [&]() {
              return
                glm::angleAxis(rand(gen) * glm::two_pi<float>(), glm::vec3{1,0,0}) *
                glm::angleAxis(rand(gen) * glm::two_pi<float>(), glm::vec3{0,1,0});
            };
            auto randSiz = [&]() {
              return glm::vec3{
                .1f + rand(gen) * 2.9f,
                .1f + rand(gen) * 2.9f,
                .1f + rand(gen) * 2.9f
              };
            };

            for(int i = 0; i < 100; i++) {
              meshInstances.push_back(Renderer::Transform{randPos(), randRot(), randSiz()});
            }
          }
          
          unlitShader.use();
          mesh.use();
          for(const Renderer::Transform &transform : meshInstances) {
            unlitShader.setUniform("uModel", transform.getModelMatrix());
            mesh.draw();
          }
        } //Scene
        { //Water
          waterShader.use();
          waterShader.setUniform("uProj", camera->getProjectionMatrix(Renderer::Core::windowSize));
          waterShader.setUniform("uView", camera->getViewMatrix());
          waterShader.setUniform("uModel", Renderer::Transform({0,-3,0}).getModelMatrix());

          waterShader.setUniform("uCamPos", camera->transform.position);
          waterShader.setUniform("uSunDir", glm::normalize(glm::vec3{1,1,-1}));
          waterShader.setUniform("uSunColor", glm::vec3{1.0f, 0.98f, 0.93f});

          waterShader.setUniform("uChoppiness", 1.5f);
          waterShader.setUniform("uAmplitudeScale", 1.0f);

          waterShader.setUniform("uFoamThreshold", 1.0f);
          waterShader.setUniform("uFoamIntensity", 1.0f);

          waterShader.setUniform("uFogColor", glm::vec4{1,.7,.5,1});
          waterShader.setUniform("uFogDensity", .7f);

          waterShader.setUniform("uWindDir", glm::vec2{1,1});
          waterShader.setUniform("uWindSpeed", 35.0f);

          waterShader.setUniform("uMeshSpacing", .5f);

          waterShader.setUniform("uSceneColor", 1);
          waterShader.setUniform("uSceneDepth", 2);
          waterShader.setUniform("uHasSceneTex", false);

          static float t = 0;
          if(paused == false) t += System::deltaTime;
          waterShader.setUniform("t", t);


          water.use();
          water.draw();
        } //Water
      } //Scene
      { //UI
        { //GL Init
          glBindVertexArray(0);
          glBindBuffer(GL_ARRAY_BUFFER, 0);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
          glUseProgram(0);

          glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

          Renderer::Core::window->pushGLStates();
        } //GL Init


        Editor::UI::PerformanceOverlay::draw(font);

        Renderer::Core::window->popGLStates();
      } //UI



      Renderer::Core::display();
    } //Render
  }

  Renderer::Core::end();

  return 0;
}