#include <System.hpp>

#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <Renderer/Shader.hpp>
#include <Mesh.hpp>

#include <iostream>


static Shader shader;
static sf::RenderWindow window;
static sf::Clock timeClock;

static glm::mat4x4 viewMatrix;


struct {
  glm::ivec2 position;
  glm::ivec2 delta;

  void update() {
    const sf::Vector2i curMousePos = sf::Mouse::getPosition(window);
    delta = glm::ivec2(curMousePos.x, curMousePos.y) - position;
    position += delta;
  }
} Mouse;
struct {
  glm::mat4x4 matrix;
  float nearPlane = .1;
  float farPlane = 50;
  float fov = 60;
} Projection;
struct {
  glm::vec3 position;
  glm::quat rotation;
  float sensitivity = 10;
} Camera;


static void UpdateProjectionMatrix() {
  const float aspectRatio = static_cast<float>(window.getSize().x) / static_cast<float>(window.getSize().y);
  Projection.matrix = glm::perspective(glm::radians(Projection.fov), aspectRatio, Projection.nearPlane, Projection.farPlane);
}
static void UpdateViewMatrix() {
  const glm::mat4x4 rotation = glm::mat4_cast(glm::conjugate(Camera.rotation));
  const glm::mat4x4 translation = glm::translate(glm::mat4x4(1), glm::vec3(-Camera.position.x,-Camera.position.y,Camera.position.z));
  viewMatrix = rotation * translation;
}
static void HandleResize() {
  const sf::Vector2u windowSize = window.getSize();

  window.setView(sf::View(sf::FloatRect{{0,0}, sf::Vector2f{window.getSize()}}));
  glViewport(0,0, windowSize.x, windowSize.y);
  UpdateProjectionMatrix();
}

static void initGL() {
  GLenum err = glewInit();
  if(err != GLEW_OK) std::cout << "GLEW init error: " << glewGetErrorString(err) << '\n';

  glEnable(GL_DEPTH_TEST);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CW);

  glClearColor(.1,.1,.15, 1);
}
static void init() {
  System::init();

  sf::ContextSettings settings;
  settings.depthBits = 24;
  settings.stencilBits = 8;
  settings.antiAliasingLevel = 4;
  settings.majorVersion = 3;
  settings.minorVersion = 3;
  settings.attributeFlags = sf::ContextSettings::Default;

  window = sf::RenderWindow(sf::VideoMode{{800,600}}, "Window", sf::State::Windowed, settings);
  // window.setFramerateLimit(120);

  initGL();

  shader = Shader("UnlitTriangle");

  const sf::Vector2i curMousePos = sf::Mouse::getPosition(window);
  Mouse.position = glm::ivec2(curMousePos.x, curMousePos.y);
  Mouse.delta = glm::ivec2(0,0);

  UpdateProjectionMatrix();
  UpdateViewMatrix();
}


int main(int argc, char *argv[]) {
  init();

  Mesh mesh;
  mesh.vertices = {
    Mesh::Vertex{{-.5,-.5,0}, {1,0,0, 1}},
    Mesh::Vertex{{ .0, .5,0}, {0,1,0, 1}},
    Mesh::Vertex{{ .5,-.5,0}, {0,0,1, 1}},
  };
  mesh.indeces = {0,1,2};


  while(window.isOpen()) {
    while(const auto &eventOpt = window.pollEvent()) {
      const auto &event = *eventOpt;

      if(event.is<sf::Event::Closed>()) window.close();
      else if(const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if(keyPressed->code == sf::Keyboard::Key::Escape) window.close();
      }
      else if(const auto *resized = event.getIf<sf::Event::Resized>()) {
        HandleResize();
      }
    }


    { //Update
      Mouse.update();

      const float deltaTime = timeClock.restart().asSeconds();

      if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
        glm::vec3 moveDir = glm::vec3(0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) moveDir += glm::vec3(0,0,1);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) moveDir -= glm::vec3(1,0,0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) moveDir -= glm::vec3(0,0,1);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) moveDir += glm::vec3(1,0,0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) moveDir -= glm::vec3(0,1,0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) moveDir += glm::vec3(0,1,0);
        if(glm::length(moveDir) != 0) Camera.position += (glm::conjugate(Camera.rotation) * moveDir) * deltaTime;

        if(glm::length(glm::vec2(Mouse.delta)) != 0) {
          const glm::vec2 lookDelta = -glm::vec2(Mouse.delta) * Camera.sensitivity * deltaTime;

          Camera.rotation = Camera.rotation * glm::angleAxis(lookDelta.y, glm::vec3(1,0,0));
          Camera.rotation = glm::angleAxis(lookDelta.x, glm::vec3(0,1,0)) * Camera.rotation;
          Camera.rotation = glm::normalize(Camera.rotation);
        }

        UpdateViewMatrix();
      }

      static bool prev_x_state = false;
      const bool cur_x_state = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X);
      if((prev_x_state != cur_x_state) && cur_x_state == true) {
        std::stringstream ss;
        ss << "Camera Position: (" << Camera.position.x << ", " << Camera.position.y << ", " << Camera.position.z << ")\n";
        ss << "Camera Rotation:\n" << PrintMatrix(glm::mat4_cast(Camera.rotation)) << '\n';
        std::cout << ss.str();
      }
      prev_x_state = cur_x_state;
    }

    { //Render
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
      glUseProgram(shader);
      shader.SetUniform("projection", Projection.matrix);
      shader.SetUniform("view", viewMatrix);
      shader.SetUniform("model", glm::mat4x4(1));
      mesh.draw();
  
      window.display();
    }
  }

  glDeleteProgram(shader);

  return 0;
}