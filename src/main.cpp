#include <System.hpp>
#include <User.hpp>

#include <Engine/Renderer.hpp>
#include <Engine/Renderer/Mesh.hpp>
#include <Engine/Renderer/MeshRenderer.hpp>

#include <SFML/Graphics.hpp>

#include <fstream>
#include <iostream>


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
  Renderer::init({800 + 225 * 2, 600}, "Game Engine");
  User::init();
}
int main(int argc, char *argv[]) {
  init();

  sf::Font font;
  if(font.openFromFile(System::PATH+"/assets/Roboto.ttf") == false) {
    std::cout << "[UI Error]: Font file was not found or could not be opened\n";
  }


  Camera *camera = new Camera();
  camera->fov = 90;
  camera->transform.position = glm::vec3(2,1,-2);
  camera->transform.rotation = glm::angleAxis(glm::radians<float>(45 + 90), glm::vec3(0,1,0)) * glm::angleAxis(glm::radians<float>(-15), glm::vec3(1,0,0));
  Renderer::SetCamera(camera);

  Renderer::ambientLight = {1,1,1,1};
  Renderer::lights.push_back(Light{
    {1,1,-1},
    {1,0,0,1},
    20
  });
  Renderer::UpdateDynamicLighting();

  Mesh mesh;
  MeshRenderer meshRenderer = MeshRenderer(nullptr, Renderer::GetShader("Lit"));

  if constexpr(false) { //Load Camera Mesh
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

            mesh.vertices.push_back(Mesh::Vertex{temp_positions[vIndex], {1,1,1,1}, temp_normals[nIndex]});
            mesh.indices.push_back(mesh.vertices.size() - 1);
          }
        }

        break;
      }
    }

    file.close();

    meshRenderer.setMesh(&mesh);
  }
  if constexpr(true) { //Load Triangle Mesh
    mesh.vertices = {
      Mesh::Vertex{{ 1,-1,0}, {1,1,1,1}, {0,0,1}},
      Mesh::Vertex{{ 0, 1,0}, {1,1,1,1}, {0,0,1}},
      Mesh::Vertex{{-1,-1,0}, {1,1,1,1}, {0,0,1}},
    };
    mesh.indices = {0,1,2};

    meshRenderer.setMesh(&mesh);
  }


  while(Renderer::window->isOpen()) {
    while(const auto &eventOpt = Renderer::window->pollEvent()) {
      const auto &event = *eventOpt;

      if(event.is<sf::Event::Closed>()) Renderer::window->close();
      else if(event.is<sf::Event::KeyPressed>()) User::HandleEvent(event);
      else if(const auto *resized = event.getIf<sf::Event::Resized>()) Renderer::UpdateWindowSize();
    }


    { //Update
      System::update();
      User::Mouse::update();
    }

    { //Render
      Renderer::clear();

      Renderer::SetState(RenderState::OpenGL); {
        meshRenderer.draw(Transform().getMatrix());
        { //Normals
          Mesh normalMesh(MeshType::Lines);
          
          glm::vec3 normal = mesh.vertices[mesh.indices[0]].normal;
          glm::vec3 center = (mesh.vertices[mesh.indices[0]].position +
                              mesh.vertices[mesh.indices[1]].position +
                              mesh.vertices[mesh.indices[2]].position) / 3.0f;

          normalMesh.vertices.push_back(Mesh::Vertex{center, {0,0,1,1}});
          normalMesh.vertices.push_back(Mesh::Vertex{center + normal, {0,0,1,1}});

          normalMesh.indices.push_back(0);
          normalMesh.indices.push_back(1);

          MeshRenderer normalRenderer(&normalMesh, Renderer::GetShader("Unlit"));
          normalRenderer.draw(Transform().getMatrix());
        } //Normals
      }


      Renderer::SetState(RenderState::UI); {
        const sf::Vector2f size = sf::Vector2f{Renderer::windowSize.x / 6.0f, static_cast<float>(Renderer::windowSize.y)};

        sf::RectangleShape background(size);
        background.setFillColor(sf::Color(15,15,15));

        sf::Text text(font);
        const uint32_t charSize = 15;
        const uint32_t elementPad = 5;
        const sf::Vector2f listPad = sf::Vector2f{5,15};
        text.setCharacterSize(charSize);

        { //Left
          background.setPosition(sf::Vector2f{0,0});
          Renderer::window->draw(background);

          const std::vector<std::string> elements = {
            BoolString("F1| Wireframe Mode", Renderer::WireframeMode),
          };

          for(int i = 0; i < elements.size(); i++) {
            text.setString(elements[i]);
            text.setPosition(listPad + sf::Vector2f{0, static_cast<float>((charSize + elementPad) * i)});
            Renderer::window->draw(text);
          }
        } //Left
        { //Right
          background.setPosition(sf::Vector2f{static_cast<float>(Renderer::windowSize.x - size.x),0});
          Renderer::window->draw(background);

          const glm::vec3 &camPos = glm::vec3(glm::ivec3(Renderer::camera->transform.position * 100.0f)) / 100.0f;

          const std::vector<std::string> elements = {
            std::format("Editor Cam Pos ({}, {}, {})", camPos.x,camPos.y,camPos.z),
          };

          const sf::Vector2f start = background.getPosition();

          for(int i = 0; i < elements.size(); i++) {
            text.setString(elements[i]);
            text.setPosition(start + listPad + sf::Vector2f{0, static_cast<float>((charSize + elementPad) * i)});
            Renderer::window->draw(text);
          }
        } //Right
      }

      Renderer::display();
    }
  }

  Renderer::end();

  return 0;
}