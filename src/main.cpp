#include <System.hpp>
#include <User.hpp>
#include <Renderer.hpp>
using namespace Renderer;
#include <UI.hpp>

#include <SFML/Graphics.hpp>

#include <fstream>
#include <iostream>
#include <random>
#include <chrono>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>


static std::vector<std::string> split_string(const std::string &_str, char _ch) {
  std::vector<std::string> tokens;

  std::string buffer;
  for(const char &ch : _str) {
    if(ch == _ch && buffer.empty() == false) {
      tokens.push_back(buffer);
      buffer.clear();
      continue;
    }

    buffer.push_back(ch);
  }

  if(buffer.empty() == false) tokens.push_back(buffer);

  return tokens;
}
static std::string BoolString(const std::string &_str, bool _val) {
  return _str + ": " + std::string(_val? "True" : "False");
}
static std::string FloatString(float _val, int _precision = 2) {
  const float scalar = std::pow(10, _precision);

  std::stringstream ss;
  ss << static_cast<int>(_val * scalar) / scalar;
  return ss.str();
}
static std::string Vec3String(const glm::vec3 _val, int _precision = 2) {
  const float scalar = std::pow(10, _precision);

  std::stringstream ss;
  ss << "(" <<
    (static_cast<int>(_val.x * scalar) / scalar) << ", " <<
    (static_cast<int>(_val.y * scalar) / scalar) << ", " <<
    (static_cast<int>(_val.z * scalar) / scalar) << ")";
  return ss.str();
}


