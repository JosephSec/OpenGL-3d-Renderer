#include <System.hpp>
#include <User.hpp>

#include <Renderer/Core.hpp>
#include <Renderer/Core/Shader.hpp>
#include <Renderer/Core/Mesh.hpp>
#include <Renderer/Component/Camera.hpp>

#include <Renderer/Debug/Graph.hpp>
#include <Renderer/Debug/GPU.hpp>

#include <iostream>


static Renderer::LineGraph fpsGraph(60, 0, 240.0f);
static float plotTime = 1.0f / 30.0f;
static float timeTillPlot = plotTime;

static Renderer::LineGraph vramGraph(60, 0, 1);


int main(int argc, char *argv[]) {
  System::init();
  Renderer::Core::init(true);
  Renderer::Core::window->setActive(true);

  Renderer::Mesh mesh; {
    mesh();

    constexpr uint32_t triCount = 1'000'000;
    constexpr float depth = 50.0f;                    // distance in front of the camera
    constexpr float fovX  = glm::radians(90.0f);      // horizontal FOV
    const float aspect = float(Renderer::Core::windowSize.x) / Renderer::Core::windowSize.y;

    // View rectangle at that depth
    const float halfW = depth * std::tan(fovX * 0.5f);   // tan(45°) = 1, so halfW == depth at 90°
    const float halfH = halfW / aspect;

    // Grid with the same aspect ratio as the window, so cells are roughly square
    const uint32_t cols = static_cast<uint32_t>(std::ceil(std::sqrt(triCount * aspect)));
    const uint32_t rows = (triCount + cols - 1) / cols;

    const float cellW = (2.0f * halfW) / cols;
    const float cellH = (2.0f * halfH) / rows;
    const float triW  = cellW * 0.9f;
    const float triH  = cellH * 0.9f;

    std::vector<Renderer::Mesh::Vertex> vertices;
    std::vector<Renderer::Mesh::Index> indices;      // must be uint32_t
    vertices.reserve(size_t(triCount) * 3);
    indices.reserve(size_t(triCount) * 3);

    for(uint32_t i = 0; i < triCount; i++) {
      const float cx = -halfW + (i % cols + 0.5f) * cellW;
      const float cy = -halfH + (i / cols + 0.5f) * cellH;
      const float z  = -depth;
      const uint32_t base = i * 3;

      vertices.push_back({{cx - triW * 0.5f, cy - triH * 0.5f, z}, {1,0,0,1}});
      vertices.push_back({{cx,               cy + triH * 0.5f, z}, {0,1,0,1}});
      vertices.push_back({{cx + triW * 0.5f, cy - triH * 0.5f, z}, {0,0,1,1}});

      indices.push_back(base + 0);
      indices.push_back(base + 1);
      indices.push_back(base + 2);
    }

    mesh.setVertices(vertices);
    mesh.setIndices(indices);
  }

  Renderer::Shader shader("Unlit");

  
  std::cout << "GPU: " << Renderer::Core::GetGPUName() << " | Available VRAM: " << Renderer::GPU::VRAMCapacity / (1024 * 1024) << "MB\n";
  std::cout << "Used VRAM: " << Renderer::GPU::VRAMInUse / 1000 << "KB | " << Renderer::GPU::GetUsedVRAM() << "%\n";


  fpsGraph.setTimerLength(1 / 30.0f);

  vramGraph.max = Renderer::GPU::VRAMCapacity;
  vramGraph.setTimerLength(1 / 30.0f);


  Renderer::Camera camera;


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
          Renderer::Core::window->setFramerateLimit(cap? 60 : 0);
        }
        else if(keyPressed->code == sf::Keyboard::Key::X && keyPressed->alt) fpsGraph.log();
      }
      else if(const auto *mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) User::HandleMouseButtonPressed(mouseButtonPressed);
      else if(const auto *mouseButtonReleased = event.getIf<sf::Event::MouseButtonReleased>()) User::HandleMouseButtonReleased(mouseButtonReleased);
    }

    { //Update
      System::update();
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
        shader.setUniform("uProj", camera.getProjectionMatrix(Renderer::Core::windowSize));
        shader.setUniform("uView", camera.getViewMatrix());
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

        
        sf::FloatRect area = {{15.0f, 15.0f}, {350.0f, 150.0f}};

        sf::RectangleShape shape(area.size);
        shape.setPosition(area.position);
        shape.setFillColor(sf::Color(25,25,25,200));
        Renderer::Core::window->draw(shape);

        fpsGraph.draw(area, sf::Color::Blue, 240.0f);
        vramGraph.draw(area, sf::Color::Red, Renderer::GPU::VRAMCapacity);

        Renderer::Core::window->popGLStates();
      } //UI



      Renderer::Core::display();
    } //Render
  }

  Renderer::Core::end();

  return 0;
}