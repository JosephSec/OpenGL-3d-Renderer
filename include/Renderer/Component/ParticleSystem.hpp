#pragma once

#include <random>
#include <chrono>

#include <Renderer/Component/MeshRenderer.hpp>

#include <Renderer/Core/API.hpp>


namespace Renderer {
  struct HORDE3D_API Particle {
  public:
    float lifeTime = 1;
    float size = 1;
    glm::vec3 position;
    glm::vec3 velocity;
  };
  class HORDE3D_API ParticleEmitter {
  public:
    ParticleEmitter() {}

    
    virtual void drawGizmos(const Transform &_transform) const = 0;

    virtual Particle getNewParticle() const = 0;


    float spawnRate = .01f;
    float spawnTimeLeft = spawnRate;

    float lifeTimeMin = 0.1f;
    float lifeTimeMax = 1.0f;

    float spawnSizeMin = 0.1f;
    float spawnSizeMax = 1.0f;


  protected:
    float getNewLifeTime(float _t) const noexcept {
      return lifeTimeMin + (lifeTimeMax - lifeTimeMin) * _t;
    }
    float getNewSpawnSize(float _t) const noexcept {
      return spawnSizeMin + (spawnSizeMax - spawnSizeMin) * _t;
    }
  };
  class HORDE3D_API BoxParticleEmitter : public ParticleEmitter {
  public:
    void drawGizmos(const Transform &_transform) const override;

    Particle getNewParticle() const override;

    glm::vec3 initialVelocity = glm::vec3(0,1,0);
    glm::vec3 area = glm::vec3(1);
  };
  class HORDE3D_API SphereParticleEmitter : public ParticleEmitter {
  public:
    void drawGizmos(const Transform &_transform) const override;

    Particle getNewParticle() const override;

    float velocityStrength = 1;
    float radius = 1;
  };

  class HORDE3D_API ParticleSystem {
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
      m_meshRenderer = MeshRenderer(&m_mesh, _rhs.m_meshRenderer.getShader());
      m_meshRenderer.backFaceCulling = _rhs.m_meshRenderer.backFaceCulling;
      m_meshRenderer.material = _rhs.m_meshRenderer.material;
      m_meshRenderer.updateInstancingData(_rhs.getInstancingData());

      return *this;
    }

    ParticleSystem() {}
    ParticleSystem(const Transform &_transform, GLuint _texture);


    void update();
    void draw() const;
    void drawGizmos() const;

    void reset();


    inline void setMesh(const Mesh &_mesh) {
      m_mesh = _mesh;
      m_meshRenderer.setMesh(&m_mesh);
    }
    inline void setShader(Shader *_shader) {
      m_meshRenderer.setShader(_shader);
    }
    inline void setMaterial(const Material &_material) {
      m_meshRenderer.material = _material;
    }


    inline size_t getParticleCount() const noexcept {
      return m_particleCount;
    }

    const std::vector<Transform> getInstancingData() const noexcept;


    Transform transform;
    glm::vec3 gravity = glm::vec3(0,-9.806f,0);
    ParticleEmitter *emitter;

  private:
    size_t m_particleCount = 0;
    std::vector<Particle> m_particles;

    Mesh m_mesh;
    MeshRenderer m_meshRenderer;
  };
}