#include <Engine/Renderer/Shader.hpp>
#include <System.hpp>

#include <fstream>
#include <sstream>


Shader::Shader(const std::string& _name) {
  program = glCreateProgram();
  vertex = CompileShader(GL_VERTEX_SHADER, LoadShaderFile(_name+".vert").c_str());
  fragment = CompileShader(GL_FRAGMENT_SHADER, LoadShaderFile(_name+".frag").c_str());

  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);

  glDeleteShader(vertex);
  glDeleteShader(fragment);
}


std::filesystem::path Shader::SHADER_FOLDER;

std::string Shader::LoadShaderFile(const std::string& _fileName) {
  std::ifstream file(SHADER_FOLDER/_fileName);
  if(!file.is_open()) {
    std::cout << "[ERROR]: Shader file is missing or can't be opened (" + (SHADER_FOLDER/_fileName).string() + ")\n";
    return "";
  }

  std::stringstream ss; ss << file.rdbuf();
  return ss.str();
}
GLuint Shader::CompileShader(GLenum type, const char* src) {
  GLuint program = glCreateShader(type);
  glShaderSource(program, 1, &src, nullptr);
  glCompileShader(program);
  GLint success;
  glGetShaderiv(program, GL_COMPILE_STATUS, &success);
  if(!success) {
    char log[512];
    glGetShaderInfoLog(program, 512, nullptr, log);
    std::cerr << "Shader Compile Error:\n" << log << "\n\n" << src;
  }
  return program;
}