#pragma once

#include <vector>
#include <glm/mat4x4.hpp>

#include <GL/glew.h>
#include <Renderer/Core/API.hpp>

#include <Renderer/Component/Light.hpp>


namespace Renderer {
  class HORDE3D_API ShadowMap {
  public:
    ShadowMap(unsigned int _resolution = 1024);
    ~ShadowMap();

    void init();
    std::vector<glm::mat4x4> calculateLightMatrices(const Renderer::PointLight &_pointLight);


    GLuint fbo;
    GLuint depthCubeMap;
    unsigned int resolution = 1024;
    float farPlane = 25;
  };
}
