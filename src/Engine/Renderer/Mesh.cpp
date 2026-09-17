#include <Engine/Renderer/Mesh.hpp>
using namespace Renderer;


Mesh::Mesh(MeshType _type) : type(_type) {}
Mesh::~Mesh() {
  if(texture != nullptr) delete texture;
}