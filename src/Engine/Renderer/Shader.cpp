#include <Engine/Renderer/Shader.hpp>

#include <fstream>
#include <sstream>

#include <iostream>


Shader::Shader(const std::string &_name) {
  program = glCreateProgram();
  GLuint vertex = CompileShader(GL_VERTEX_SHADER, LoadShaderFile(_name+".vert").c_str());
  GLuint fragment = CompileShader(GL_FRAGMENT_SHADER, LoadShaderFile(_name+".frag").c_str());

  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);

  glDeleteShader(vertex);
  glDeleteShader(fragment);
}


std::filesystem::path Shader::s_shaderFolder;

std::string Shader::LoadShaderFile(const std::string &_fileName) {
  std::ifstream file(s_shaderFolder/_fileName);
  if(!file.is_open()) {
    std::cout << "[SHADER ERROR]: Shader file is missing or can't be opened (" + (s_shaderFolder/_fileName).string() + ")\n";
    return "";
  }

  std::stringstream ss; ss << file.rdbuf();
  return ss.str();
}
GLuint Shader::CompileShader(GLenum type, const char *_src) {
  GLuint program = glCreateShader(type);

  glShaderSource(program, 1, &_src, nullptr);
  glCompileShader(program);

  GLint success;
  glGetShaderiv(program, GL_COMPILE_STATUS, &success);
  if(!success) {
    char log[512];
    glGetShaderInfoLog(program, 512, nullptr, log);
    std::cerr << "[SHADER ERROR]:\n" << log << "\n\n" << _src;
  }

  return program;
}