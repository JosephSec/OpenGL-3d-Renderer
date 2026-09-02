#include <Engine/Renderer/Mesh.hpp>

#include <cstring>


Mesh::Mesh(MeshType _type) : type(_type) {}
Mesh::~Mesh() {
  if(texture != nullptr) delete texture;
}