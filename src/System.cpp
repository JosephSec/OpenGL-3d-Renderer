#include <System.hpp>

#include <windows.h>
#include <filesystem>

#include <SFML/System/Time.hpp>

#include <Engine/Renderer/Shader.hpp>


std::string System::PATH;

sf::Clock System::timeClock;
float System::deltaTime;


void System::init() {
  char buffer[MAX_PATH];
  GetModuleFileNameA(NULL, buffer, MAX_PATH);
  PATH = std::filesystem::path(buffer).parent_path().parent_path().string();

  Shader::SHADER_FOLDER = std::filesystem::path(PATH)/"assets"/"shaders";
}
void System::update() {
  deltaTime = timeClock.restart().asSeconds();
}