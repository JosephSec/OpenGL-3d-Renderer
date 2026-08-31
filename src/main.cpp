#include <System.hpp>
#include <User.hpp>
#include <Engine/Renderer.hpp>
#include <Editor.hpp>

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

  const int16_t GRID_SIZE = 25;
  std::mt19937 gen(std::chrono::high_resolution_clock().now().time_since_epoch().count());
  std::uniform_real_distribution<double> rand(-GRID_SIZE, GRID_SIZE);
  std::vector<std::pair<glm::vec3, glm::vec3>> bodies;
  for(int i = 0; i < 50; i++) {
    bodies.push_back({glm::vec3(rand(gen), 0, rand(gen)), glm::vec3(0)});
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

      animationT += System::deltaTime * .1;

      for(&[position, velocity] : bodies) {
        const glm::vec3 dir = glm::vec3(0) - position;
        const float dist = glm::length(dir);
        velocity += length != 0? dir/sqrt(length)*(Gravity*(planet.mass/length)) : glm::vec3(0);

        position += velocity;
      }
    }

    { //Render
      Renderer::clear();

      Renderer::SetShader(Renderer::UnlitShader);

      Editor::draw();

      // primitiveRenderer.draw(glm::mat4x4(1));

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
              glm::angleAxis(animationT, glm::vec3(0,1,0)) *
              glm::vec3(x,0,z);
            primitiveRenderer.draw(Transform(position, glm::rotate(glm::mat4x4(1), static_cast<float>(angle), glm::vec3(1,0,0)), glm::vec3(1)).getMatrix());
          }
        }
      }

      primitiveRenderer.draw(Transform(glm::vec3(0), glm::quat(), glm::vec3(5)).getMatrix());

      // linesRenderer.draw(glm::mat4x4(1));

      Renderer::display();
    }
  }

  Renderer::end();

  return 0;
}