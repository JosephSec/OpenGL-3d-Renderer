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


namespace Renderer {
  struct MeshRenderer {
  public:
    MeshRenderer(Mesh *_mesh, Shader *_shader) : m_mesh(_mesh), m_shader(_shader) {}

    void draw(const glm::mat4x4 &_model) const {
      m_shader->use();
      m_shader->setUniform("uModel", _model);

      m_mesh->use();
      m_mesh->draw();
    }
    inline void draw(const Transform &_transform) const {
      draw(_transform.getModelMatrix());
    }


    Mesh *m_mesh;
    Shader *m_shader;
  };
}

static GLuint fbo, vao, colorTex, depthTex;
static glm::ivec2 postSize;
static void resizePost(const glm::ivec2 _size) {
  postSize = _size;

  { //Color
    glCreateTextures(GL_TEXTURE_2D, 1, &colorTex);
    glTextureStorage2D(colorTex, 1, GL_RGBA8, postSize.x,postSize.y);

    glTextureParameteri(colorTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(colorTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureParameteri(colorTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(colorTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  } //Color
  { //Depth
    glCreateTextures(GL_TEXTURE_2D, 1, &depthTex);
    glTextureStorage2D(depthTex, 1, GL_DEPTH_COMPONENT24, postSize.x,postSize.y);

    glTextureParameteri(depthTex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(depthTex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureParameteri(depthTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(depthTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  } //Depth

  glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, colorTex, 0);
  glNamedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT,  depthTex, 0);
  if(glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "[POST PROCESS ERROR]: Post process framebuffer is incomplete\n";
  }
}
static void initPost(const glm::ivec2 _size) {
  glGenFramebuffers(1, &fbo);
  glGenVertexArrays(1, &vao);
  
  resizePost(_size);
}
static void beginPost() {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glViewport(0,0, postSize.x,postSize.y);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
static void endPost(Renderer::Shader *_shader, Renderer::Camera *_camera, float _t) {
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0,0, Renderer::Core::windowSize.x, Renderer::Core::windowSize.y);
  glDisable(GL_DEPTH_TEST);

  glBindTextureUnit(1, colorTex);
  glBindTextureUnit(2, depthTex);

  _shader->use();
  _shader->setUniform("uColor", 1);
  _shader->setUniform("uDepth", 2);
  _shader->setUniform("uViewSize", glm::vec2{Renderer::Core::windowSize});
  _shader->setUniform("uCameraWorldPos", _camera->transform.position);
  _shader->setUniform("uTime", _t);

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 3);

  glEnable(GL_DEPTH_TEST);
}


static bool wireframeMode = false;
static bool paused = false;
static bool doPost = false;
int main(int argc, char *argv[]) {
  System::init();
  Renderer::Core::init(true);
  Renderer::Core::window->setActive(true);
  initPost(Renderer::Core::windowSize);
  Editor::UI::PerformanceOverlay::init();

  sf::Font font(System::PATH+"/assets/Roboto.ttf");

  Renderer::Shader unlitShader("Unlit");
  Renderer::Shader postShader("PostProcess");
  Renderer::Shader waterShader("Water");

  Renderer::Camera *camera = new Renderer::Camera();
  camera->farPlane = 10000;


  Renderer::Mesh water; {
    water();

    const glm::ivec2 grid = glm::ivec2{300,300};
    const float scale = .5f;

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
  Renderer::Mesh cube; {
    cube();
    
    std::vector<Renderer::Mesh::Vertex> vertices = {
      Renderer::Mesh::Vertex{{-1,-1, 1}, {1,0,0,1}},
      Renderer::Mesh::Vertex{{-1, 1, 1}, {0,1,0,1}},
      Renderer::Mesh::Vertex{{ 1, 1, 1}, {0,0,1,1}},
      Renderer::Mesh::Vertex{{ 1,-1, 1}, {1,1,1,1}},
      
      Renderer::Mesh::Vertex{{-1,-1,-1}, {1,0,0,1}},
      Renderer::Mesh::Vertex{{-1, 1,-1}, {0,1,0,1}},
      Renderer::Mesh::Vertex{{ 1, 1,-1}, {0,0,1,1}},
      Renderer::Mesh::Vertex{{ 1,-1,-1}, {1,1,1,1}},
    };
    std::vector<Renderer::Mesh::Index> indices = {
      0,4,5, 0,5,1, //-x
      6,7,3, 6,3,2, //+x

      0,3,7, 0,7,4, //-y
      1,5,6, 1,6,2, //+y

      7,6,5, 7,5,4, //-z
      0,1,2, 0,2,3, //+z
    };

    cube.setVertices(vertices);
    cube.setIndices(indices);
  }


  std::cout << "Active Monitor Refresh Rate: " << Renderer::Monitor::GetCurrentMonitorRefreshRate() << '\n';
  std::cout << "GPU: " << Renderer::GPU::GetGPUName() << " | Available VRAM: " << Renderer::GPU::VRAMCapacity / (1024 * 1024) << "MB\n";
  std::cout << "Used VRAM: " << Renderer::GPU::VRAMInUse / 1000 << "KB | " << Renderer::GPU::GetUsedVRAM() << "%\n";
  while(Renderer::Core::window->isOpen()) {
    while(const auto &eventOpt = Renderer::Core::window->pollEvent()) {
      const auto &event = *eventOpt;

      if(event.is<sf::Event::Closed>()) Renderer::Core::window->close();
      else if(const auto *resized = event.getIf<sf::Event::Resized>()) {
        Renderer::Core::UpdateWindowSize();
        resizePost(Renderer::Core::windowSize);
      }
      else if(event.is<sf::Event::FocusLost>()) User::HandleFocusLost();

      else if(const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        User::HandleKeyPressed(keyPressed);
        if(keyPressed->code == sf::Keyboard::Key::Space) paused = !paused;
        else if(keyPressed->code == sf::Keyboard::Key::F1) Editor::UI::PerformanceOverlay::ToggleEnabled();
        else if(keyPressed->code == sf::Keyboard::Key::F2) {
          wireframeMode = !wireframeMode;
          glPolygonMode(GL_FRONT_AND_BACK, wireframeMode? GL_LINE : GL_FILL);
        }
        else if(keyPressed->code == sf::Keyboard::Key::F3) doPost = !doPost;
      }
      else if(const auto *mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) User::HandleMouseButtonPressed(mouseButtonPressed);
      else if(const auto *mouseButtonReleased = event.getIf<sf::Event::MouseButtonReleased>()) User::HandleMouseButtonReleased(mouseButtonReleased);
    }

    static float t = 0;
    if(paused == false) t += System::deltaTime;

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
        static constexpr float slowModeSpeed = 5;
        static constexpr float defaultModeSpeed = 25;
        static constexpr float fastModeSpeed = 50;

        glm::vec3 moveDir = glm::vec3(0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) moveDir -= glm::vec3(0,0,1);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) moveDir -= glm::vec3(1,0,0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) moveDir += glm::vec3(0,0,1);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) moveDir += glm::vec3(1,0,0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) moveDir -= glm::vec3(0,1,0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) moveDir += glm::vec3(0,1,0);
        if(glm::length(moveDir) != 0) {
          float speed = System::deltaTime;
          if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) speed *= fastModeSpeed;
          else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) speed *= slowModeSpeed;
          else speed *= defaultModeSpeed;

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

      const glm::mat4x4 projMat = camera->getProjectionMatrix(Renderer::Core::windowSize);
      const glm::mat4x4 viewMat = camera->getViewMatrix();

      unlitShader.use();
      unlitShader.setUniform("uProj", projMat);
      unlitShader.setUniform("uView", viewMat);
      waterShader.use();
      waterShader.setUniform("uProj", projMat);
      waterShader.setUniform("uView", viewMat);
      postShader.use();
      postShader.setUniform("uProj", projMat);
      postShader.setUniform("uView", viewMat);

      User::update();
    } //Update

    { //Render
      Renderer::Core::clear();

      { //Scene
        { //GL Init
          glEnable(GL_DEPTH_TEST);
          glDepthMask(GL_TRUE);
          glDepthFunc(GL_LESS);
          // glEnable(GL_CULL_FACE);
          glDisable(GL_CULL_FACE);
          glCullFace(GL_BACK);
          glFrontFace(GL_CW);
          glDisable(GL_BLEND);

          glPolygonMode(GL_FRONT_AND_BACK, wireframeMode? GL_LINE : GL_FILL);
        } //GL Init

        if(doPost) beginPost();
        { //Water
          waterShader.use();
          waterShader.setUniform("uModel", Renderer::Transform({0,-3,0}).getModelMatrix());

          waterShader.setUniform("uCamPos", camera->transform.position);
          waterShader.setUniform("uSunDir", glm::normalize(glm::vec3{1,1,-1}));
          waterShader.setUniform("uSunColor", glm::vec3{1.0f, 0.98f, 0.93f});

          waterShader.setUniform("uChoppiness", 3.5f);
          waterShader.setUniform("uAmplitudeScale", 4.0f);

          waterShader.setUniform("uFoamThreshold", 1.0f);
          waterShader.setUniform("uFoamIntensity", 1.0f);

          waterShader.setUniform("uFogColor", glm::vec4{.68f,.74f,.78f,1});
          waterShader.setUniform("uFogDensity", .3f);

          waterShader.setUniform("uWindDir", glm::vec2{1,0});
          waterShader.setUniform("uWindSpeed", 55.0f);

          waterShader.setUniform("uMeshSpacing", 0.5f);

          waterShader.setUniform("uSceneColor", 1);
          waterShader.setUniform("uSceneDepth", 2);
          waterShader.setUniform("uHasSceneTex", false);

          waterShader.setUniform("uTime", t);


          water.use();
          water.draw();
        } //Water
        if(doPost) endPost(&postShader, camera, t);
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