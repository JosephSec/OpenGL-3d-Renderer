#include <Renderer/Light.hpp>
#include <System.hpp>
#include <Renderer.hpp>


Light::Light(const Vec3& _pos, const Color& _clr, float _range) : GameObject(Transform(_pos, Quaternion(), Vec3one*.25f)), color(_clr), range(_range) {
  mesh = new Mesh(Mesh::Circle);
}

void Light::update() {
  transform.rotation = Renderer::camera.rotation;
}
static float model[16];
void Light::draw() const {
  transform.getMatrix(model);
  Renderer::UnlitTriangleShader.SetUniform("model", glm::make_mat4(model));
  mesh->colors = std::vector<Color>(mesh->colors.size(), color);
  mesh->draw();
}