#pragma once

#include <vector>

#include <GL/glew.h>

#include <Renderer/Component/MeshRenderer.hpp>
#include <ParticleEffect.hpp>



class TestScene {
public:
  static GLuint nullTexture;
  static GLuint particleTexture;

  static std::vector<std::pair<Renderer::Mesh, Renderer::MeshRenderer>> meshes;
  static std::vector<std::vector<Renderer::Transform>> meshInstances;

  static ParticleEffect particleEffect;


  static void init();
  static void update();
  static void draw();
};