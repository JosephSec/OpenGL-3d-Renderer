#include <TestScene.hpp>
#include <System.hpp>

#include <Renderer/Core.hpp>

#include <random>
#include <chrono>



GLuint TestScene::nullTexture;
GLuint TestScene::particleTexture;

std::vector<std::pair<Renderer::Mesh, Renderer::MeshRenderer>> TestScene::meshes;
std::vector<std::vector<Renderer::Transform>> TestScene::meshInstances;

std::vector<Renderer::ParticleSystem*> TestScene::particleSystems;

Renderer::ParticleSystem TestScene::boxParticleSystem;
Renderer::ParticleSystem TestScene::sphereParticleSystem;


void TestScene::init() {
  nullTexture = Renderer::MeshHelper::LoadTexture(System::PATH+"/assets/textures/null.png");
  particleTexture = Renderer::MeshHelper::LoadTexture(System::PATH+"/assets/textures/particle.png");

  Renderer::Core::ambientLight = {.1,.1,.1, .1};
  Renderer::Core::lights = {
    Renderer::Light{{0,0,0}, {1,0,0}, 15.0f, 1.0f},
    Renderer::Light{{0,0,0}, {0,1,0}, 15.0f, 1.0f},
    Renderer::Light{{0,0,0}, {0,0,1}, 15.0f, 1.0f},
    Renderer::Light{{0,0,0}, {1,1,0}, 15.0f, 1.0f},
    Renderer::Light{{0,0,0}, {1,0,1}, 15.0f, 1.0f},
    Renderer::Light{{0,0,0}, {0,1,1}, 15.0f, 1.0f},
  };
  Renderer::Core::UpdateDynamicLighting();


  if(true) { //Load Camera
    meshes.push_back({Renderer::Mesh(), Renderer::MeshRenderer(nullptr, Renderer::Core::GetShader("Lit"))});
    Renderer::MeshHelper::LoadMeshObj(meshes.back().first, std::filesystem::path(System::PATH)/"assets/meshes/camera.obj");
    meshes.back().second.setMesh(&meshes.back().first);
  }
  if(false) { //Load Dragon
    meshes.push_back({Renderer::Mesh(), Renderer::MeshRenderer(nullptr, Renderer::Core::GetShader("Lit"))});
    Renderer::MeshHelper::LoadMeshObj(meshes.back().first, std::filesystem::path(System::PATH)/"assets/meshes/dragon.obj");
    meshes.back().second.material = Renderer::Material{{1,1,1}, {0.05f,1.0f}};
    meshes.back().second.setMesh(&meshes.back().first);
  }
  if(false) { //Load UV Sphere
    meshes.push_back({Renderer::Mesh(), Renderer::MeshRenderer(nullptr, Renderer::Core::GetShader("Lit"))});
    meshes.back().first = Renderer::MeshHelper::GenerateUVSphere();
    meshes.back().second.setMesh(&meshes.back().first);
  }
  if(false) { //Load Cylinder
    meshes.push_back({Renderer::Mesh(), Renderer::MeshRenderer(nullptr, Renderer::Core::GetShader("Lit"))});
    meshes.back().first = Renderer::MeshHelper::GenerateCylinder();
    meshes.back().second.setMesh(&meshes.back().first);
  }

  if(false) { //Generate Mesh Instances
    static constexpr uint64_t SPAWN_COUNT = 30;
    static constexpr float SPAWN_RADIUS = 15;
    static constexpr float ROTATION_RANGE = 180;
    static constexpr float SCALE_RANGE_MIN = .2f;
    static constexpr float SCALE_RANGE_MAX = 1.0f;

    std::mt19937 gen(std::chrono::high_resolution_clock().now().time_since_epoch().count());
    std::uniform_real_distribution<float> randRadius(-SPAWN_RADIUS, SPAWN_RADIUS);
    std::uniform_real_distribution<float> randSpin(-ROTATION_RANGE, ROTATION_RANGE);
    std::uniform_real_distribution<float> randSize(SCALE_RANGE_MIN, SCALE_RANGE_MAX);


    const unsigned int meshCount = meshes.size();
    meshInstances = std::vector<std::vector<Renderer::Transform>>(meshCount, std::vector<Renderer::Transform>(SPAWN_COUNT));
    for(int i = 0; i < meshCount; i++) {
      for(int j = 0; j < SPAWN_COUNT; j++) {
        const glm::vec3 randomPos = {randRadius(gen),randRadius(gen),randRadius(gen)};
        const glm::quat randomRot = glm::angleAxis(glm::radians<float>(randSpin(gen)), glm::vec3(0,1,0)) * glm::angleAxis(glm::radians<float>(randSpin(gen)), glm::vec3(1,0,0));

        meshInstances[i][j] = Renderer::Transform(randomPos, randomRot, glm::vec3(randSize(gen)));
      }

      meshes[i].second.updateInstancingData(meshInstances[i]);
    }
  }


  { //Init Box Particle System
    boxParticleSystem = Renderer::ParticleSystem(Renderer::Transform(glm::vec3(-5,-4.75f,-5)), particleTexture);
    Renderer::Mesh mesh = Renderer::MeshHelper::GenerateQuad();
    boxParticleSystem.setMesh(mesh);
    boxParticleSystem.setShader(Renderer::Core::GetShader("Unlit"));
    boxParticleSystem.gravity = glm::vec3(0);

    Renderer::BoxParticleEmitter *particleEmitter = new Renderer::BoxParticleEmitter();
    particleEmitter->spawnRate = .01f;
    particleEmitter->lifeTimeMin = 0.1f;
    particleEmitter->lifeTimeMax = 1.0f;
    particleEmitter->spawnSizeMin = 0.1f;
    particleEmitter->spawnSizeMax = 0.5f;
    particleEmitter->initialVelocity = glm::vec3(0,1,0);
    particleEmitter->area = glm::vec3(3,.5f,3);

    boxParticleSystem.emitter = particleEmitter;
  } //Init Box Particle System
  { //Init Sphere Particle System
    sphereParticleSystem = Renderer::ParticleSystem(Renderer::Transform(glm::vec3(-5,0,-5)), particleTexture);
    Renderer::Mesh mesh = Renderer::MeshHelper::GenerateQuad();
    Renderer::MeshHelper::RandomizeMeshColors(mesh, {{0,1,0,1}});
    sphereParticleSystem.setMesh(mesh);
    sphereParticleSystem.setShader(Renderer::Core::GetShader("Unlit"));
    sphereParticleSystem.gravity = glm::vec3(0);

    Renderer::SphereParticleEmitter *particleEmitter = new Renderer::SphereParticleEmitter();
    particleEmitter->spawnRate = .01f;
    particleEmitter->lifeTimeMin = 0.1f;
    particleEmitter->lifeTimeMax = 1.0f;
    particleEmitter->spawnSizeMin = 0.1f;
    particleEmitter->spawnSizeMax = 0.5f;
    particleEmitter->velocityStrength = 1;
    particleEmitter->radius = 1;
    
    sphereParticleSystem.emitter = particleEmitter;
  } //Init Sphere Particle System

  particleSystems = {&boxParticleSystem, &sphereParticleSystem};
}
void TestScene::update() {
  { //Dynamic Lighting
    static constexpr float MOVE_RADIUS = 5;

    static float animationT = 0;
    animationT += System::deltaTime;
    const float c = glm::cos(animationT);
    const float s = glm::sin(animationT);

    const std::vector<glm::vec3> points = {
      glm::vec3(0, c * MOVE_RADIUS, s * MOVE_RADIUS),
      glm::vec3(c * MOVE_RADIUS, 0, s * MOVE_RADIUS),
      glm::vec3(c * MOVE_RADIUS, s * MOVE_RADIUS, 0),
      -glm::vec3(0, c * MOVE_RADIUS, s * MOVE_RADIUS),
      -glm::vec3(c * MOVE_RADIUS, 0, s * MOVE_RADIUS),
      -glm::vec3(c * MOVE_RADIUS, s * MOVE_RADIUS, 0),
    };
    for(int i = 0; i < Renderer::Core::lights.size(); i++) {
      Renderer::Core::lights[i].position = points[i];
    }

    Renderer::Core::UpdateDynamicLighting();
  } //Dynamic Lighting  

  if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) { //GPU Instancing Updates
    for(int i = 0; i < meshInstances.size(); i++) {
      for(Renderer::Transform &_instance : meshInstances[i]) {
        _instance.position -= _instance.position * System::deltaTime * .5f;
      }

      meshes[i].second.updateInstancingData(meshInstances[i]);
    }
  }

  for(Renderer::ParticleSystem *particleSystem : particleSystems) particleSystem->update();
}
void TestScene::draw() { 
  { //Ground
    Renderer::Mesh mesh = Renderer::MeshHelper::GeneratePlane(glm::vec2(20,20), glm::ivec2(10,10));
    Renderer::MeshRenderer meshRenderer = Renderer::MeshRenderer(&mesh, Renderer::Core::GetShader("Lit"));
    meshRenderer.material = Renderer::Material{{1,1,1}, {.5f,.1f}};
    meshRenderer.draw(Renderer::Transform(glm::vec3(0,-5,0)));
  } //Ground

  for(const auto &[mesh, renderer] : meshes) renderer.drawInstanced();

  if(true && meshes.empty() == false) { //Material Types
    meshes[0].second.material = Renderer::Material{{1,1,1}, {.1f,1}};
    meshes[0].second.draw(Renderer::Transform(glm::vec3(-2,0,0), glm::quat(), glm::vec3(.5f)));

    meshes[0].second.material = Renderer::Material{{1,1,1}, {.5f,.5f}};
    meshes[0].second.draw(Renderer::Transform(glm::vec3(0,0,0), glm::quat(), glm::vec3(.5f)));

    meshes[0].second.material = Renderer::Material{{1,1,1}, {1,.1f}};
    meshes[0].second.draw(Renderer::Transform(glm::vec3(2,0,0), glm::quat(), glm::vec3(.5f)));
  }

  { //Texture Displays
    Renderer::Mesh mesh = Renderer::MeshHelper::GenerateQuad(glm::vec2(3,3));
    Renderer::MeshRenderer meshRenderer = Renderer::MeshRenderer(&mesh, Renderer::Core::GetShader("Unlit"));
    meshRenderer.backFaceCulling = false;
    meshRenderer.material = Renderer::Material{{1,1,1}, {.5f,.1f}};

    meshRenderer.material.diffuseTex = nullTexture;
    meshRenderer.draw(Renderer::Transform(glm::vec3(-2,0,-10)));
    meshRenderer.material.diffuseTex = particleTexture;
    meshRenderer.draw(Renderer::Transform(glm::vec3(2,0,-10)));
  } //Texture Displays

  { //Particle Systems
    std::vector<size_t> sortedIndices(particleSystems.size());
    std::iota(sortedIndices.begin(), sortedIndices.end(), 0);
    std::sort(sortedIndices.begin(), sortedIndices.end(), [](size_t a, size_t b) {
      float distA = glm::length(Renderer::Core::camera->transform.position - particleSystems[a]->transform.position);
      float distB = glm::length(Renderer::Core::camera->transform.position - particleSystems[b]->transform.position);
      return distA > distB;
    });
  
    for(size_t i : sortedIndices) particleSystems[i]->draw();
  } //Particle Systems
}