#include <System.hpp>

#include <windows.h>
#include <filesystem>

#include <Renderer/Shader.hpp>


std::string System::PATH;

void System::init() {
  char buffer[MAX_PATH];
  GetModuleFileNameA(NULL, buffer, MAX_PATH);
  PATH = std::filesystem::path(buffer).parent_path().parent_path().string();

  Shader::SHADER_FOLDER = std::filesystem::path(PATH)/"assets"/"shaders";
}