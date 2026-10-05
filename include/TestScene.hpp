#pragma once

#include <vector>
#include <functional>

#include <GL/glew.h>

#include <Renderer/Component/MeshRenderer.hpp>
#include <Renderer/Component/ParticleSystem.hpp>
#include <Renderer/Core/Shader.hpp>
#include <Renderer/Core/ShadowMap.hpp>


class TestScene {
public:
  static GLuint nullTexture;
  static GLuint particleTexture;

  static std::vector<Renderer::Mesh> meshes;
  static std::vector<Renderer::MeshRenderer> meshRenderers;
  static std::vector<std::vector<Renderer::Transform>> meshInstances;

  static std::vector<Renderer::ParticleSystem*> particleSystems;

  static Renderer::ParticleSystem boxParticleSystem;
  static Renderer::ParticleSystem sphereParticleSystem;


  static void init();
  static void update();
  static void draw();
};
