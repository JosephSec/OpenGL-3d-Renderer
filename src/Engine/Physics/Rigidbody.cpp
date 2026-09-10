#include <Engine/Physics/Rigidbody.hpp>

#include <Engine/Renderer.hpp>
#include <Engine/Renderer/MeshRenderer.hpp>

#include <Engine/SceneManager.hpp>
#include <Engine/SceneManager/PrimitiveMeshGenerator.hpp>

#include <Engine/Editor.hpp>


glm::vec3 Rigidbody::gravityDirection = glm::vec3(0,-1,0);
float Rigidbody::gravityStrength = 0;


Rigidbody::Rigidbody(
  const Transform &_transform,
  float _mass,
  const glm::vec3 _initialVelocity,
  float _radius
)
: transform(_transform), velocity(_initialVelocity), mass(_mass), radius(_radius) {}

void Rigidbody::update() {
  velocity += glm::normalize(gravityDirection) * gravityStrength * Editor::deltaTime;
  transform.position += velocity * Editor::deltaTime;

  for(const Rigidbody &rigidbody : SceneManager::scene.rigidbodys) {
    if(&rigidbody == this) continue;

    const float distance = glm::length(transform.position - rigidbody.transform.position);
    if(distance > (radius + rigidbody.radius)) continue;

    const glm::vec3 a = velocity * (mass - rigidbody.mass) / (mass + rigidbody.mass);
    const glm::vec3 b = rigidbody.velocity * (2 * rigidbody.mass) / (mass + rigidbody.mass);
    velocity = a + b;
  }
}
void Rigidbody::draw() const {
  Mesh mesh = MeshGenerator::UVSphere(16,16, radius);

  MeshRenderer meshRenderer(&mesh, &Renderer::UnlitShader);
  meshRenderer.draw(transform.getMatrix());
}
void Rigidbody::drawGizmos() const {
  Mesh mesh;
  mesh.vertices = {
    Mesh::Vertex{{0,0,0}, {0,1,0,1}},
    Mesh::Vertex{velocity, {1,0,0,1}},
  };
  mesh.indices = {0,1};
  mesh.type = MeshType::Lines;

  MeshRenderer meshRenderer(&mesh, &Renderer::UnlitShader);
  meshRenderer.draw(Transform(transform.position).getMatrix());
}