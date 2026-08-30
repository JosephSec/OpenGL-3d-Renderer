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

  MeshRenderer primitiveRenderer(nullptr, &Renderer::UnlitShader);
  primitiveRenderer.backFaceCulling = true;

  const std::vector<glm::vec4> colors = {
    {1,0,0, 1},
    {0,1,0, 1},
    {0,0,1, 1},
    {1,1,1, 1},
    {0,0,0, 1},
  };

  Mesh primitiveMesh; {
    // Editor::LoadMeshPrimitive(primitiveMesh, std::filesystem::path(System::PATH)/"assets"/"meshes"/"cube.mesh");

    primitiveMesh.vertices = {
      Mesh::Vertex{{-.5,-.5, .5}, {1,1,1, 1}},
      Mesh::Vertex{{-.5, .5, .5}, {1,1,1, 1}},
      Mesh::Vertex{{ .5, .5, .5}, {1,1,1, 1}},
      Mesh::Vertex{{ .5,-.5, .5}, {1,1,1, 1}},
      
      Mesh::Vertex{{-.5,-.5,-.5}, {1,1,1, 1}},
      Mesh::Vertex{{-.5, .5,-.5}, {1,1,1, 1}},
      Mesh::Vertex{{ .5, .5,-.5}, {1,1,1, 1}},
      Mesh::Vertex{{ .5,-.5,-.5}, {1,1,1, 1}},
    };
    primitiveMesh.indices = {
      0,1,2, 0,2,3, //+z
      7,6,5, 7,5,4, //-z

      2,6,7, 2,7,3, //+x
      0,4,5, 0,5,1, //-x

      1,5,6, 1,6,2, //+y
      0,3,7, 0,7,4, //-y
    };

    for(int i = 0; i < primitiveMesh.vertices.size(); i++) {
      primitiveMesh.vertices[i].color = colors[i % colors.size()];
    }
    
    primitiveRenderer.setMesh(&primitiveMesh);
  }
  Editor::SaveMeshPrimitive(primitiveMesh, std::filesystem::path(System::PATH)/"assets"/"meshes"/"cube.mesh");

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

      primitiveRenderer.draw(Transform().getMatrix());

      // const int16_t GRID_SIZE = 10;
      // const float MID_HEIGHT = 10;
      // for(int x = -GRID_SIZE; x < GRID_SIZE; x++) {
      //   for(int z = -GRID_SIZE; z <= GRID_SIZE; z++) {
      //     // const float height = (1 - glm::length(glm::vec2(x,z)) / static_cast<float>(GRID_SIZE)) * MID_HEIGHT + .1;
      //     const float height = 1;
      //     const float a = glm::cos(static_cast<float>(x));
      //     const float b = glm::cos(static_cast<float>(z));
      //     const float c = glm::cos(animationT);
      //     const float maxAngle = 90;
      //     const float angle = glm::radians((90 + (c * a * b) * maxAngle) * c);

      //     primitiveRenderer.draw(Transform(glm::vec3(x,0,z), glm::rotate(glm::mat4x4(1), angle, glm::vec3(1,0,0)), glm::vec3(1,height,1)).getMatrix());
      //   }
      // }

      // linesRenderer.draw(glm::mat4x4(1));

      Renderer::display();
    }
  }

  Renderer::end();

  return 0;
}