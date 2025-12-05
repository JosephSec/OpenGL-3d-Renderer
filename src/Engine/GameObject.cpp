#include <Engine/GameObject.hpp>
#include <User.hpp>
#include <Renderer.hpp>

#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>


GameObject::GameObject(Transform _transform, Mesh* _mesh) : transform(_transform), mesh(_mesh) {}

void GameObject::update() {
  if(mesh) mesh->update();
}

static float model[16];
void GameObject::draw() const {
  if(!mesh) return;

  transform.getMatrix(model);
  glm::mat4 modelMat = glm::make_mat4(model);
  Renderer::LitTriangleShader.SetUniform("model", modelMat);
  
  glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));
  Renderer::LitTriangleShader.SetUniform("normal", normalMat);

  mesh->draw();
}