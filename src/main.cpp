#include <System.hpp>
#include <User.hpp>
#include <EditorCamera.hpp>
#include <TestScene.hpp>
#include <UI/Core.hpp>

#include <Renderer/Core.hpp>

#include <SFML/Graphics.hpp>

#include <fstream>
#include <iostream>
#include <random>
#include <chrono>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>


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

  if constexpr (false) { //Generate Particle Texture
    sf::Image image(sf::Vector2u(50,50));

    for(unsigned int x = 0; x < image.getSize().x; x++) {
      for(unsigned int y = 0; y < image.getSize().y; y++) {
        const float alpha = std::max<float>(0, 1 - glm::length(glm::vec2(x - 25,y - 25)) / 25.0f);

        image.setPixel(sf::Vector2u{x,y}, sf::Color{255,255,255, static_cast<uint8_t>(alpha * 255)});
      }      
    }

    image.saveToFile(System::PATH+"/assets/textures/particle.png");
  }
  if constexpr (false) { //Generate Transparent Gizmo Tex
    const std::filesystem::path file = System::PATH+"/assets/textures/gizmos/lighting/spot.png";

    sf::Image image;
    image.loadFromFile(file);

    for(unsigned int x = 0; x < image.getSize().x; x++) {
      for(unsigned int y = 0; y < image.getSize().y; y++) {
        if(image.getPixel(sf::Vector2u{x,y}) == sf::Color{255,0,0,255}) {
          image.setPixel(sf::Vector2u{x,y}, sf::Color{0,0,0,0});
        }
      }      
    }

    image.saveToFile(file);
  }
  Renderer::PointLight::GizmoTexture = Renderer::MeshHelper::LoadTexture(System::PATH+"/assets/textures/gizmos/lighting/point.png");
  Renderer::DirectionalLight::GizmoTexture = Renderer::MeshHelper::LoadTexture(System::PATH+"/assets/textures/gizmos/lighting/directional.png");
  Renderer::SpotLight::GizmoTexture = Renderer::MeshHelper::LoadTexture(System::PATH+"/assets/textures/gizmos/lighting/spot.png");


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
        TestScene::draw();

        Renderer::Core::ClearDepthBuffer();
        
        bool prevWireFrameState = Renderer::Core::GetWireFrameMode();
        Renderer::Core::ToggleWireframeMode(false);

        if(System::ShowMeshNormals && TestScene::meshes.empty() == false) {
          Renderer::Mesh normalMesh = Renderer::MeshHelper::GenerateNormalGizmo(TestScene::meshes[0], {0,0,1,1}, -1);
          Renderer::MeshRenderer normalRenderer(&normalMesh, Renderer::Core::GetShader("Unlit"));
          normalRenderer.draw(Renderer::Transform(glm::vec3(0), glm::quat(), glm::vec3(.5f)).getMatrix());
        }
        if(System::ShowLightGizmos) {
          Renderer::Mesh textureMesh = Renderer::MeshHelper::GenerateQuad();
          Renderer::MeshRenderer textureRenderer(&textureMesh, Renderer::Core::GetShader("Unlit"));
          textureRenderer.backFaceCulling = false;

          std::vector<size_t> sortedIndices(Renderer::Core::lights.size());
          std::iota(sortedIndices.begin(), sortedIndices.end(), 0);
          std::sort(sortedIndices.begin(), sortedIndices.end(), [](size_t _a, size_t _b) {
            const float distA = glm::length2(Renderer::Core::lights[_a]->position - Renderer::Core::camera->transform.position);
            const float distB = glm::length2(Renderer::Core::lights[_b]->position - Renderer::Core::camera->transform.position);
            return distA > distB;
          });

          for(size_t i : sortedIndices) Renderer::Core::lights[i]->drawGizmos();
        }
        if(System::ShowParticleGizmos) {
          TestScene::boxParticleSystem.drawGizmos();
          TestScene::sphereParticleSystem.drawGizmos();
        }
      
        Renderer::Core::ToggleWireframeMode(prevWireFrameState);
      }

      UI::Core::draw();

      Renderer::Core::display();
    }
  }

  Renderer::Core::end();

  return 0;
}