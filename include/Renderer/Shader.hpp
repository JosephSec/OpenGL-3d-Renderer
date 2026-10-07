#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Graphics/Color.hpp>
#include <GL/glew.h>


namespace Renderer {
  class Shader {
  public:
    static inline void SetShaderFolder(const std::filesystem::path &_path) {
      s_shaderFolder = _path;
    }


    Shader() {}
    Shader(const std::string &_name);


    void SetUniform(const std::string &_uniform, const glm::mat3 &_matrix) const;
    void SetUniform(const std::string &_uniform, const glm::mat4 &_matrix) const;
    void SetUniform(const std::string &_uniform, bool _val) const;
    void SetUniform(const std::string &_uniform, unsigned int _val) const;
    void SetUniform(const std::string &_uniform, int _val) const;
    void SetUniform(const std::string &_uniform, float _val) const;
    void SetUniform(const std::string &_uniform, const glm::vec2 _vec) const;
    void SetUniform(const std::string &_uniform, const glm::vec3 _vec) const;
    void SetUniform(const std::string &_uniform, const glm::vec4 _vec) const;
    void SetUniform(const std::string &_uniform, const sf::Color _clr) const;

    
    GLuint program;
    

  private:
    static std::filesystem::path s_shaderFolder;

    static std::vector<GLuint> GetShaders(const std::string &_name);
    static GLuint CompileShader(GLenum _type, const char *_src);

    static GLint tryGetUniformLocation(GLuint _program, const std::string &_name);

    static bool ValidateShaderProgram(GLuint _program);
  };
}