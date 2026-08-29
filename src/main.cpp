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

  MeshRenderer quadRenderer(nullptr, &Renderer::UnlitShader);
  quadRenderer.backFaceCulling = false;
  Mesh rgbaQuad; {
    rgbaQuad.vertices = {
      Mesh::Vertex{{-.5,-.5,0}, {1,0,0, .5}},
      Mesh::Vertex{{-.5, .5,0}, {0,1,0, .5}},
      Mesh::Vertex{{ .5, .5,0}, {0,0,1, .5}},
      Mesh::Vertex{{ .5,-.5,0}, {1,1,1, .5}},
    };
    rgbaQuad.indices = {0,1,2, 0,2,3};
    
    quadRenderer.setMesh(&rgbaQuad);
  }

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

  //MAKE MESH PRIMITIVES
  //MAKE MESH PRIMITIVES
  //MAKE MESH PRIMITIVES
  //MAKE MESH PRIMITIVES
  //MAKE MESH PRIMITIVES
  //MAKE MESH PRIMITIVES
  //MAKE MESH PRIMITIVES
  //MAKE MESH PRIMITIVES
  //MAKE MESH PRIMITIVES
  //MAKE MESH PRIMITIVES
  //MAKE MESH PRIMITIVES
  
  float rotation = 0;


  while(Renderer::window.isOpen()) {
    while(const auto &eventOpt = Renderer::window.pollEvent()) {
      const auto &event = *eventOpt;

      if(event.is<sf::Event::Closed>()) Renderer::window.close();
      else if(event.is<sf::Event::KeyPressed>()) User::HandleEvent(event);
      else if(const auto *resized = event.getIf<sf::Event::Resized>()) Renderer::HandleResize();
    }


    { //Update
      System::update();
      User::Mouse::update();
      Editor::update();

      rotation += System::deltaTime;
    }

    { //Render
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
      glUseProgram(Renderer::UnlitShader);

      Editor::draw();

      const int16_t GRID_SIZE = 10;
      const float MID_HEIGHT = 10;
      for(int x = -GRID_SIZE; x < GRID_SIZE; x++) {
        for(int z = -GRID_SIZE; z <= GRID_SIZE; z++) {
          const float height = (1 - glm::length(glm::vec2(x,z)) / static_cast<float>(GRID_SIZE)) * MID_HEIGHT + .1;

          quadRenderer.draw(Transform(glm::vec3(x,0,z), glm::rotate(glm::mat4x4(1), rotation, glm::vec3(0,1,0)), glm::vec3(1,height,1)).getMatrix());
        }
      }

      // linesRenderer.draw(glm::mat4x4(1));
  
      Renderer::window.display();
    }
  }

  glDeleteProgram(Renderer::UnlitShader);

  return 0;
}