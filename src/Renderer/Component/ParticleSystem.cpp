#include <Renderer/Component/ParticleSystem.hpp>
using namespace Renderer;

#include <Renderer/Core.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include <random>
#include <chrono>


void BoxParticleEmitter::drawGizmos(const Renderer::Transform &_transform) const {
  Renderer::Mesh boxMesh = Renderer::MeshHelper::GenerateWireCube();
  Renderer::MeshHelper::RandomizeMeshColors(boxMesh, {{0,1,0,1}});

  Renderer::MeshRenderer boxRenderer(&boxMesh, Renderer::Core::GetShader("Unlit"));
  boxRenderer.draw(Renderer::Transform(_transform.position, _transform.rotation, area).getMatrix());
}

Particle BoxParticleEmitter::getNewParticle() const {
  std::mt19937 gen(std::chrono::high_resolution_clock().now().time_since_epoch().count());
  std::uniform_real_distribution<float> randUniform(0,1);
  
  return Particle{
    ParticleEmitter::getNewLifeTime(randUniform(gen)),
    ParticleEmitter::getNewSpawnSize(randUniform(gen)),
    glm::vec3{randUniform(gen) * area.x, randUniform(gen) * area.y,randUniform(gen) * area.z} - area / 2.0f,
    initialVelocity
  };
}


void SphereParticleEmitter::drawGizmos(const Renderer::Transform &_transform) const {
  Renderer::Mesh sphereMesh = Renderer::MeshHelper::GenerateWireSphere(16, radius);
  Renderer::MeshHelper::RandomizeMeshColors(sphereMesh, {{0,1,0,1}});

  Renderer::MeshRenderer sphereRenderer(&sphereMesh, Renderer::Core::GetShader("Unlit"));
  sphereRenderer.draw(Renderer::Transform(_transform.position, _transform.rotation, glm::vec3(1)).getMatrix());
}

Particle SphereParticleEmitter::getNewParticle() const {
  std::mt19937 gen(std::chrono::high_resolution_clock().now().time_since_epoch().count());
  std::uniform_real_distribution<float> randUniform(0,1);
  
  const glm::vec3 position = glm::normalize(glm::vec3{
    glm::cos(randUniform(gen) * glm::two_pi<float>()),
    glm::sin(randUniform(gen) * glm::two_pi<float>()),
    glm::sin(randUniform(gen) * glm::two_pi<float>())
  }) * glm::pow(randUniform(gen), 1.0f / 3.0f) * radius;

  return Particle{
    ParticleEmitter::getNewLifeTime(randUniform(gen)),
    ParticleEmitter::getNewSpawnSize(randUniform(gen)),
    position,
    glm::normalize(position) * velocityStrength
  };
}



ParticleSystem::ParticleSystem(const Renderer::Transform &_transform, GLuint _texture) : transform(_transform) {
  m_meshRenderer.backFaceCulling = false;
  m_meshRenderer.material.diffuseTex = _texture;
}

void ParticleSystem::update() {
  { //Update Spawn Time
    emitter->spawnTimeLeft -= Renderer::Core::deltaTime;
    while(emitter->spawnTimeLeft <= 0) {
      emitter->spawnTimeLeft += emitter->spawnRate;

      std::mt19937 gen(std::chrono::high_resolution_clock().now().time_since_epoch().count());
      std::uniform_real_distribution<float> randUniform(0, 1);

      m_particleCount += 1;
      Particle particle = emitter->getNewParticle();
      particle.position += transform.position;
      m_particles.push_back(particle);
    }
  } //Update Spawn Time

  if(m_particleCount <= 0) return;

  size_t killCount = 0;
  { //Update Particles
    const size_t backIndex = m_particleCount - 1;
    for(int i = backIndex; i >= 0; i--) {
      Particle &particle = m_particles[i];
      particle.lifeTime -= Renderer::Core::deltaTime;

      if(particle.lifeTime <= 0) {
        std::swap(m_particles[i], m_particles[backIndex - killCount]);
        killCount += 1;
        continue;
      }

      particle.velocity += gravity * Renderer::Core::deltaTime;
      particle.position += particle.velocity * Renderer::Core::deltaTime;
    }

    m_particleCount -= killCount;
    m_particles.resize(m_particleCount);
  } //Update Particles

  m_meshRenderer.updateInstancingData(getInstancingData());
}
void ParticleSystem::draw() const {
  m_meshRenderer.drawInstanced();
}
void ParticleSystem::drawGizmos() const {
  if(emitter != nullptr) emitter->drawGizmos(transform);
}

void ParticleSystem::reset() {
  m_particleCount = 0;
  m_particles.resize(0);
  m_meshRenderer.clearInstancingData();
}


const std::vector<Renderer::Transform> ParticleSystem::getInstancingData() const noexcept {
  std::vector<Renderer::Transform> transforms(m_particleCount);

  const glm::vec3 cameraPos = Renderer::Core::camera != nullptr?
    Renderer::Core::camera->transform.position : glm::vec3(0);

  std::vector<size_t> sortedIndices(m_particleCount);
  for(size_t i = 0; i < m_particleCount; i++) sortedIndices[i] = i;

  std::sort(sortedIndices.begin(), sortedIndices.end(), [&](size_t _a, size_t _b) {
    float distA = glm::length2(m_particles[_a].position - cameraPos);
    float distB = glm::length2(m_particles[_b].position - cameraPos);
    return distA > distB;
  });

  for(size_t i = 0; i < m_particleCount; i++) {
    const Particle &particle = m_particles[sortedIndices[i]];

    transforms[i] = Renderer::Transform(
      particle.position,
      Renderer::Transform::LookAtRotation(particle.position, cameraPos, glm::vec3(0,1,0)),
      glm::vec3(particle.size)
    );
  }

  return transforms;
}
