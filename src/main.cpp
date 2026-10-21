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
#include <Editor/UIButton.hpp>

#include <random>
#include <chrono>

#include <iostream>


static Editor::UIButton metricsButton(glm::vec4{15,15, 25,25}, glm::vec4(1));
static sf::Texture metricsTexture;


struct Body {
  glm::vec3 transform; //xy - position | z - radius
  glm::vec3 color;
};

int main(int argc, char *argv[]) {
  System::init();
  Renderer::Core::init(true);
  Renderer::Core::window->setActive(true);

  Editor::PerformanceOverlay::init();

  Renderer::Mesh mesh; {
    mesh();

    mesh.setVertices({
      Renderer::Mesh::Vertex{{-1,-1, 0}, {1,0,0,1}},
      Renderer::Mesh::Vertex{{ 0, 1, 0}, {0,1,0,1}},
      Renderer::Mesh::Vertex{{ 1,-1, 0}, {0,0,1,1}},
    });
    mesh.setIndices({0,1,2});
  }

  Renderer::Shader shader("Unlit");

  bool x = metricsTexture.loadFromFile(System::PATH+"/assets/editor/textures/metric.png");
  metricsButton.setTexture(&metricsTexture);

  sf::Font font(System::PATH+"/assets/Roboto.ttf");


  std::cout << "Active Monitor Refresh Rate: " << Renderer::Monitor::GetCurrentMonitorRefreshRate() << '\n';
  std::cout << "GPU: " << Renderer::GPU::GetGPUName() << " | Available VRAM: " << Renderer::GPU::VRAMCapacity / (1024 * 1024) << "MB\n";
  std::cout << "Used VRAM: " << Renderer::GPU::VRAMInUse / 1000 << "KB | " << Renderer::GPU::GetUsedVRAM() << "%\n";


  Renderer::Camera *camera = new Renderer::Camera();


  std::vector<Body> bodies(1000); {
    std::mt19937 gen(std::chrono::high_resolution_clock().now().time_since_epoch().count());
    std::uniform_real_distribution<float> rand(0,1);

    for(Body &body : bodies) {
      body.transform = {
        rand(gen) * Renderer::Core::windowSize.x,
        rand(gen) * Renderer::Core::windowSize.y,
        10 + rand(gen) * 40.0f
      };
      body.color = {
        .1f + rand(gen) * .9f,
        .1f + rand(gen) * .9f,
        .1f + rand(gen) * .9f
      };
    }
  }
  Renderer::GPUBuffer bodiesBuffer;
  bodiesBuffer();
  bodiesBuffer.upload(bodies.size() * sizeof(Body), bodies.data(), GL_DYNAMIC_DRAW);

  Renderer::Shader spaceShader("Space");
  GLuint spaceVAO = 0; {
    glCreateVertexArrays(1, &spaceVAO);

    glVertexArrayVertexBuffer(spaceVAO, 0, bodiesBuffer.id(), 0, sizeof(Body));


    glVertexArrayAttribFormat(spaceVAO, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Body, transform));
    glVertexArrayAttribBinding(spaceVAO, 0, 0);
    glEnableVertexArrayAttrib(spaceVAO, 0);

    glVertexArrayAttribFormat(spaceVAO, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Body, color));
    glVertexArrayAttribBinding(spaceVAO, 1, 0);
    glEnableVertexArrayAttrib(spaceVAO, 1);
  }


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
        } //GL Init


        shader.use();
        shader.setUniform("uProj", camera->getProjectionMatrix(Renderer::Core::windowSize));
        shader.setUniform("uView", camera->getViewMatrix());
        shader.setUniform("uModel", Renderer::Transform({0,0,-3}).getModelMatrix());

        mesh.use();
        mesh.draw();
      } //Scene
      { //UI
        { //GL Init
          glBindVertexArray(0);
          glBindBuffer(GL_ARRAY_BUFFER, 0);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
          glUseProgram(0);

          Renderer::Core::window->pushGLStates();
        } //GL Init


        metricsButton.setColor(glm::vec4{1,1,1, Editor::PerformanceOverlay::IsEnabled? 1 : .2f});
        metricsButton.draw();

        Editor::PerformanceOverlay::draw(font);

        Renderer::Core::window->popGLStates();
      } //UI



      Renderer::Core::display();
    } //Render
  }

  Renderer::Core::end();

  return 0;
}