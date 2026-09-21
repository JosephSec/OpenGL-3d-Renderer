#include <Renderer/Shader.hpp>
using namespace Renderer;

#include <fstream>
#include <sstream>
#include <iostream>


//private
std::filesystem::path Shader::s_shaderFolder;
//private


Shader::Shader(const std::string &_name) {
  program = glCreateProgram();
  const std::vector<GLuint> shaders = GetShaders(_name);

  for(const GLuint shader : shaders) glAttachShader(program, shader);
  glLinkProgram(program);

  for(const GLuint shader : shaders) glDeleteShader(shader);
}


void Shader::SetUniform(const std::string &_uniform, const glm::mat3 &_matrix) const {
  glUniformMatrix3fv(tryGetUniformLocation(program, _uniform), 1, GL_FALSE, glm::value_ptr(_matrix));
}
void Shader::SetUniform(const std::string &_uniform, const glm::mat4 &_matrix) const {
  glUniformMatrix4fv(tryGetUniformLocation(program, _uniform), 1, GL_FALSE, glm::value_ptr(_matrix));
}
void Shader::SetUniform(const std::string &_uniform, bool _val) const {
  glUniform1i(tryGetUniformLocation(program, _uniform), _val);
}
void Shader::SetUniform(const std::string &_uniform, const int _val) const {
  glUniform1i(tryGetUniformLocation(program, _uniform), _val);
}
void Shader::SetUniform(const std::string &_uniform, const float _val)const {
  glUniform1f(tryGetUniformLocation(program, _uniform), _val);
}
void Shader::SetUniform(const std::string &_uniform, const glm::vec2 _vec) const {
  glUniform2f(tryGetUniformLocation(program, _uniform), _vec.x, _vec.y);
}
void Shader::SetUniform(const std::string &_uniform, const glm::vec3 _vec) const {
  glUniform3f(tryGetUniformLocation(program, _uniform), _vec.x, _vec.y, _vec.z);
}
void Shader::SetUniform(const std::string &_uniform, const glm::vec4 _vec) const {
  glUniform4f(tryGetUniformLocation(program, _uniform), _vec.x, _vec.y, _vec.z, _vec.w);
}
void Shader::SetUniform(const std::string &_uniform, const sf::Color _clr) const {
  glUniform4f(tryGetUniformLocation(program, _uniform), _clr.r/255.0f, _clr.g/255.0f, _clr.b/255.0f, _clr.a/255.0f);
}


std::vector<GLuint> Shader::GetShaders(const std::string &_name) {
  std::vector<GLuint> shaders;
  shaders.reserve(3);

  const std::vector<std::pair<GLuint, std::filesystem::path>> shaderTypes = {
    {GL_VERTEX_SHADER,   s_shaderFolder/_name/(_name + ".vert")},
    {GL_FRAGMENT_SHADER, s_shaderFolder/_name/(_name + ".frag")},
    {GL_COMPUTE_SHADER,  s_shaderFolder/_name/(_name + ".comp")},
  };

  for(const auto &[type, filePath] : shaderTypes) {
    if(std::filesystem::exists(filePath) == false) continue;

    std::ifstream file(filePath);
    if(file.is_open() == false) {
      std::cout << "[SHADER ERROR]: Shader file is missing or can't be opened (" + filePath.string() + ")\n";
      continue;
    }

    std::stringstream ss;
    ss << file.rdbuf();
    const std::string str = ss.str();
    shaders.push_back(CompileShader(type, str.c_str()));
  }

  return shaders;
}
GLuint Shader::CompileShader(GLenum _type, const char *_src) {
  GLuint program = glCreateShader(_type);

  glShaderSource(program, 1, &_src, nullptr);
  glCompileShader(program);

  GLint success;
  glGetShaderiv(program, GL_COMPILE_STATUS, &success);
  if(!success) {
    char log[512];
    glGetShaderInfoLog(program, 512, nullptr, log);

    glDeleteShader(program);

    std::cout << "[SHADER ERROR]: Failed to compile shader.\nLog: " + std::string(log);
    throw std::runtime_error("Failed to compile shader.\nLog: " + std::string(log));
  }

  return program;
}

GLint Shader::tryGetUniformLocation(GLuint _program, const std::string &_name) {
  GLint location = glGetUniformLocation(_program, _name.c_str());
  
  if(location == -1) {
    std::cout << "[SHADER ERROR]: Attempted to find shader uniform that does not exists (" << _name << ")\n";
  }

  return location;
}