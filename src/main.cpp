#include <System.hpp>
#include <User.hpp>
#include <Engine/Renderer.hpp>
#include <Editor.hpp>
#include <SceneManager.hpp>
#include <Engine/Scene.hpp>
#include <Engine/PrimitiveMeshGenerator.hpp>

#include <SFML/Graphics.hpp>
#include <random>
#include <iostream>


static void init() {
  System::init();
  Renderer::init();
  User::init();

  Editor::init();
  // SceneManager::init();
}


int main(int argc, char *argv[]) {
  init();

  if constexpr(false) { //Remake Primitives
    Mesh mesh = GeneratePyramid();
    Editor::SaveMeshPrimitive(mesh, "pyramid");
    SceneManager::TestPrimitiveMeshLoading();
  }
  

  Scene scene;

  static float animationT = 0;

  { //Load Meshes
    Mesh mesh;
    Editor::LoadMeshPrimitive(mesh, "cube");
    scene.LoadMeshToScene("cube", mesh);

    Editor::LoadMeshPrimitive(mesh, "quad");
    scene.LoadMeshToScene("quad", mesh);
  }

  { //Cube
    scene.gameObjects.emplace_back(GameObject{MeshRenderer(scene.GetMeshFromScene("cube"), &Renderer::UnlitShader), Transform()});
    scene.gameObjects.back().m_update = [](GameObject* go) {
      go->transform.position.y = 1.5 + glm::sin(animationT);
      go->transform.rotation = glm::angleAxis(animationT, glm::vec3(0,1,0));
    };
    scene.gameObjects.back().m_draw = [](const GameObject* go) {
      go->meshRenderer.draw(go->transform.getMatrix());
    };
  }
  { //Ground
    GameObject gameObject = GameObject{MeshRenderer(scene.GetMeshFromScene("quad"), &Renderer::UnlitShader), Transform()};
    gameObject.transform.rotation = glm::angleAxis(glm::radians<float>(-90), glm::vec3(1,0,0));
    gameObject.transform.scale = glm::vec3(30,30,1);

    scene.gameObjects.push_back(gameObject);
    scene.gameObjects.back().m_update = [](GameObject* go) {};
    scene.gameObjects.back().m_draw = [](const GameObject* go) {
      go->meshRenderer.draw(go->transform.getMatrix());
    };
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
      Editor::update();
      // SceneManager::update();

      animationT += System::deltaTime;
      scene.update();
    }

    { //Render
      Renderer::clear();

      // SceneManager::draw();
      scene.draw();
      Editor::draw();

      Renderer::display();
    }
  }

  Renderer::end();
  Editor::end();
  // SceneManager::end();

  return 0;
}