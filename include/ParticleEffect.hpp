#pragma once

#include <Renderer/Component/MeshRenderer.hpp>


struct Particle {
public:
  float lifeTime = 1;
  float size = 1;
  glm::vec3 position;
  glm::vec3 velocity;
};
class ParticleEffect {
public:
  ParticleEffect(const ParticleEffect&) = delete;
  ParticleEffect& operator=(const ParticleEffect&) = delete;
  
  ParticleEffect(ParticleEffect&&) noexcept = default;
  ParticleEffect& operator=(ParticleEffect &&_rhs) noexcept {
    transform = _rhs.transform;

    spawnRate = _rhs.spawnRate;
    lifeTime  = _rhs.lifeTime;

    mesh = _rhs.mesh;
    meshRenderer = Renderer::MeshRenderer(&mesh, _rhs.meshRenderer.getShader());
    meshRenderer.backFaceCulling = _rhs.meshRenderer.backFaceCulling;
    meshRenderer.material = _rhs.meshRenderer.material;
    meshRenderer.updateInstancingData(_rhs.getInstancingData());

    particleCount = _rhs.particleCount;
    particles = _rhs.particles;

    spawnTimeLeft = _rhs.spawnTimeLeft;
    return *this;
  }

  ParticleEffect() {}
  ParticleEffect(const Renderer::Transform &_transform, GLuint _texture);


  void update();
  void draw() const;

  void setMesh(const Renderer::Mesh &_mesh);
  void setShader(Renderer::Shader *_shader);
  void setMaterial(const Renderer::Material &_material);

  const std::vector<Renderer::Transform> getInstancingData() const noexcept;
  inline size_t getParticleCount() const noexcept {
    return particleCount;
  }


  Renderer::Transform transform;

  float spawnRate = .05;
  float lifeTime = 1;
  float spawnMinSize = .1f;
  float spawnMaxSize = 1;

  glm::vec3 gravity = glm::vec3(0,-9.806,0);
  glm::vec3 initialVelocity = glm::vec3(0,5,0);
  glm::vec3 spawnArea = glm::vec3(1);

private:
  Renderer::Mesh mesh;
  Renderer::MeshRenderer meshRenderer;

  size_t particleCount = 0;
  std::vector<Particle> particles;

  float spawnTimeLeft = spawnRate;
};