static void init() {
  System::init();
  Core::init({800 + 225 * 2, 600}, "3D Rendering Library");
  User::init();
  UI::Core::init(Core::window, "Roboto.ttf");

  Core::window->setVerticalSyncEnabled(true);
}
int main(int argc, char *argv[]) {
  init();


  Camera *camera = new Camera();
  camera->fov = 90;
  camera->transform.position = glm::vec3(2.5,2,-4.5);
  camera->transform.rotation = glm::angleAxis(glm::radians<float>(145), glm::vec3(0,1,0)) * glm::angleAxis(glm::radians<float>(-30), glm::vec3(1,0,0));
  Core::SetCamera(camera);

  Core::ambientLight = {.1,.1,.1, .1};
  Core::lights = {
    Light{{0,0,0}, {1,0,0}, 100.0f, 1.0f},
    Light{{0,0,0}, {0,1,0}, 100.0f, 1.0f},
    Light{{0,0,0}, {0,0,1}, 100.0f, 1.0f},
    Light{{0,0,0}, {1,1,0}, 100.0f, 1.0f},
    Light{{0,0,0}, {1,0,1}, 100.0f, 1.0f},
    Light{{0,0,0}, {0,1,1}, 100.0f, 1.0f},
  };
  Core::UpdateDynamicLighting();

  Mesh meshA;
  Mesh meshB;
  Mesh meshC;
  MeshRenderer meshRendererA = MeshRenderer(nullptr, Core::GetShader("Lit"));
  MeshRenderer meshRendererB = MeshRenderer(nullptr, Core::GetShader("Lit"));
  MeshRenderer meshRendererC = MeshRenderer(nullptr, Core::GetShader("Lit"));

  std::vector<MeshRenderer*> meshes = {&meshRendererA, &meshRendererB, &meshRendererC};
  std::vector<std::vector<Transform>> meshInstances;

  { //Load Camera
    std::ifstream file(System::PATH+"/assets/meshes/camera.obj");

    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec3> temp_normals;

    std::string str;
    while(std::getline(file, str)) {
      const uint32_t strSize = str.size();

      for(int i = 0; i < strSize; i++) {
        char &ch = str[i];
        if(ch != ' ') continue;

        const std::string prefix = str.substr(0,i);
        if(prefix == "v") {
          const std::vector<std::string> parts = split_string(str.substr(i + 1), ' ');
          temp_positions.push_back({std::stof(parts[0]),std::stof(parts[1]),std::stof(parts[2])});
        }
        else if(prefix == "vn") {
          const std::vector<std::string> parts = split_string(str.substr(i + 1), ' ');
          temp_normals.push_back({std::stof(parts[0]),std::stof(parts[1]),std::stof(parts[2])});
        }
        else if(prefix == "f") {
          const std::vector<std::string> parts = split_string(str.substr(i + 1), ' ');

          for(const std::string &part : parts) {
            const std::vector<std::string> subParts = split_string(part, '/');

            int vIndex = std::stoi(subParts[0]) - 1;
            int uIndex = std::stoi(subParts[1]) - 1;
            int nIndex = std::stoi(subParts[2]) - 1;

            meshA.vertices.push_back(Mesh::Vertex{temp_positions[vIndex], {1,1,1,1}, -temp_normals[nIndex]});
            meshA.indices.push_back(meshA.vertices.size() - 1);
          }
        }

        break;
      }
    }

    file.close();

    meshes[0]->setMesh(&meshA);
  }
  { //Load UV Sphere
    meshB = MeshHelper::GenerateUVSphere(16,16, .5f);
    meshes[1]->setMesh(&meshB);
  }
  { //Load Pyramid
    // meshC = MeshHelper::GeneratePyramid(4,1, .5f);
    meshC = MeshHelper::GenerateCylinder();
    meshes[2]->setMesh(&meshC);
  }

  { //Generate Mesh Instances
    static constexpr uint64_t SPAWN_COUNT = 500;
    static constexpr float SPAWN_RADIUS = 50;
    static constexpr float ROTATION_RANGE = 180;
    static constexpr float SCALE_RANGE_MIN = .1f;
    static constexpr float SCALE_RANGE_MAX = 2.5f;
    
    std::mt19937 gen(std::chrono::high_resolution_clock().now().time_since_epoch().count());
    std::uniform_real_distribution<float> randRadius(-SPAWN_RADIUS, SPAWN_RADIUS);
    std::uniform_real_distribution<float> randSpin(-ROTATION_RANGE, ROTATION_RANGE);
    std::uniform_real_distribution<float> randSize(SCALE_RANGE_MIN, SCALE_RANGE_MAX);


    const unsigned int meshCount = meshes.size();
    meshInstances.resize(meshCount);
    for(int i = 0; i < meshCount; i++) {
      meshInstances[i].resize(SPAWN_COUNT);

      for(int j = 0; j < SPAWN_COUNT; j++) {
        const glm::vec3 randomPos = {randRadius(gen),randRadius(gen),randRadius(gen)};
        const glm::quat randomRot = glm::angleAxis(glm::radians<float>(randSpin(gen)), glm::vec3(0,1,0)) * glm::angleAxis(glm::radians<float>(randSpin(gen)), glm::vec3(1,0,0));

        meshInstances[i][j] = Transform(randomPos, randomRot, glm::vec3(randSize(gen)));
      }

      meshes[i]->updateInstancingData(meshInstances[i]);
    }
  }


  while(Core::window->isOpen()) {
    while(const auto &eventOpt = Core::window->pollEvent()) {
      const auto &event = *eventOpt;

      if(event.is<sf::Event::Closed>()) Core::window->close();
      else if(const auto *resized = event.getIf<sf::Event::Resized>()) Core::UpdateWindowSize();

      else if(const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) User::HandleKeyPressed(keyPressed);
      else if(const auto *mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) User::HandleMouseButtonPressed(mouseButtonPressed);
    }


    { //Update
      static std::optional<glm::ivec2> mouseLockPosition;

      System::update();
      if(mouseLockPosition.has_value() == false) User::Mouse::update();
      else User::Mouse::update(mouseLockPosition.value());

      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) { //GPU Instancing Updates
        for(int i = 0; i < meshes.size(); i++) {
          for(Transform &_instance : meshInstances[i]) {
            _instance.position -= _instance.position * System::deltaTime * .5f;
          }

          meshes[i]->updateInstancingData(meshInstances[i]);
        }
      }

      { //Dynamic Lighting
        static constexpr float MOVE_RADIUS = 75;

        static float animationT = 0;
        animationT += System::deltaTime;
        const float c = glm::cos(animationT);
        const float s = glm::sin(animationT);

        const std::vector<glm::vec3> points = {
          glm::vec3(0, c * MOVE_RADIUS, s * MOVE_RADIUS),
          glm::vec3(c * MOVE_RADIUS, 0, s * MOVE_RADIUS),
          glm::vec3(c * MOVE_RADIUS, s * MOVE_RADIUS, 0),
          -glm::vec3(0, c * MOVE_RADIUS, s * MOVE_RADIUS),
          -glm::vec3(c * MOVE_RADIUS, 0, s * MOVE_RADIUS),
          -glm::vec3(c * MOVE_RADIUS, s * MOVE_RADIUS, 0),
        };
        for(int i = 0; i < Core::lights.size(); i++) Core::lights[i].position = points[i];

        Core::UpdateDynamicLighting();
      } //Dynamic Lighting

      if(Core::window->hasFocus() == true) { //Camera Movement
        static constexpr float sensitivity = 1;
        static constexpr float slowModeSpeed = 10;
        static constexpr float fastModeSpeed = 20;

        if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
          mouseLockPosition = Core::windowSize / 2;

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

          if(glm::length(glm::vec2(User::Mouse::delta)) != 0) {
            const glm::vec2 lookDelta = -glm::vec2(User::Mouse::delta) * sensitivity * System::deltaTime;

            const glm::quat pitch = glm::angleAxis(lookDelta.y, glm::vec3(1,0,0));
            const glm::quat yaw = glm::angleAxis(lookDelta.x, glm::vec3(0,1,0));
            camera->transform.rotation = glm::normalize(yaw * camera->transform.rotation * pitch);
          }

          Core::UpdateViewMatrix();
        }
        else if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {
          mouseLockPosition = Core::windowSize / 2;

          if(glm::length(glm::vec2(User::Mouse::delta)) != 0) {
            const glm::vec2 lookDelta = glm::vec2(User::Mouse::delta.x, -User::Mouse::delta.y) * sensitivity * System::deltaTime;
            camera->transform.position -= camera->transform.rotation * glm::vec3(lookDelta.x, lookDelta.y, 0);

            Core::UpdateViewMatrix();
          }      
        }
        else mouseLockPosition = std::nullopt;
      }
    }

    { //Render
      Core::clear();

      Core::SetState(Renderer::State::OpenGL); {
        for(const MeshRenderer *renderer : meshes) renderer->drawInstanced();

        if(System::ShowMeshNormals) {
          Mesh normalMesh = MeshHelper::GenerateNormalGizmos(meshC, {0,0,1,1});
          MeshRenderer normalRenderer(&normalMesh, Core::GetShader("Unlit"));
          normalRenderer.draw(glm::mat4x4(1));
        }
        if(System::ShowLightGizmos) {
          Mesh lightMesh = MeshHelper::GenerateUVSphere(8,8,.25f);
          MeshRenderer lightRenderer(&lightMesh, Core::GetShader("Unlit"));
          lightRenderer.backFaceCulling = false;

          for(const Light &light : Core::lights) {
            MeshHelper::RandomizeMeshColors(lightMesh, {glm::vec4(light.color, 1)});
            lightRenderer.updateMeshData();
            lightRenderer.draw(Transform(light.position).getMatrix());
          }
        }
        if(true) { //Show Transform Gizmos
          // Mesh mesh(MeshType::Lines);
          // mesh.vertices = {
          //   Mesh::Vertex{{0,0,0}, {1,0,0,1}},
          //   Mesh::Vertex{transform.right(), {1,0,0,1}},

          //   Mesh::Vertex{{0,0,0}, {0,1,0,1}},
          //   Mesh::Vertex{transform.up(), {0,1,0,1}},

          //   Mesh::Vertex{{0,0,0}, {0,0,1,1}},
          //   Mesh::Vertex{transform.forward(), {0,0,1,1}},
          // };
          // mesh.indices = {0,1,2,3,4,5};

          // MeshRenderer transformRenderer(&mesh, Core::GetShader("Unlit"));
          // transformRenderer.draw(transform.getMatrix());
        }
      }

      UI::Core::draw();
      if constexpr (false) { //Draw Old UI
        Core::SetState(Renderer::State::UI); 
        const sf::Vector2f size = sf::Vector2f{Core::windowSize.x / 6.0f, static_cast<float>(Core::windowSize.y)};

        sf::RectangleShape background(size);
        background.setFillColor(sf::Color(15,15,15));

        sf::Font font;
        sf::Text text(font);
        const uint32_t charSize = 15;
        const uint32_t elementPad = 5;
        const sf::Vector2f listPad = sf::Vector2f{5,15};
        text.setCharacterSize(charSize);

        { //Left
          background.setPosition(sf::Vector2f{0,0});
          Core::window->draw(background);

          const std::vector<std::string> elements = {
            BoolString("F1| Wireframe Mode", Core::WireframeMode),
            BoolString("F2| Show Normals", System::ShowMeshNormals),
            BoolString("F3| Light Gizmos", System::ShowLightGizmos),
          };

          for(int i = 0; i < elements.size(); i++) {
            text.setString(elements[i]);
            text.setPosition(listPad + sf::Vector2f{0, static_cast<float>((charSize + elementPad) * i)});
            Core::window->draw(text);
          }
        } //Left
        { //Right
          background.setPosition(sf::Vector2f{static_cast<float>(Core::windowSize.x - size.x),0});
          Core::window->draw(background);

          const glm::vec3 &camPos = glm::vec3(glm::ivec3(Core::camera->transform.position * 100.0f)) / 100.0f;
          glm::vec3 camEuler;
          glm::extractEulerAngleYXZ(glm::mat4_cast(camera->transform.rotation), camEuler.y, camEuler.x, camEuler.z);
          camEuler *= (180.0f / glm::pi<float>());

          const std::vector<std::string> elements = {
            std::format("Editor Cam Pos {}", Vec3String(camPos)),
            std::format("Editor Cam Rot {}", Vec3String(camEuler)),
          };

          const sf::Vector2f start = background.getPosition();

          for(int i = 0; i < elements.size(); i++) {
            text.setString(elements[i]);
            text.setPosition(start + listPad + sf::Vector2f{0, static_cast<float>((charSize + elementPad) * i)});
            Core::window->draw(text);
          }
        } //Right
      }

      Core::display();
    }
  }

  Core::end();

  return 0;
}


//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS