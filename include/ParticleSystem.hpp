#pragma once

#include <Renderer/Component/MeshRenderer.hpp>

#include <random>
#include <chrono>


struct Particle {
public:
  float lifeTime = 1;
  float size = 1;
  glm::vec3 position;
  glm::vec3 velocity;
};
class ParticleEmitter {
public:
  ParticleEmitter() : m_randSeed(std::chrono::high_resolution_clock().now().time_since_epoch().count()), m_randGen(0, 1) {}

  
  virtual void drawGizmos(const Renderer::Transform &_transform) const = 0;

  virtual glm::vec3 getNewParticlePosition() const = 0;
  virtual glm::vec3 getNewParticleVelocity() const = 0;

  float getNewParticleLifeTime() const;
  float getNewParticleSize() const;


  float spawnRate = .01f;
  float spawnTimeLeft = spawnRate;

  float lifeTimeMin = 0.1f;
  float lifeTimeMax = 1.0f;

  float spawnSizeMin = 0.1f;
  float spawnSizeMax = 1.0f;


private:
  mutable std::mt19937 m_randSeed;
  mutable std::uniform_real_distribution<float> m_randGen;
};
class BoxParticleEmitter : public ParticleEmitter {
public:
  void drawGizmos(const Renderer::Transform &_transform) const override;

  glm::vec3 getNewParticlePosition() const override;
  glm::vec3 getNewParticleVelocity() const override;

  glm::vec3 initialVelocity = glm::vec3(0,1,0);
  glm::vec3 area = glm::vec3(1);
};

class ParticleSystem {
public:
  ParticleSystem(const ParticleSystem&) = delete;
  ParticleSystem& operator=(const ParticleSystem&) = delete;
  
  ParticleSystem(ParticleSystem&&) noexcept = default;
  ParticleSystem& operator=(ParticleSystem &&_rhs) noexcept {
    transform = _rhs.transform;

    gravity = _rhs.gravity;

    m_particleCount = _rhs.m_particleCount;
    m_particles = _rhs.m_particles;

    if(emitter != nullptr) delete emitter;
    emitter = _rhs.emitter;

    m_mesh = _rhs.m_mesh;
    m_meshRenderer = Renderer::MeshRenderer(&m_mesh, _rhs.m_meshRenderer.getShader());
    m_meshRenderer.backFaceCulling = _rhs.m_meshRenderer.backFaceCulling;
    m_meshRenderer.material = _rhs.m_meshRenderer.material;
    m_meshRenderer.updateInstancingData(_rhs.getInstancingData());

    return *this;
  }

  ParticleSystem() {}
  ParticleSystem(const Renderer::Transform &_transform, GLuint _texture);


  void update();
  void draw() const;
  void drawGizmos() const;


  inline void setMesh(const Renderer::Mesh &_mesh) {
    m_mesh = _mesh;
    m_meshRenderer.setMesh(&m_mesh);
  }
  inline void setShader(Renderer::Shader *_shader) {
    m_meshRenderer.setShader(_shader);
  }
  inline void setMaterial(const Renderer::Material &_material) {
    m_meshRenderer.material = _material;
  }


  inline size_t getParticleCount() const noexcept {
    return m_particleCount;
  }

  const std::vector<Renderer::Transform> getInstancingData() const noexcept;


  Renderer::Transform transform;
  glm::vec3 gravity = glm::vec3(0,-9.806f,0);
  ParticleEmitter *emitter;

private:
  size_t m_particleCount = 0;
  std::vector<Particle> m_particles;

  Renderer::Mesh m_mesh;
  Renderer::MeshRenderer m_meshRenderer;
};