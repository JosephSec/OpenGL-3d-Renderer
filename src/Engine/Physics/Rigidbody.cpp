#include <Engine/Physics/Rigidbody.hpp>
#include <Engine/Physics.hpp>

#include <Engine/Renderer.hpp>
#include <Engine/Renderer/MeshRenderer.hpp>

#include <Engine/SceneManager.hpp>
#include <Engine/SceneManager/PrimitiveMeshGenerator.hpp>


Rigidbody::Rigidbody(
  const Transform &_transform,
  float _mass,
  const glm::vec3 _initialVelocity,
  float _radius
)
: transform(_transform), velocity(_initialVelocity), mass(_mass), radius(_radius) {}

void Rigidbody::fixedUpdate() {
  velocity += Physics::gravity * Physics::fixedDeltaTime;
  transform.position += velocity * Physics::fixedDeltaTime;
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