#pragma once

#include <Engine/API.hpp>

#include <string>
#include <filesystem>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SFML/Graphics/Color.hpp>

#include <GL/glew.h>


class ZENGINE_API Shader {
public:
  static inline void SetShaderFolder(const std::filesystem::path &_path) {
    s_shaderFolder = _path;
  }


  Shader() {}
  Shader(const std::string &_name);
  Shader(const char *_src);

  inline operator GLuint() const {
    return program;
  }

  void SetUniform(const std::string &_uniform, const glm::mat3 &_matrix) const {
    glUniformMatrix3fv(tryGetUniformLocation(program, _uniform), 1, GL_FALSE, glm::value_ptr(_matrix));
  }
  void SetUniform(const std::string &_uniform, const glm::mat4 &_matrix) const {
    glUniformMatrix4fv(tryGetUniformLocation(program, _uniform), 1, GL_FALSE, glm::value_ptr(_matrix));
  }
  void SetUniform(const std::string &_uniform, bool _val) const {
    glUniform1i(tryGetUniformLocation(program, _uniform), _val);
  }
  void SetUniform(const std::string &_uniform, const int _val) const {
    glUniform1i(tryGetUniformLocation(program, _uniform), _val);
  }
  void SetUniform(const std::string &_uniform, const float _val)const {
    glUniform1f(tryGetUniformLocation(program, _uniform), _val);
  }
  void SetUniform(const std::string &_uniform, const glm::vec2 _vec) const {
    glUniform2f(tryGetUniformLocation(program, _uniform), _vec.x, _vec.y);
  }
  void SetUniform(const std::string &_uniform, const glm::vec3 _vec) const {
    glUniform3f(tryGetUniformLocation(program, _uniform), _vec.x, _vec.y, _vec.z);
  }
  void SetUniform(const std::string &_uniform, const glm::vec4 _vec) const {
    glUniform4f(tryGetUniformLocation(program, _uniform), _vec.x, _vec.y, _vec.z, _vec.w);
  }
  void SetUniform(const std::string &_uniform, const sf::Color _clr) const {
    glUniform4f(tryGetUniformLocation(program, _uniform), _clr.r/255.0f, _clr.g/255.0f, _clr.b/255.0f, _clr.a/255.0f);
  }

  GLuint program;
  

private:
  static std::filesystem::path s_shaderFolder;

  static std::string LoadShaderFile(const std::string &_fileName);
  static GLuint CompileShader(GLenum type, const char *_src);

  static GLint tryGetUniformLocation(GLuint _program, const std::string &_name);
};