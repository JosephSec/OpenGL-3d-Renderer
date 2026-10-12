#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SFML/Graphics/Color.hpp>
#include <GL/glew.h>

#include <Renderer/Core/API.hpp>


namespace Renderer {
  class RENDER3D_API Shader {
  public:
    struct UniformLocation {
      GLint id = -1;
    };


    static inline void SetShaderFolder(const std::filesystem::path &_path) {
      s_shaderFolder = _path;
    }


    Shader() {}
    explicit Shader(const std::string &_name);
    ~Shader();

    void use() const noexcept;

    void setUniform(UniformLocation _loc, const glm::mat3 &_matrix) const;
    void setUniform(UniformLocation _loc, const glm::mat4 &_matrix) const;
    void setUniform(UniformLocation _loc, bool _val) const;
    void setUniform(UniformLocation _loc, unsigned int _val) const;
    void setUniform(UniformLocation _loc, int _val) const;
    void setUniform(UniformLocation _loc, float _val) const;
    void setUniform(UniformLocation _loc, const glm::vec2 _vec) const;
    void setUniform(UniformLocation _loc, const glm::vec3 _vec) const;
    void setUniform(UniformLocation _loc, const glm::vec4 _vec) const;
    void setUniform(UniformLocation _loc, const sf::Color _clr) const;

    template <typename T>
    void setUniform(const std::string &_name, const T &_val) const {
      setUniform(getUniform(_name), _val);
    }

    template <typename T>
    void setUniform(UniformLocation, T) const = delete;

    
    inline std::string getName() const noexcept {
      return m_name;
    }
    inline uint16_t getID() const noexcept {
      return m_id;
    }

    UniformLocation getUniform(const std::string &_name) const noexcept;
    

  private:
    static std::filesystem::path s_shaderFolder;
    static uint16_t s_nextID;

    static std::vector<GLuint> GetShaders(const std::string &_name);
    static GLuint CompileShader(GLenum _type, const char *_src);

    static std::unordered_map<std::string, GLint> CacheUniforms(GLuint _program);

    static void CheckLink(GLuint _program);


    const std::string m_name;
    GLuint m_program = 0;
    uint16_t m_id = 0;
    std::unordered_map<std::string, GLint> m_uniformLocations;
  };
}