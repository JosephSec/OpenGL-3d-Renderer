#include <Renderer/Core/ShadowMap.hpp>
using namespace Renderer;

#include <glm/gtc/matrix_transform.hpp>


ShadowMap::ShadowMap(unsigned int _resolution) : resolution(_resolution) {}
ShadowMap::~ShadowMap() {
  if(fbo != 0) glDeleteFramebuffers(1, &fbo);
  if(depthCubeMap != 0) glDeleteTextures(1, &depthCubeMap);
}

void ShadowMap::init() {
  glGenTextures(1, &depthCubeMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);

  for(int i = 0; i < 6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);

  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
std::vector<glm::mat4x4> ShadowMap::calculateLightMatrices(const Renderer::PointLight &_pointLight) {
  glm::mat4x4 shadowProjection = glm::perspective(glm::radians<float>(90), 1.0f,.1f, farPlane);

  std::vector<glm::mat4x4> shadowTransforms;

  shadowTransforms.push_back(shadowProjection * glm::lookAt(_pointLight.position, _pointLight.position + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
  shadowTransforms.push_back(shadowProjection * glm::lookAt(_pointLight.position, _pointLight.position + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
  shadowTransforms.push_back(shadowProjection * glm::lookAt(_pointLight.position, _pointLight.position + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)));
  shadowTransforms.push_back(shadowProjection * glm::lookAt(_pointLight.position, _pointLight.position + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)));
  shadowTransforms.push_back(shadowProjection * glm::lookAt(_pointLight.position, _pointLight.position + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
  shadowTransforms.push_back(shadowProjection * glm::lookAt(_pointLight.position, _pointLight.position + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));

  return shadowTransforms;
}
