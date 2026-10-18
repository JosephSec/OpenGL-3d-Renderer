#pragma once

#include <Renderer/Core/API.hpp>

#include <Renderer/Core/Cubemap.hpp>
#include <Renderer/Pipeline/Scene.hpp>

#include <GL/glew.h>


namespace Renderer {
  class RENDER3D_API ShadowMap {
  public:
    static constexpr int   SHADOW_SIZE = 1024;
    static constexpr float SHADOW_FAR  = 30.0f;
    static constexpr GLuint SHADOW_UNIT = 5;


    ShadowMap();

    void drawShadows(const Scene &_scene, const Shader *_shadowDepthShader);
    void loadToShader(const glm::vec3 _lightPos, const Shader *_shader);


    Cubemap m_cubemap;
    GLuint m_fbo = 0;
  };
}