#include <ParticleEffect.hpp>

#include <Renderer/Core.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include <random>
#include <chrono>


ParticleEffect::ParticleEffect(const Renderer::Transform &_transform, GLuint _texture) : transform(_transform) {
  meshRenderer.backFaceCulling = false;
  meshRenderer.material.diffuseTex = _texture;
}


void ParticleEffect::update() {
  { //Update Spawn Time
    spawnTimeLeft -= Renderer::Core::deltaTime;
    while(spawnTimeLeft <= 0) {
      spawnTimeLeft += spawnRate;

      std::mt19937 gen(std::chrono::high_resolution_clock().now().time_since_epoch().count());
      std::uniform_real_distribution<float> randPosX(0, spawnArea.x);
      std::uniform_real_distribution<float> randPosY(0, spawnArea.y);
      std::uniform_real_distribution<float> randPosZ(0, spawnArea.z);
      std::uniform_real_distribution<float> randSize(spawnMinSize, spawnMaxSize);

      particleCount += 1;
      const Particle particle = {
        lifeTime,
        randSize(gen),
        transform.position + glm::vec3(randPosX(gen), randPosY(gen), randPosZ(gen)) - spawnArea / 2.0f,
        initialVelocity
      };

      particles.push_back(particle);
    }
  } //Update Spawn Time

  if(particleCount <= 0) return;

  size_t killCount = 0;
  { //Update Particles
    const size_t backIndex = particleCount - 1;
    for(int i = backIndex; i >= 0; i--) {
      Particle &particle = particles[i];
      particle.lifeTime -= Renderer::Core::deltaTime;

      if(particle.lifeTime <= 0) {
        std::swap(particles[i], particles[backIndex - killCount]);
        killCount += 1;
        continue;
      }

      particle.velocity += gravity * Renderer::Core::deltaTime;
      particle.position += particle.velocity * Renderer::Core::deltaTime;
    }

    particleCount -= killCount;
    particles.resize(particleCount);
  } //Update Particles

  meshRenderer.updateInstancingData(getInstancingData());
}
void ParticleEffect::draw() const {
  meshRenderer.drawInstanced();
  // for(const Renderer::Transform &transform : particleTransforms) meshRenderer.draw(transform);
}

void ParticleEffect::setMesh(const Renderer::Mesh &_mesh) {
  mesh = _mesh;
  meshRenderer.setMesh(&mesh);
}
void ParticleEffect::setShader(Renderer::Shader *_shader) {
  meshRenderer.setShader(_shader);
}
void ParticleEffect::setMaterial(const Renderer::Material &_material) {
  meshRenderer.material = _material;
}

const std::vector<Renderer::Transform> ParticleEffect::getInstancingData() const noexcept {
  std::vector<Renderer::Transform> transforms(particleCount);

  const glm::vec3 cameraPos = Renderer::Core::camera != nullptr?
    Renderer::Core::camera->transform.position : glm::vec3(0);

  std::vector<size_t> sortedIndices(particleCount);
  for(size_t i = 0; i < particleCount; i++) sortedIndices[i] = i;

  std::sort(sortedIndices.begin(), sortedIndices.end(), [&](size_t _a, size_t _b) {
    float distA = glm::length2(particles[_a].position - cameraPos);
    float distB = glm::length2(particles[_b].position - cameraPos);
    return distA > distB;
  });

  for(size_t i = 0; i < particleCount; i++) {
    const Particle &particle = particles[sortedIndices[i]];

    transforms[i] = Renderer::Transform(
      particle.position,
      Renderer::Transform::LookAtRotation(particle.position, cameraPos, glm::vec3(0,1,0)),
      glm::vec3(particle.size)
    );
  }

  return transforms;
}
