#include <System.hpp>
#include <User.hpp>
#include <Engine/Renderer.hpp>
#include <Engine/Editor.hpp>
#include <Engine/SceneManager.hpp>
#include <Engine/SceneManager/Scene.hpp>
#include <Engine/SceneManager/PrimitiveMeshGenerator.hpp>

#include <SFML/Graphics.hpp>
#include <random>
#include <iostream>


static void init() {
  System::init();
  Renderer::init();
  User::init();

  Editor::init(std::filesystem::path(System::PATH));
  SceneManager::init();
}


int main(int argc, char *argv[]) {
  init();

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
    { //UV-Sphere
      GameObject gameObject = GameObject(MeshRenderer(SceneManager::scene.GetMeshFromScene("UV-Sphere"), &Renderer::UnlitShader));
      gameObject.name = "UV-Sphere";

      gameObject.transform.position = glm::vec3(3,3,-3);
      // gameObject.transform.scale = glm::vec3(.25f);

      SceneManager::scene.gameObjects.push_back(gameObject);
      SceneManager::scene.gameObjects.back().m_update = [](GameObject* go) {};
      SceneManager::scene.gameObjects.back().m_draw = [](const GameObject* go) {
        go->meshRenderer.draw(go->transform.getMatrix());
      };
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

      animationT += System::deltaTime;
      SceneManager::update();
    }

    { //Render
      Renderer::clear();

      SceneManager::draw();
      Editor::draw();

      Renderer::display();
    }
  }

  Renderer::end();
  Editor::end();
  // SceneManager::end();

  return 0;
}