#include <System.hpp>
#include <User.hpp>
#include <Renderer.hpp>

#include <UI.hpp>

#include <filesystem>
#include <windows.h>

#include <Engine/Gizmos.hpp>
#include <Engine/TransformGizmo.hpp>


string System::PATH;

sf::Clock System::timeClock;
float System::deltaTime;

std::vector<string> System::assets;

Raycast System::rayCastHit;


void System::init() {
  char buffer[MAX_PATH]; GetModuleFileNameA(NULL, buffer, MAX_PATH);
  PATH = std::filesystem::path(buffer).parent_path().parent_path().string();

  Renderer::init();

  Gizmos::init();

  UI::Manager::init(PATH, Renderer::window, true);


  GenerateReleaseAssets();
}
void System::update() {
  deltaTime = timeClock.restart().asMilliseconds() * .001f;

  User::update();

  Gizmos::gizmos.clear();
  Renderer::update();

  Gizmos::update();

  UI::Manager::update();
}
void System::end() {
  Renderer::clear();
  UI::Manager::clear();
}