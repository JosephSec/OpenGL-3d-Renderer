#include <System.hpp>
#include <User.hpp>
#include <Engine/Renderer.hpp>
#include <Editor.hpp>
#include <SceneManager.hpp>

#include <SFML/Graphics.hpp>

#include <iostream>

#include <random>


static void init() {
  System::init();
  Renderer::init();
  User::init();

  Editor::init();
}


int main(int argc, char *argv[]) {
  init();

  Mesh temp_mesh;
  for(const auto &pair : Editor::primitiveMeshPaths) {
    Editor::LoadMeshPrimitive(temp_mesh, pair.first);
    Editor::SaveMeshPrimitive(temp_mesh, pair.first);
  }


  const std::vector<glm::vec4> colors = {
    {1,0,0, 1},
    {0,1,0, 1},
    {0,0,1, 1},
    {1,1,1, 1},
    {0,0,0, 1},
  };

  MeshRenderer primitiveRenderer(nullptr, &Renderer::UnlitShader);
  primitiveRenderer.backFaceCulling = true;

  Mesh primitiveMesh; {
    Editor::LoadMeshPrimitive(primitiveMesh, "cube");
    // Editor::RandomizeMeshColors(primitiveMesh, colors);  
    primitiveRenderer.setMesh(&primitiveMesh);
    Editor::SaveMeshPrimitive(primitiveMesh, "cube");
  }


  MeshRenderer circleMeshRenderer(nullptr, &Renderer::UnlitShader);
  Mesh circleMesh; {
    Editor::LoadMeshPrimitive(circleMesh, "circle");
    Editor::RandomizeMeshColors(primitiveMesh, {glm::vec4(1,0,0,1)});  
    circleMeshRenderer.setMesh(&circleMesh);
  }


  GameObject groundObject;
  Mesh groundMesh; {
    Editor::LoadMeshPrimitive(groundMesh, "quad");
    
    sf::Texture texture;

    MeshRenderer groundMeshRenderer(&groundMesh, &Renderer::UnlitShader);
    groundObject = GameObject{groundMeshRenderer, Transform(glm::vec3(0), glm::rotate(glm::mat4x4(1), glm::radians<float>(-90), glm::vec3(1,0,0)), glm::vec3(30,30,1))};
    SceneManager::gameObjects.push_back(&groundObject);
  }

  GameObject itemDropObject;
  Mesh itemDropMesh; {
    Editor::LoadMeshPrimitive(itemDropMesh, "cube");
    Editor::RandomizeMeshColors(itemDropMesh, colors);
    
    MeshRenderer groundMeshRenderer(&itemDropMesh, &Renderer::UnlitShader);
    itemDropObject = GameObject{groundMeshRenderer, Transform(glm::vec3(0,1,-5), glm::quat(), glm::vec3(.5f))};
    SceneManager::gameObjects.push_back(&itemDropObject);
  }

  
  Camera *playerCamera = new Camera(glm::vec3(0,2,0));
  float playerCameraXRotation = 0;
  Editor::playModeCamera = playerCamera;
  Editor::cameras.push_back(playerCamera);

  GameObject playerObject;
  Mesh playerMesh; {
    Editor::LoadMeshPrimitive(playerMesh, "cylinder");
    Editor::RandomizeMeshColors(playerMesh, {glm::vec4(1,0,0,1)});
    
    MeshRenderer playerRenderer(&playerMesh, &Renderer::UnlitShader);
    playerObject = GameObject{playerRenderer, Transform(glm::vec3(0,1,0), glm::quat(), glm::vec3(.5,1,.5))};
    SceneManager::gameObjects.push_back(&playerObject);
  }

  float animationT = 0;


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

      animationT += System::deltaTime * 100;


      if(Editor::playMode) {
        if(true) { //Item Drop Movement
          itemDropObject.transform.position = glm::vec3(0,1 + glm::sin(glm::radians<float>(animationT)) * .25,-5);
          itemDropObject.transform.rotation = glm::angleAxis(glm::radians<float>(animationT), glm::vec3(0,1,0));
        }

        if(true) { //Player Movement
          glm::vec3 moveDir = glm::vec3(0);
          if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) moveDir -= glm::vec3(0,0,1);
          if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) moveDir -= glm::vec3(1,0,0);
          if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) moveDir += glm::vec3(0,0,1);
          if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) moveDir += glm::vec3(1,0,0);
          if(glm::length(moveDir) != 0) {
            const float speed = System::deltaTime * (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)? 5 : 2);
            playerObject.transform.position += (playerObject.transform.rotation * glm::normalize(moveDir)) * speed;
            playerCamera->transform.position = playerObject.transform.position + glm::vec3(0,1,0);

            Renderer::UpdateViewMatrix();
          }

          if(glm::length(glm::vec2(User::Mouse::delta)) != 0) {
            const glm::vec2 lookDelta = -glm::vec2(User::Mouse::delta) * Editor::sensitivity * System::deltaTime;
            playerCameraXRotation = glm::clamp(playerCameraXRotation + lookDelta.y, glm::radians<float>(-90), glm::radians<float>(90));

            const glm::quat yaw = glm::angleAxis(lookDelta.x, glm::vec3(0,1,0));
            const glm::quat pitch = glm::angleAxis(playerCameraXRotation, glm::vec3(1,0,0));

            playerObject.transform.rotation = glm::normalize(playerObject.transform.rotation * yaw);
            playerCamera->transform.rotation = glm::normalize(playerObject.transform.rotation * pitch);

            Renderer::UpdateViewMatrix();
          }
        }
      }
    }

    { //Render
      Renderer::clear();

      Renderer::SetShader(Renderer::UnlitShader);

      Editor::draw();

      const glm::quat rotation = Renderer::camera->transform.rotation;
      for(int i = 0; i < itemDropMesh.vertices.size(); i++) {
        const glm::vec3 vertPosition = itemDropMesh.vertices[i].position;
        const glm::quat rotation = glm::quatLookAt(glm::normalize(vertPosition - Renderer::camera->transform.position), glm::vec3(0,1,0));
        circleMeshRenderer.draw(Transform(vertPosition, rotation, glm::vec3(.1)).getMatrix());
      }

      SceneManager::draw();

      Renderer::display();
    }
  }

  Renderer::end();
  Editor::end();

  return 0;
}