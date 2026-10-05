#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <Renderer/Core/API.hpp>
#include <Renderer/Core/Shader.hpp>


namespace Renderer {
  enum LightType {
    Point = 0,
    Directional = 1,
    Spot = 2
  };
  struct HORDE3D_API Light {
  public:
    Light(LightType _type, const glm::vec3 _position = glm::vec3(0), const glm::vec3 _color = glm::vec3(1), float _brightness = 1);
    virtual ~Light() = default;

    virtual void setUniforms(Shader *_shader, unsigned int _index) const = 0;
    virtual void drawGizmos() const = 0;


    glm::vec3 position = glm::vec3(0);
    glm::vec3 color = glm::vec3(1);
    float brightness = 1;
    
  protected:
    void drawGizmoTexture(GLuint _texture) const;

    LightType type = LightType::Point;
  };
  struct HORDE3D_API PointLight : public Light {
  public:
    static GLuint GizmoTexture;


    PointLight(
      const glm::vec3 _position = glm::vec3(0),
      const glm::vec3 _color = glm::vec3(1),
      float _brightness = 1,
      float _radius = 1
    );

    void setUniforms(Shader *_shader, unsigned int _index) const override;
    void drawGizmos() const override;


    float radius = 1;
  };
  struct HORDE3D_API DirectionalLight : public Light {
  public:
    static GLuint GizmoTexture;


    DirectionalLight(
      const glm::vec3 _position = glm::vec3(0),
      const glm::vec3 _color = glm::vec3(1),
      float _brightness = 1,
      const glm::vec3 _direction = glm::vec3(-1)
    );

    void setUniforms(Shader *_shader, unsigned int _index) const override;
    void drawGizmos() const override;


    glm::vec3 direction = glm::vec3{-1,-1,-1};
  };
  struct HORDE3D_API SpotLight : public Light {
  public:
    static GLuint GizmoTexture;


    SpotLight(
      const glm::vec3 _position = glm::vec3(0),
      const glm::vec3 _color = glm::vec3(1),
      float _brightness = 1,
      float _radius = 1,
      const glm::vec3 _direction = {0,0,-1},
      float _innerCutOff = 15,
      float _outerCutOff = 45
    );

    void setUniforms(Shader *_shader, unsigned int _index) const override;
    void drawGizmos() const override;


    float radius = 1;
    glm::vec3 direction = {0,0,-1};
    float innerCutOff = 15; //degrees at which the brightest region lies
    float outerCutOff = 45; //degrees at which the darkest region lies
  };
}