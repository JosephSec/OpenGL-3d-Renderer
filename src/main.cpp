#include <System.hpp>
#include <User.hpp>

#include <Renderer/Core.hpp>
#include <Renderer/Core/Shader.hpp>
#include <Renderer/Core/Mesh.hpp>
#include <Renderer/Component/Camera.hpp>
#include <Renderer/Debug/Graph.hpp>
#include <Renderer/Hardware/GPU.hpp>
#include <Renderer/Hardware/Monitor.hpp>

#include <Editor/UIButton.hpp>

#include <iostream>


static Renderer::LineGraph fpsGraph(60, 0, 240.0f);
static Renderer::LineGraph vramGraph(60, 0, 1);
static bool metricsEnabled = false;

static Editor::UIButton metricsButton(glm::vec4{15,15, 25,25}, glm::vec4(1));
static sf::Texture metricsTexture;


int main(int argc, char *argv[]) {
  System::init();
  Renderer::Core::init(true);
  Renderer::Core::window->setActive(true);

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


  fpsGraph.max = Renderer::Monitor::RefreshRate;
  fpsGraph.setTimerLength(1 / 30.0f);

  vramGraph.max = Renderer::GPU::VRAMCapacity;
  vramGraph.setTimerLength(1 / 30.0f);


  Renderer::Camera *camera = new Renderer::Camera();


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
        else if(keyPressed->code == sf::Keyboard::Key::X && keyPressed->alt) fpsGraph.log();
        else if(keyPressed->code == sf::Keyboard::Key::F1) metricsEnabled = !metricsEnabled;
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
          fpsGraph.max = Renderer::Monitor::RefreshRate;
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


        fpsGraph.update(1.0f / System::deltaTime, System::deltaTime);
        vramGraph.update(Renderer::GPU::VRAMInUse, System::deltaTime);

        metricsButton.setColor(glm::vec4{1,1,1, metricsEnabled? 1 : .2f});
        metricsButton.draw();

        if(metricsEnabled == true) {
          sf::FloatRect area = {{15.0f, 15.0f}, {350.0f, 150.0f}};
          area.position = sf::Vector2f{static_cast<float>(Renderer::Core::windowSize.x),static_cast<float>(Renderer::Core::windowSize.y)} - area.position - area.size;

          sf::RectangleShape shape(area.size);
          shape.setPosition(area.position);
          shape.setFillColor(sf::Color(25,25,25,200));
          Renderer::Core::window->draw(shape);

          fpsGraph.draw(area, sf::Color::Blue);
          vramGraph.draw(area, sf::Color::Red);

          sf::Text text(font);
          text.setString(std::to_string(static_cast<int>(1.0f / System::deltaTime)));
          text.setCharacterSize(15);
          text.setPosition(fpsGraph.getPlotPosition(area, fpsGraph.getSize() - 1) - text.getLocalBounds().size);
          Renderer::Core::window->draw(text);
        }

        Renderer::Core::window->popGLStates();
      } //UI



      Renderer::Core::display();
    } //Render
  }

  Renderer::Core::end();

  return 0;
}