#include <System.hpp>
#include <User.hpp>
#include <EditorCamera.hpp>
#include <TestScene.hpp>
#include <UI.hpp>

#include <Renderer/Core.hpp>

#include <SFML/Graphics.hpp>

#include <fstream>
#include <iostream>
#include <random>
#include <chrono>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>


static std::string BoolString(const std::string &_str, bool _val) {
  return _str + ": " + std::string(_val? "True" : "False");
}
static std::string FloatString(float _val, int _precision = 2) {
  const float scalar = std::pow(10, _precision);

  std::stringstream ss;
  ss << static_cast<int>(_val * scalar) / scalar;
  return ss.str();
}
static std::string Vec3String(const glm::vec3 _val, int _precision = 2) {
  const float scalar = std::pow(10, _precision);

  std::stringstream ss;
  ss << "(" <<
    (static_cast<int>(_val.x * scalar) / scalar) << ", " <<
    (static_cast<int>(_val.y * scalar) / scalar) << ", " <<
    (static_cast<int>(_val.z * scalar) / scalar) << ")";
  return ss.str();
}


static void init() {
  System::init();

  Renderer::Core::init({800 + 225 * 2, 600}, "3D Rendering Library");
  Renderer::Core::window->setVerticalSyncEnabled(true);
  Renderer::Core::LoadShader("Unlit");
  Renderer::Core::LoadShader("Lit");
  

  User::init();
  EditorCamera::init();
  TestScene::init();
  UI::Core::init(Renderer::Core::window, "Roboto.ttf");
}
int main(int argc, char *argv[]) {
  init();

  { //Generate Particle Texture
    sf::Image image(sf::Vector2u(50,50));

    for(int x = 0; x < image.getSize().x; x++) {
      for(int y = 0; y < image.getSize().y; y++) {
        const float alpha = std::max<float>(0, 1 - glm::length(glm::vec2(x - 25,y - 25)) / 25.0f);

        image.setPixel(sf::Vector2u{x,y}, sf::Color{255,255,255, alpha * 255});
      }      
    }

    image.saveToFile(System::PATH+"/assets/textures/particle.png");
  }


  while(Renderer::Core::window->isOpen()) {
    while(const auto &eventOpt = Renderer::Core::window->pollEvent()) {
      const auto &event = *eventOpt;

      if(event.is<sf::Event::Closed>()) Renderer::Core::window->close();
      else if(const auto *resized = event.getIf<sf::Event::Resized>()) Renderer::Core::UpdateWindowSize();
      else if(event.is<sf::Event::FocusLost>()) User::HandleFocusLost();

      else if(const auto *keyPressed = event.getIf<sf::Event::KeyPressed>()) User::HandleKeyPressed(keyPressed);
      else if(const auto *mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) User::HandleMouseButtonPressed(mouseButtonPressed);
      else if(const auto *mouseButtonReleased = event.getIf<sf::Event::MouseButtonReleased>()) User::HandleMouseButtonReleased(mouseButtonReleased);
    }


    { //Update
      System::update();
      User::Cursor::update();
      EditorCamera::update();
      TestScene::update();
    }

    { //Render
      Renderer::Core::clear();

      Renderer::Core::SetState(Renderer::State::OpenGL); {
        if(System::ShowMeshNormals && TestScene::meshes.empty() == false) {
          Renderer::Mesh normalMesh = Renderer::MeshHelper::GenerateNormalGizmos(TestScene::meshes[0].first, {0,0,1,1});
          Renderer::MeshRenderer normalRenderer(&normalMesh, Renderer::Core::GetShader("Unlit"));
          normalRenderer.draw(glm::mat4x4(1));
        }
        if(System::ShowLightGizmos) {
          Renderer::Mesh lightMesh = Renderer::MeshHelper::GenerateUVSphere(8,8,.25f);
          Renderer::MeshRenderer lightRenderer(&lightMesh, Renderer::Core::GetShader("Unlit"));
          lightRenderer.backFaceCulling = false;

          for(const Renderer::Light &light : Renderer::Core::lights) {
            Renderer::MeshHelper::RandomizeMeshColors(lightMesh, {glm::vec4(light.color, 1)});
            lightRenderer.updateMeshData();
            lightRenderer.draw(Renderer::Transform(light.position).getMatrix());
          }
        }
        if(System::ShowParticleGizmos) {
          { //Particle Transforms
            Renderer::Mesh mesh(Renderer::MeshType::Lines);
            mesh.vertices = {
              Renderer::Mesh::Vertex{{0,0,0}, {1,0,0,1}},
              Renderer::Mesh::Vertex{{0,0,0}, {1,0,0,1}},

              Renderer::Mesh::Vertex{{0,0,0}, {0,1,0,1}},
              Renderer::Mesh::Vertex{{0,0,0}, {0,1,0,1}},

              Renderer::Mesh::Vertex{{0,0,0}, {0,0,1,1}},
              Renderer::Mesh::Vertex{{0,0,0}, {0,0,1,1}},
            };
            mesh.indices = {0,1,2,3,4,5};
            Renderer::MeshRenderer transformRenderer(&mesh, Renderer::Core::GetShader("Unlit"));

            for(const Renderer::Transform &transform : TestScene::particleSystem.getInstancingData()) {
            mesh.vertices[1] = Renderer::Mesh::Vertex{transform.right(),   {1,0,0,1}};
            mesh.vertices[3] = Renderer::Mesh::Vertex{transform.up(),      {0,1,0,1}};
            mesh.vertices[5] = Renderer::Mesh::Vertex{transform.forward(), {0,0,1,1}};

            transformRenderer.updateMeshData();
            transformRenderer.draw(Renderer::Transform(transform.position, glm::quat(), glm::vec3(1)));
          }
          } //Particle Transforms
          TestScene::particleSystem.drawGizmos();
        }

        TestScene::draw();
      }

      UI::Core::draw();

      Renderer::Core::display();
    }
  }

  Renderer::Core::end();

  return 0;
}


//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS
//TRY OUT COMPUTE SHADERS