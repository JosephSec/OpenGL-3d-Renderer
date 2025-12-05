#include <Renderer/Shader.hpp>
#include <System.hpp>

#include <fstream>


Shader::Shader(const string& _name) {
  shader = glCreateProgram();
  vertex = CompileShader(GL_VERTEX_SHADER, LoadShaderFile(_name+".vert").c_str());
  fragment = CompileShader(GL_FRAGMENT_SHADER, LoadShaderFile(_name+".frag").c_str());

  glAttachShader(shader, vertex);
  glAttachShader(shader, fragment);
  glLinkProgram(shader);

  glDeleteShader(vertex);
  glDeleteShader(fragment);
}


string Shader::LoadShaderFile(const string& _fileName) {
  std::ifstream file(System::PATH+"/assets/shaders/"+_fileName);
  if(!file.is_open()) {
    Debug::error("Shader file is missing or can't be opened (" + _fileName +")");
    return "";
  }

  System::assets.push_back("shaders/"+_fileName);

  std::stringstream ss; ss << file.rdbuf();
  return ss.str();
}

GLuint Shader::CompileShader(GLenum type, const char* src) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if(!success) {
    char log[512];
    glGetShaderInfoLog(shader, 512, nullptr, log);
    std::cerr << "Shader Compile Error:\n" << log << "\n\n" << src;
  }
  return shader;
}