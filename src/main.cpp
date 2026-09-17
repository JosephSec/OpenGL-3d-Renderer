#include <System.hpp>
#include <User.hpp>

#include <SFML/Graphics.hpp>

#include <fstream>
#include <iostream>

#include <Engine/Renderer.hpp>
#include <Engine/Renderer/Mesh.hpp>
#include <Engine/Renderer/MeshRenderer.hpp>
#include <Engine/Renderer/MeshHelper.hpp>
using namespace Renderer;


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


static void init() {
  System::init();
  Core::init({800 + 225 * 2, 600}, "Game Engine");
  User::init();

  Core::window->setVerticalSyncEnabled(true);
}
int main(int argc, char *argv[]) {
  init();

  sf::Font font;
  if(font.openFromFile(System::PATH+"/assets/Roboto.ttf") == false) {
    std::cout << "[UI Error]: Font file was not found or could not be opened\n";
  }


  Camera *camera = new Camera();
  camera->fov = 90;
  camera->transform.position = glm::vec3(3,3,-3);
  camera->transform.rotation = glm::angleAxis(glm::radians<float>(45 + 90), glm::vec3(0,1,0)) * glm::angleAxis(glm::radians<float>(-45), glm::vec3(1,0,0));
  Core::SetCamera(camera);

  Core::ambientLight = {0,0,1,.1};
  Core::lights.push_back(Light{
    {0,1,0},
    {1,0,0},
    5.0f,
    1.0f
  });
  Core::lights.push_back(Light{
    {0,1,0},
    {0,1,0},
    5.0f,
    1.0f
  });
  Core::UpdateDynamicLighting();

  Mesh mesh;
  MeshRenderer meshRenderer = MeshRenderer(nullptr, Core::GetShader("Lit"));

  if constexpr(true) { //Load Camera
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

            mesh.vertices.push_back(Mesh::Vertex{temp_positions[vIndex], {1,1,1,1}, -temp_normals[nIndex]});
            mesh.indices.push_back(mesh.vertices.size() - 1);
          }
        }

        break;
      }
    }

    file.close();
  }
  if constexpr(false) mesh = MeshHelper::GeneratePlane(glm::vec2(30,30), glm::ivec2(3,3));
  meshRenderer.setMesh(&mesh);


  while(Core::window->isOpen()) {
    while(const auto &eventOpt = Core::window->pollEvent()) {
      const auto &event = *eventOpt;

      if(event.is<sf::Event::Closed>()) Core::window->close();
      else if(event.is<sf::Event::KeyPressed>()) User::HandleEvent(event);
      else if(const auto *resized = event.getIf<sf::Event::Resized>()) Core::UpdateWindowSize();
    }


    { //Update
      static std::optional<glm::ivec2> mouseLockPosition;

      System::update();
      if(mouseLockPosition.has_value() == false) User::Mouse::update();
      else User::Mouse::update(mouseLockPosition.value());

      static float animationT = 0;
      animationT += System::deltaTime;
      Core::lights[0].position = glm::vec3(glm::cos(animationT) * 5, 1, glm::sin(animationT) * 5);
      Core::lights[1].position = glm::vec3(0, glm::cos(animationT) * 5, glm::sin(animationT) * 5);
      Core::UpdateDynamicLighting();

      { //Camera Movement
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
        Transform transform;
        meshRenderer.draw(transform.getMatrix());

        if(System::ShowMeshNormals) {
          Mesh normalMesh = MeshHelper::GenerateNormalLines(mesh, {0,0,1,1});
          MeshRenderer normalRenderer(&normalMesh, Core::GetShader("Unlit"));
          normalRenderer.draw(transform.getMatrix());
        }
        if(System::ShowLightGizmos) {
          Mesh lightMesh = MeshHelper::GenerateUVSphere(8,8,.25f);
          MeshRenderer lightRenderer(&lightMesh, Core::GetShader("Unlit"));
          lightRenderer.backFaceCulling = false;

          for(const Light &light : Core::lights) {
            MeshHelper::RandomizeMeshColors(lightMesh, {glm::vec4(light.color, 1)});
            lightRenderer.update();
            lightRenderer.draw(Transform(light.position).getMatrix());
          }
        }
      }

      Core::SetState(Renderer::State::UI); {
        const sf::Vector2f size = sf::Vector2f{Core::windowSize.x / 6.0f, static_cast<float>(Core::windowSize.y)};

        sf::RectangleShape background(size);
        background.setFillColor(sf::Color(15,15,15));

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

          const std::vector<std::string> elements = {
            std::format("Editor Cam Pos ({}, {}, {})", camPos.x,camPos.y,camPos.z),
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