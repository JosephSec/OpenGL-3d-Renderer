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

  const std::filesystem::path primitiveMeshPath = std::filesystem::path(System::PATH)/"assets"/"meshes"/"pyramid.mesh";
  Mesh primitiveMesh; {
    Editor::LoadMeshPrimitive(primitiveMesh, primitiveMeshPath);

    for(int i = 0; i < primitiveMesh.vertices.size(); i++) {
      primitiveMesh.vertices[i].color = colors[i % colors.size()];
    }
  
    primitiveRenderer.setMesh(&primitiveMesh);
  }
  // Editor::SaveMeshPrimitive(primitiveMesh, primitiveMeshPath);

  MeshRenderer linesRenderer(nullptr, &Renderer::UnlitShader);
  Mesh rgbLines(MeshType::Lines); {
    rgbLines.vertices = {
      Mesh::Vertex{{0,0,0}, {1,0,0, 1}},
      Mesh::Vertex{{1,0,0}, {1,0,0, 1}},

      Mesh::Vertex{{0,0,0}, {0,1,0, 1}},
      Mesh::Vertex{{0,1,0}, {0,1,0, 1}},

      Mesh::Vertex{{0,0,0}, {0,0,1, 1}},
      Mesh::Vertex{{0,0,1}, {0,0,1, 1}},
    };
    rgbLines.indices = {0,1, 2,3, 4,5};
    
    linesRenderer.setMesh(&rgbLines);
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

      animationT += System::deltaTime;
    }

    { //Render
      Renderer::clear();

      Renderer::SetShader(Renderer::UnlitShader);

      Editor::draw();

      // primitiveRenderer.draw(glm::mat4x4(1));

      const int16_t GRID_SIZE = 10;
      const float MID_HEIGHT = 10;
      for(int x = -GRID_SIZE; x < GRID_SIZE; x++) {
        for(int z = -GRID_SIZE; z <= GRID_SIZE; z++) {
          const float a = glm::cos(static_cast<float>(x));
          const float b = glm::cos(static_cast<float>(z));
          const float c = glm::cos(animationT);
          const float maxAngle = 180;
          const float startAngle = 90;
          const float angle = glm::radians((startAngle + (c * a * b) * maxAngle) * c);

          primitiveRenderer.draw(Transform(glm::vec3(x,0,z), glm::rotate(glm::mat4x4(1), angle, glm::vec3(1,0,0)), glm::vec3(1)).getMatrix());
        }
      }

      // linesRenderer.draw(glm::mat4x4(1));

      Renderer::display();
    }
  }

  Renderer::end();

  return 0;
}