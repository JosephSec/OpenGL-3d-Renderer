#include <System.hpp>
#include <User.hpp>
#include <Engine/Renderer.hpp>
#include <Editor.hpp>

#include <SFML/Graphics.hpp>

#include <iostream>


static void init() {
  System::init();
  Renderer::init();
  User::init();

  Editor::init();
}


int main(int argc, char *argv[]) {
  init();


  const std::vector<glm::vec4> colors = {
    {1,0,0, 1},
    {0,1,0, 1},
    {0,0,1, 1},
    {1,1,1, 1},
    {0,0,0, 1},
  };

  MeshRenderer primitiveRenderer(nullptr, &Renderer::UnlitShader);
  primitiveRenderer.backFaceCulling = true;

  const std::filesystem::path primitiveMeshPath = std::filesystem::path(System::PATH)/"assets"/"meshes"/"cube.mesh";
  Mesh primitiveMesh; {
    Editor::LoadMeshPrimitive(primitiveMesh, primitiveMeshPath);

    for(int i = 0; i < primitiveMesh.vertices.size(); i++) {
      primitiveMesh.vertices[i].color = colors[i % colors.size()];
    }
  
    primitiveRenderer.setMesh(&primitiveMesh);
  }
  // Editor::SaveMeshPrimitive(primitiveMesh, primitiveMeshPath);

  MeshRenderer circleMeshRenderer(nullptr, &Renderer::UnlitShader);
  Mesh circleMesh; {
    Editor::LoadMeshPrimitive(circleMesh, std::filesystem::path(System::PATH)/"assets"/"meshes"/"circle.mesh");
    for(int i = 0; i < circleMesh.vertices.size(); i++) circleMesh.vertices[i].color = {1,0,0,1};
    circleMeshRenderer.setMesh(&circleMesh);
  }


  MeshRenderer groundMeshRenderer(nullptr, &Renderer::UnlitShader);
  Mesh groundMesh; {
    Editor::LoadMeshPrimitive(groundMesh, std::filesystem::path(System::PATH)/"assets"/"meshes"/"quad.mesh");
    groundMeshRenderer.setMesh(&groundMesh);
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

      animationT += System::deltaTime * .1;
    }

    { //Render
      Renderer::clear();

      Renderer::SetShader(Renderer::UnlitShader);

      Editor::draw();

      primitiveRenderer.draw(glm::mat4x4(1));

      // const glm::quat rotation = Renderer::camera->transform.rotation;
      // for(int i = 0; i < primitiveMesh.vertices.size(); i++) {
      //   const glm::vec3 vertPosition = primitiveMesh.vertices[i].position;
      //   const glm::quat rotation = glm::quatLookAt(glm::normalize(vertPosition - Renderer::camera->transform.position), glm::vec3(0,1,0));
      //   circleMeshRenderer.draw(Transform(vertPosition, rotation, glm::vec3(.1)).getMatrix());
      // }

      const glm::vec3 size = glm::vec3(30 + glm::cos(animationT) * 10, 30 + glm::cos(animationT) * 10, 1);
      groundMeshRenderer.draw(Transform(glm::vec3(0), glm::rotate(glm::mat4x4(1), glm::radians<float>(-90), glm::vec3(1,0,0)), size).getMatrix());

      if(true) { //Weird Grid
        const int16_t GRID_SIZE = 25;
        for(int x = -GRID_SIZE; x < GRID_SIZE; x++) {
          for(int z = -GRID_SIZE; z <= GRID_SIZE; z++) {
            const double a = glm::cos(static_cast<float>(x));
            const double b = glm::cos(static_cast<float>(z));
            const double c = glm::cos(animationT);
            const double maxAngle = 180;
            const double startAngle = 90;
            const double angle = glm::radians((startAngle + (c * a * b) * maxAngle) * c);

            const glm::vec3 position = 
              glm::angleAxis(animationT * (x * z) * System::deltaTime * System::deltaTime, glm::vec3(1,0,0)) *
              // glm::angleAxis(animationT, glm::vec3(0,1,0)) *
              glm::vec3(x,0,z);
            primitiveRenderer.draw(Transform(position, glm::rotate(glm::mat4x4(1), static_cast<float>(angle), glm::vec3(1,0,0)), glm::vec3(1)).getMatrix());
          }
        }
      }

      Renderer::display();
    }
  }

  Renderer::end();
  Editor::end();

  return 0;
}