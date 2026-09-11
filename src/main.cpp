#include <System.hpp>
#include <User.hpp>

#include <Engine/Renderer.hpp>
#include <Engine/Editor.hpp>

#include <Engine/SceneManager.hpp>
#include <Engine/SceneManager/Scene.hpp>
#include <Engine/SceneManager/PrimitiveMeshGenerator.hpp>

#include <Engine/Physics.hpp>

#include <SFML/Graphics.hpp>
#include <random>
#include <iostream>


static std::string BoolString(const std::string &_str, bool _val) {
  return _str + ": " + std::string(_val? "True" : "False");
}


static void init() {
  System::init();
  Renderer::init({800 + 225 * 2, 600}, "Game Engine");
  User::init();

  Editor::init(std::filesystem::path(System::PATH));
  SceneManager::init();
  Physics::init();
}
int main(int argc, char *argv[]) {
  init();

  Editor::camera->transform.position = glm::vec3(5,5,5);
  Editor::camera->transform.rotation = glm::angleAxis(glm::radians<float>(45), glm::vec3(0,1,0)) * glm::angleAxis(glm::radians<float>(-30), glm::vec3(1,0,0));
  Renderer::UpdateViewMatrix();

  sf::Font font;
  if(font.openFromFile(System::PATH+"/assets/Roboto.ttf") == false) {
    std::cout << "[UI Error]: Font file was not found or could not be opened\n";
  }


  if constexpr(false) { //Remake Primitives
    Mesh mesh = MeshGenerator::Pyramid();
    Editor::SaveMeshPrimitive(mesh, "pyramid");
    SceneManager::TestPrimitiveMeshLoading();
  }


  static float itemDropDist = 3;
  static float animationT = 0;
  { //Init Scene
    { //Load Meshes
      Mesh mesh;
      Editor::LoadMeshPrimitive(mesh, "cube");
      SceneManager::scene.LoadMeshToScene("Cube", mesh);

      Editor::LoadMeshPrimitive(mesh, "quad");
      Editor::RandomizeMeshColors(mesh, {{0,0,0, 1}});
      SceneManager::scene.LoadMeshToScene("Ground", mesh);

      Editor::LoadMeshPrimitive(mesh, "cylinder");
      Editor::RandomizeMeshColors(mesh, {{1,0,0, 1}});
      SceneManager::scene.LoadMeshToScene("Player", mesh);

      mesh = MeshGenerator::UVSphere();
      SceneManager::scene.LoadMeshToScene("UV-Sphere", mesh);
    }

    { //Item Drop
      GameObject gameObject = GameObject(MeshRenderer(SceneManager::scene.GetMeshFromScene("Cube"), &Renderer::UnlitShader));
      gameObject.name = "Item Drop";

      gameObject.transform.position = glm::vec3(0,1.5, -5);

      SceneManager::scene.gameObjects.push_back(gameObject);
      SceneManager::scene.gameObjects.back().m_update = [](GameObject* go) {
        go->transform.position = glm::vec3(0,1.5 + glm::sin(animationT), -5);
        go->transform.rotation = glm::angleAxis(animationT, glm::vec3(0,1,0));
      };
      SceneManager::scene.gameObjects.back().m_draw = [](const GameObject* go) {
        go->meshRenderer.draw(go->transform.getMatrix());
      };
      SceneManager::scene.gameObjects.back().m_drawGizmos = [](const GameObject* go) {
        Mesh mesh = MeshGenerator::WireSphere(24, itemDropDist);
        MeshRenderer(&mesh, &Renderer::UnlitShader).draw(go->transform.getMatrix());
      };
    }
    { //Ground
      GameObject gameObject = GameObject(MeshRenderer(SceneManager::scene.GetMeshFromScene("Ground"), &Renderer::UnlitShader));
      gameObject.name = "Ground";

      gameObject.transform.rotation = glm::angleAxis(glm::radians<float>(-90), glm::vec3(1,0,0));
      gameObject.transform.scale = glm::vec3(30,30,1);

      SceneManager::scene.gameObjects.push_back(gameObject);
      SceneManager::scene.gameObjects.back().m_update = [](GameObject* go) {};
      SceneManager::scene.gameObjects.back().m_draw = [](const GameObject* go) {
        go->meshRenderer.draw(go->transform.getMatrix());
      };
    }
    { //Player
      SceneManager::scene.camera = new Camera(glm::vec3(0,2,0));
      SceneManager::scene.camera->nearPlane = .25f;
      Editor::cameras.push_back(SceneManager::scene.camera);

      GameObject gameObject = GameObject(MeshRenderer(SceneManager::scene.GetMeshFromScene("Player"), &Renderer::UnlitShader));
      gameObject.name = "Player";

      gameObject.transform.position = glm::vec3(0,1,0);
      gameObject.transform.scale = glm::vec3(1,2,1);

      SceneManager::scene.gameObjects.push_back(gameObject);
      SceneManager::scene.gameObjects.back().m_update = [](GameObject* go) {
        if(Editor::IsActiveCamera()) return; //NEEDS TO BE REMOVED! CREATE GLOBAL RUNTIME VARIABLES FOR PLAYMODE

        { //Body Movement
          glm::vec3 moveDir = glm::vec3(0);
          if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) moveDir -= glm::vec3(0,0,1);
          if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) moveDir -= glm::vec3(1,0,0);
          if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) moveDir += glm::vec3(0,0,1);
          if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) moveDir += glm::vec3(1,0,0);
          if(glm::length(moveDir) != 0) {
            const float speed = System::deltaTime * (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)? 5 : 2);
            go->transform.position += (go->transform.rotation * glm::normalize(moveDir)) * speed;
            SceneManager::scene.camera->transform.position = go->transform.position + glm::vec3(0,1,0);

            Renderer::UpdateViewMatrix();
          }
        }

        { //Camera Movement
          if(glm::length(glm::vec2(User::Mouse::delta)) != 0) {
            const glm::vec2 lookDelta = -glm::vec2(User::Mouse::delta) * Editor::sensitivity * System::deltaTime;
            SceneManager::playerCameraXRotation = glm::clamp(SceneManager::playerCameraXRotation + lookDelta.y, glm::radians<float>(-90), glm::radians<float>(90));

            const glm::quat yaw = glm::angleAxis(lookDelta.x, glm::vec3(0,1,0));
            const glm::quat pitch = glm::angleAxis(SceneManager::playerCameraXRotation, glm::vec3(1,0,0));

            go->transform.rotation = glm::normalize(go->transform.rotation * yaw);
            SceneManager::scene.camera->transform.rotation = glm::normalize(go->transform.rotation * pitch);

            Renderer::UpdateViewMatrix();
          }
        }
      
        { //Item Drop Interaction
          for(GameObject &gameObject : SceneManager::scene.gameObjects) {
            if(gameObject.name != "Item Drop" || gameObject.isActive == false) continue;

            if(glm::length(gameObject.transform.position - go->transform.position) <= itemDropDist) {
              gameObject.isActive = false;
            }

            break;
          }
        }
      };
      SceneManager::scene.gameObjects.back().m_draw = [](const GameObject* go) {
        go->meshRenderer.draw(go->transform.getMatrix());
      };
    }
  
    { //Rigidbodys
      Physics::gravity = glm::vec3(0,-9.806,0);

      for(int i = 0; i < 10; i++) {
        SceneManager::scene.rigidbodys.push_back(Rigidbody(Transform(glm::vec3(-5 - i, 5, -5)), 1, glm::vec3(0,i,-i)));
      }
    }
  }
  

  while(Renderer::window->isOpen()) {
    while(const auto &eventOpt = Renderer::window->pollEvent()) {
      const auto &event = *eventOpt;

      if(event.is<sf::Event::Closed>()) Renderer::window->close();
      else if(event.is<sf::Event::KeyPressed>()) User::HandleEvent(event);
      else if(const auto *resized = event.getIf<sf::Event::Resized>()) Renderer::HandleResize();
    }


    { //Update
      System::update();
      User::Mouse::update();
      Editor::update(System::deltaTime, User::Mouse::delta);

      if(Editor::PlayModePaused == false) {
        animationT += System::deltaTime;

        SceneManager::update();
        Physics::update();
      }
    }

    { //Render
      Renderer::clear();

      Renderer::SetState(RenderState::OpenGL); {
        SceneManager::draw();
        Editor::draw();
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
            BoolString("F1| Play Mode Paused", Editor::PlayModePaused),
            BoolString("F2| Play Mode Focused", Editor::PlayModeFocused),
            BoolString("F3| Play Mode Gizmos", Editor::PlayModeGizmos),
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

          const glm::vec3 &camPos = glm::vec3(glm::ivec3(Editor::camera->transform.position * 100.0f)) / 100.0f;

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
  Editor::end();
  SceneManager::end();

  return 0;
}