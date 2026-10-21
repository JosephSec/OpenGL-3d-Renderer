#include <System.hpp>
#include <User.hpp>

#include <Renderer/Core.hpp>
#include <Renderer/Core/Shader.hpp>
#include <Renderer/Core/Mesh.hpp>
#include <Renderer/Component/Camera.hpp>
#include <Renderer/Debug/Graph.hpp>
#include <Renderer/Hardware/GPU.hpp>
#include <Renderer/Hardware/Monitor.hpp>
#include <Renderer/Pipeline/GPUMemoryManager.hpp>

#include <Editor/UI/PerformanceOverlay.hpp>
#include <Editor/UIButton.hpp>

#include <random>
#include <chrono>

#include <iostream>


static bool wireframeMode = false;
int main(int argc, char *argv[]) {
  System::init();
  Renderer::Core::init(true);
  Renderer::Core::window->setActive(true);

  Editor::PerformanceOverlay::init();

  Renderer::Mesh *mesh = Renderer::GPUMemoryManager::CreateMesh(); {
    mesh->setVertices({
      Renderer::Mesh::Vertex{{-1,-1, 0}, {1,0,0,1}},
      Renderer::Mesh::Vertex{{ 0, 1, 0}, {0,1,0,1}},
      Renderer::Mesh::Vertex{{ 1,-1, 0}, {0,0,1,1}},
    });
    mesh->setIndices({0,1,2});
  }

  Renderer::Shader unlitShader("Unlit");

  sf::Font font(System::PATH+"/assets/Roboto.ttf");

  Renderer::Camera *camera = new Renderer::Camera();


  Renderer::Shader *waterShader = Renderer::GPUMemoryManager::CreateShader("Water");
  Renderer::Mesh *water = Renderer::GPUMemoryManager::CreateMesh(); {
    const glm::ivec2 grid = glm::ivec2{1419,1419};
    const float scale = .1f;

    std::vector<Renderer::Mesh::Vertex> vertices;
    for(int y = 0; y <= grid.y; y++) {
      for(int x = 0; x <= grid.x; x++) {
        vertices.push_back(Renderer::Mesh::Vertex{
          glm::vec3{x - static_cast<float>(grid.x + 1) / 2.0f, 0, -(y - static_cast<float>(grid.y + 1) / 2.0f)} * scale,
          {0,0,1,1},
        });
      }
    }
    water->setVertices(vertices);

    std::vector<Renderer::Mesh::Index> indices;
    for(int y = 0; y < grid.y; y++) {
      for(int x = 0; x < grid.x; x++) {
        const Renderer::Mesh::Index a = x + y * (grid.x + 1);
        const Renderer::Mesh::Index b = a + grid.x + 1;
        const Renderer::Mesh::Index c = b + 1;
        const Renderer::Mesh::Index d = a + 1;
        
        indices.push_back(a);
        indices.push_back(b);
        indices.push_back(c);

        indices.push_back(a);
        indices.push_back(c);
        indices.push_back(d);
      }
    }
    water->setIndices(indices);
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
        if(keyPressed->code == sf::Keyboard::Key::Space) {
          static bool cap = false;
          cap = !cap;
          Renderer::Core::window->setFramerateLimit(cap? Renderer::Monitor::RefreshRate / 2 : 0);
        }
        else if(keyPressed->code == sf::Keyboard::Key::F1) Editor::PerformanceOverlay::ToggleEnabled();
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
          Editor::PerformanceOverlay::fpsGraph.max = Renderer::Monitor::RefreshRate;
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

      Editor::PerformanceOverlay::update(System::deltaTime);

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


        { //Triangle
          unlitShader.use();
          unlitShader.setUniform("uProj", camera->getProjectionMatrix(Renderer::Core::windowSize));
          unlitShader.setUniform("uView", camera->getViewMatrix());
          unlitShader.setUniform("uModel", Renderer::Transform({0,0,-3}).getModelMatrix());

          mesh->use();
          mesh->draw();
        } //Triangle
        { //Water
          waterShader->use();
          waterShader->setUniform("uProj", camera->getProjectionMatrix(Renderer::Core::windowSize));
          waterShader->setUniform("uView", camera->getViewMatrix());
          waterShader->setUniform("uModel", Renderer::Transform({0,-3,0}).getModelMatrix());

          static float t = 0;
          t += System::deltaTime;
          waterShader->setUniform("t", t);


          water->use();
          water->draw();
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


        Editor::PerformanceOverlay::draw(font);

        Renderer::Core::window->popGLStates();
      } //UI



      Renderer::Core::display();
    } //Render
  }

  Renderer::Core::end();

  return 0;
}