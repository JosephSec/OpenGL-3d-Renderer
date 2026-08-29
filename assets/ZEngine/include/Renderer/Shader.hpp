#pragma once

#include <string>
#include <filesystem>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SFML/Graphics/Color.hpp>

#include <GL/glew.h>

#include <iostream>


class Shader {
public:
  Shader() {}
  Shader(const std::string& name);

  inline operator GLuint() const {
    return program;
  }

  inline void SetUniform(const std::string& _uniform, const glm::mat3& matrix) const {
    glUniformMatrix3fv(glGetUniformLocation(program, _uniform.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
  }
  inline void SetUniform(const std::string& _uniform, const glm::mat4& matrix) const {
    glUniformMatrix4fv(glGetUniformLocation(program, _uniform.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
  }
  inline void SetUniform(const std::string& _uniform, const int& val) const {
    glUniform1i(glGetUniformLocation(program, _uniform.c_str()), val);
  }
  inline void SetUniform(const std::string& _uniform, const float& val) const {
    glUniform1f(glGetUniformLocation(program, _uniform.c_str()), val);
  }
  inline void SetUniform(const std::string& _uniform, const glm::vec2& vec) const {
    glUniform2f(glGetUniformLocation(program, _uniform.c_str()), vec.x, vec.y);
  }
  inline void SetUniform(const std::string& _uniform, const glm::vec3& vec) const {
    glUniform3f(glGetUniformLocation(program, _uniform.c_str()), vec.x, vec.y, vec.z);
  }
  inline void SetUniform(const std::string& _uniform, const glm::vec4& vec) const {
    glUniform4f(glGetUniformLocation(program, _uniform.c_str()), vec.x, vec.y, vec.z, vec.w);
  }
  inline void SetUniform(const std::string& _uniform, const sf::Color& clr) const {
    glUniform4f(glGetUniformLocation(program, _uniform.c_str()), clr.r/255.0f, clr.g/255.0f, clr.b/255.0f, clr.a/255.0f);
  }

  GLuint program, vertex, fragment;
  

  static std::filesystem::path SHADER_FOLDER;

  static std::string LoadShaderFile(const std::string& _fileName);
  static GLuint CompileShader(GLenum type, const char* src);
};