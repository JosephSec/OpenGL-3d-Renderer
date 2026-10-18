#include <Renderer/Core/ShadowMap.hpp>
using namespace Renderer;

#include <Renderer/Core.hpp>


ShadowMap::ShadowMap() : m_cubemap(SHADOW_SIZE, GL_DEPTH_COMPONENT32F, 1) {
  glTextureParameteri(m_cubemap.id(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTextureParameteri(m_cubemap.id(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTextureParameteri(m_cubemap.id(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(m_cubemap.id(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTextureParameteri(m_cubemap.id(), GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTextureParameteri(m_cubemap.id(), GL_TEXTURE_COMPARE_MODE, GL_NONE);

  glCreateFramebuffers(1, &m_fbo);
  glNamedFramebufferDrawBuffer(m_fbo, GL_NONE);
  glNamedFramebufferReadBuffer(m_fbo, GL_NONE);
}

void ShadowMap::drawShadows(const Scene &_scene, const Shader *_shadowDepthShader) {
  if(_scene.objects.empty() == true || _scene.lights.empty() == true) return;

  const glm::vec3 faceDir[6] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
  const glm::vec3 faceUp [6] = {{0,-1,0},{0,-1,0},{0,0,1},{0,0,-1},{0,-1,0},{0,-1,0}};


  Renderer::StateCache::PushStates();

  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glViewport(0, 0, Renderer::ShadowMap::SHADOW_SIZE, Renderer::ShadowMap::SHADOW_SIZE);
  glDisable(GL_SCISSOR_TEST);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glDepthMask(GL_TRUE);
  Renderer::StateCache::Invalidate();      // raw GL above bypassed the cache


  for(int i = 0; i < 1; i++) {
  // for(int i = 0; i < _scene.lights().size(); i++) {
    const Light *light = _scene.lights[i];
    const glm::mat4x4 lightProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.05f, SHADOW_FAR);


    _shadowDepthShader->use();
    _shadowDepthShader->setUniform("uLightPos", light->position);
    _shadowDepthShader->setUniform("uShadowFar", SHADOW_FAR);

    for(int f = 0; f < 6; f++) {
      glNamedFramebufferTextureLayer(m_fbo, GL_DEPTH_ATTACHMENT, m_cubemap.id(), 0, f);
      glClear(GL_DEPTH_BUFFER_BIT);

      _shadowDepthShader->setUniform("uLightSpace", lightProj * glm::lookAt(light->position, light->position + faceDir[f], faceUp[f]));
      
      for(const SceneObject &object : _scene.objects) {
        if(object.meshRenderer.castShadows == false) continue;

        object.meshRenderer.getMesh()->use();
        _shadowDepthShader->setUniform("uModel", object.transform.getModelMatrix());
        object.meshRenderer.getMesh()->draw();
      }
    }


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, Renderer::Core::windowSize.x, Renderer::Core::windowSize.y);
    Renderer::StateCache::Invalidate();
    Renderer::StateCache::PullStates();
  }
}
void ShadowMap::loadToShader(const glm::vec3 _lightPos, const Shader *_shader) {
  m_cubemap.bind(SHADOW_UNIT);
  _shader->use();
  _shader->setUniform("uShadowCube", (int)SHADOW_UNIT);
  _shader->setUniform("uShadowLightPos", _lightPos);
  _shader->setUniform("uShadowFar", SHADOW_FAR);
  _shader->setUniform("uHasShadow", true);
}