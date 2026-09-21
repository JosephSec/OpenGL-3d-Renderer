#include <Renderer/MeshHelper.hpp>
using namespace Renderer;

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>

#include <fstream>
#include <iostream>


bool MeshHelper::SaveMesh(const Mesh &_mesh, const std::filesystem::path &_path) {
  if(std::filesystem::exists(_path.parent_path()) == false) {
    std::filesystem::create_directories(_path.parent_path());
  }

  std::ofstream file(_path, std::ios::binary);
  if(!file.is_open()) {
    std::cout << "[Editor Error]: Failed to open file for writing: " << _path.string() << '\n';
    return false;
  }

  const uint32_t vertexCount = static_cast<uint32_t>(_mesh.vertices.size());
  const uint32_t indexCount = static_cast<uint32_t>(_mesh.indices.size());

  file.write(reinterpret_cast<const char*>(&vertexCount), sizeof(vertexCount));
  file.write(reinterpret_cast<const char*>(_mesh.vertices.data()), sizeof(Mesh::Vertex) * vertexCount);

  file.write(reinterpret_cast<const char*>(&indexCount), sizeof(indexCount));
  file.write(reinterpret_cast<const char*>(_mesh.indices.data()), sizeof(uint32_t) * indexCount);

  file.close();
  return true;
}
bool MeshHelper::LoadMesh(Mesh &_mesh, const std::filesystem::path &_path) {
  if(std::filesystem::exists(_path) == false) {
    std::cout << "[Editor Error]: File does not exist: " << _path.string() << '\n';
    return false;
  }

  std::ifstream file(_path, std::ios::binary);
  if(!file.is_open()) {
    std::cout << "[Editor Error]: Failed to open file for reading: " << _path.string() << '\n';
    return false;
  }

  uint32_t vertexCount = 0;
  uint32_t indexCount = 0;

  file.read(reinterpret_cast<char*>(&vertexCount), sizeof(vertexCount));
  _mesh.vertices.resize(vertexCount);
  file.read(reinterpret_cast<char*>(_mesh.vertices.data()), sizeof(Mesh::Vertex) * vertexCount);

  file.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
  _mesh.indices.resize(indexCount);
  file.read(reinterpret_cast<char*>(_mesh.indices.data()), sizeof(uint32_t) * indexCount);

  file.close();
  return true;
}

void MeshHelper::RandomizeMeshColors(Mesh &_mesh, const std::vector<glm::vec4> &_colors) {
  const uint32_t vertexCount = _mesh.vertices.size();
  const uint32_t colorCount = _colors.size();

  for(int i = 0; i < vertexCount; i++) {
    _mesh.vertices[i].color = _colors[i % colorCount];
  }
}

Mesh MeshHelper::GenerateNormalGizmos(const Mesh &_mesh, const glm::vec4 _color) {
  Mesh mesh(MeshType::Lines);
  
  for(int i = 0; i < _mesh.indices.size() / 3; i++) {
    const int start = i * 3;

    const glm::vec3 normal = _mesh.vertices[_mesh.indices[start]].normal;
    const glm::vec3 center = (
      _mesh.vertices[_mesh.indices[start+0]].position +
      _mesh.vertices[_mesh.indices[start+1]].position +
      _mesh.vertices[_mesh.indices[start+2]].position
    ) / 3.0f;

    mesh.indices.push_back(mesh.vertices.size());
    mesh.vertices.push_back(Mesh::Vertex{center, {0,0,1,1}});

    mesh.indices.push_back(mesh.vertices.size());
    mesh.vertices.push_back(Mesh::Vertex{center + normal, {0,0,1,1}});
  }

  return mesh;
}


glm::vec3 MeshHelper::CalculateFaceNormal(const glm::vec3 _a, const glm::vec3 _b, const glm::vec3 _c) {
  glm::vec3 edge1 = _b - _a;
  glm::vec3 edge2 = _c - _a;
  glm::vec3 normal = glm::cross(edge1, edge2);

  if(glm::length2(normal) < 0.00001f) return glm::vec3(0.0f, 0.0f, 0.0f); 

  return -glm::normalize(normal);
}
void MeshHelper::CalculateNormals(Mesh &_mesh, bool _useIndices) {
  if(_useIndices == false) {
    const unsigned int vertexCount = _mesh.vertices.size();

    for(int i = 0; i < vertexCount; i += 3) {
      Mesh::Vertex &a = _mesh.vertices[i + 0];
      Mesh::Vertex &b = _mesh.vertices[i + 1];
      Mesh::Vertex &c = _mesh.vertices[i + 2];

      const glm::vec3 normal = CalculateFaceNormal(a.position, b.position, c.position);

      a.normal = normal;
      b.normal = normal;
      c.normal = normal;
    }
  }
  else if(_useIndices == true) {
    const unsigned int indexCount = _mesh.indices.size();

    for(int i = 0; i < indexCount; i += 3) {
      Mesh::Vertex &a = _mesh.vertices[_mesh.indices[i + 0]];
      Mesh::Vertex &b = _mesh.vertices[_mesh.indices[i + 1]];
      Mesh::Vertex &c = _mesh.vertices[_mesh.indices[i + 2]];

      const glm::vec3 normal = CalculateFaceNormal(a.position, b.position, c.position);

      a.normal = normal;
      b.normal = normal;
      c.normal = normal;
    }
  }
}

Mesh MeshHelper::GenerateCircle(uint16_t _resolution, float _radius) {
  Mesh mesh;
  
  const float phiDif = glm::two_pi<float>() / _resolution;
  
  mesh.vertices.resize(1 + _resolution);
  mesh.vertices[0] = Mesh::Vertex{glm::vec3(0), {1,1,1, 1}, {0,0,1}};
  for(int i = 0; i < _resolution; i++) {
    mesh.vertices[i + 1] = Mesh::Vertex{
      glm::vec3{glm::cos(phiDif * i), -glm::sin(phiDif * i), 0} * _radius,
      {1,1,1, 1},
      {0,0,1},
    };
  }

  mesh.indices.resize(_resolution * 3);
  for(int i = 0; i < _resolution; i++) {
    const unsigned int a = 0;
    const unsigned int b = i + 1;
    const unsigned int c = b % _resolution + 1;

    const uint32_t start = i * 3;
    mesh.indices[start+0] = a;
    mesh.indices[start+1] = b;
    mesh.indices[start+2] = c;
  }

  return mesh;
}
Mesh MeshHelper::GenerateCube(const glm::vec3 _size) {
  Mesh mesh;

  const std::vector<Mesh::Vertex> vertices = {
    Mesh::Vertex{{-.5 * _size.x, -.5 * _size.y,  .5 * _size.z}, {1,1,1, 1}},
    Mesh::Vertex{{-.5 * _size.x,  .5 * _size.y,  .5 * _size.z}, {1,1,1, 1}},
    Mesh::Vertex{{ .5 * _size.x,  .5 * _size.y,  .5 * _size.z}, {1,1,1, 1}},
    Mesh::Vertex{{ .5 * _size.x, -.5 * _size.y,  .5 * _size.z}, {1,1,1, 1}},
    Mesh::Vertex{{-.5 * _size.x, -.5 * _size.y, -.5 * _size.z}, {1,1,1, 1}},
    Mesh::Vertex{{-.5 * _size.x,  .5 * _size.y, -.5 * _size.z}, {1,1,1, 1}},
    Mesh::Vertex{{ .5 * _size.x,  .5 * _size.y, -.5 * _size.z}, {1,1,1, 1}},
    Mesh::Vertex{{ .5 * _size.x, -.5 * _size.y, -.5 * _size.z}, {1,1,1, 1}},
  };
  const std::vector<unsigned int> indices = {
    0,1,2, 0,2,3, //+z
    7,6,5, 7,5,4, //-z
    
    0,4,5, 0,5,1, //-x
    3,2,6, 3,6,7, //+x
    
    0,3,7, 0,7,4, //-y
    1,5,6, 1,6,2, //+y
  };

  const unsigned int vertexCount = indices.size();
  mesh.vertices.resize(vertexCount);
  mesh.indices.resize(vertexCount);
  for(int i = 0; i < vertexCount; i++) {
    mesh.vertices[i] = vertices[indices[i]];
    mesh.indices[i] = i;
  }

  CalculateNormals(mesh);

  return mesh;
}
Mesh MeshHelper::GenerateCylinder(uint16_t _resolution, float _height, float _radius) {
  Mesh mesh;

  std::vector<Mesh::Vertex> vertices;
  std::vector<unsigned int> indices;

  const float phiDif = glm::two_pi<float>() / _resolution;

  vertices.push_back(Mesh::Vertex{glm::vec3{0, _height / 2.0,0}, {1,1,1,1}, {0, 1,0}});
  for(int i = 0; i < _resolution; i++) {
    vertices.push_back(Mesh::Vertex{
      {glm::cos(phiDif * i) * _radius, _height / 2.0, glm::sin(phiDif * i) * _radius},
      {1,1,1, 1},
      {0,1,0},
    });
  }
  vertices.push_back(Mesh::Vertex{glm::vec3{0,-_height / 2.0,0}, {1,1,1,1}, {0,-1,0}});
  for(int i = 0; i < _resolution; i++) {
    vertices.push_back(Mesh::Vertex{
      {glm::cos(phiDif * i) * _radius,-_height / 2.0, glm::sin(phiDif * i) * _radius},
      {1,1,1, 1},
      {0,-1,0},
    });
  }

  const unsigned int top = 0;
  const unsigned int bottom = top + 1 + _resolution;
  for(int i = 0; i < _resolution; i++) {
    const unsigned int a = top + 1 + i;
    const unsigned int b = top + 1 + (i + 1) % _resolution;
    const unsigned int c = bottom + 1 + i;
    const unsigned int d = bottom + 1 + (i + 1) % _resolution;

    indices.push_back(top);
    indices.push_back(a);
    indices.push_back(b);

    indices.push_back(d);
    indices.push_back(c);
    indices.push_back(bottom);

    indices.push_back(a);
    indices.push_back(c);
    indices.push_back(d);

    indices.push_back(a);
    indices.push_back(d);
    indices.push_back(b);
  }

  const unsigned int vertexCount = indices.size();
  mesh.vertices.resize(vertexCount);
  mesh.indices.resize(vertexCount);
  for(int i = 0; i < vertexCount; i++) {
    mesh.vertices[i] = vertices[indices[i]];
    mesh.indices[i] = i;
  }

  CalculateNormals(mesh);

  return mesh;
}
Mesh MeshHelper::GenerateQuad(const glm::vec2 _size) {
  Mesh mesh;

  mesh.vertices = {
    Mesh::Vertex{{-.5 * _size.x, -.5 * _size.y, 0}, {1,1,1, 1}, {0,0,1}},
    Mesh::Vertex{{-.5 * _size.x,  .5 * _size.y, 0}, {1,1,1, 1}, {0,0,1}},
    Mesh::Vertex{{ .5 * _size.x,  .5 * _size.y, 0}, {1,1,1, 1}, {0,0,1}},
    Mesh::Vertex{{ .5 * _size.x, -.5 * _size.y, 0}, {1,1,1, 1}, {0,0,1}},
  };
  mesh.indices = {0,1,2, 0,2,3};

  return mesh;
}
Mesh MeshHelper::GeneratePlane(const glm::vec2 _size, const glm::ivec2 _grid) {
  Mesh mesh;

  const glm::vec2 scalar = glm::vec2(_size.x / _grid.x, _size.y / _grid.y);

  for(int z = 0; z <= _grid.y; z++) {
    for(int x = 0; x <= _grid.x; x++) {
      mesh.vertices.push_back(Mesh::Vertex{
        {(x * scalar.x) - _size.x / 2.0f, 0, (-z * scalar.y) + _size.y / 2.0f},
        {1,1,1,1},
        {0,1,0}
      });
    }
  }
  for(int z = 0; z < _grid.y; z++) {
    for(int x = 0; x < _grid.x; x++) {
      const unsigned int a = x + z * (_grid.x + 1);
      const unsigned int b = a + _grid.x + 1;
      const unsigned int c = b + 1;
      const unsigned int d = a + 1;

      mesh.indices.push_back(a);
      mesh.indices.push_back(b);
      mesh.indices.push_back(c);

      mesh.indices.push_back(a);
      mesh.indices.push_back(c);
      mesh.indices.push_back(d);
    }
  }

  return mesh;
}
Mesh MeshHelper::GeneratePyramid(uint16_t _resolution, float _height, float _radius) {
  Mesh mesh;
  
  const float phiDif = glm::two_pi<float>() / _resolution;

  mesh.vertices.resize(1 + _resolution + 1);
  mesh.vertices[0] = Mesh::Vertex{{0, _height / 2.0, 0}, {1,1,1, 1}};
  for(int i = 0; i < _resolution; i++) {
    mesh.vertices[i + 1] = Mesh::Vertex{
      {glm::cos(phiDif * i) * _radius, -_height / 2.0, glm::sin(phiDif * i) * _radius},
      {1,1,1, 1}
    };
  }
  mesh.vertices[1 + _resolution] = Mesh::Vertex{{0, -_height / 2.0, 0}, {1,1,1, 1}};

  mesh.indices.resize(_resolution * 6);
  for(int i = 0; i < _resolution; i++) {
    const unsigned int a = 0;
    const unsigned int b = i + 1;
    const unsigned int c = b % _resolution + 1;
    const unsigned int d = 1 + _resolution;

    const uint32_t start = i * 6;
    mesh.indices[start+0] = a;
    mesh.indices[start+1] = b;
    mesh.indices[start+2] = c;

    mesh.indices[start+3] = d;
    mesh.indices[start+4] = c;
    mesh.indices[start+5] = b;
  }

  return mesh;
}
Mesh MeshHelper::GenerateUVSphere(uint16_t _segments, uint16_t _rings, float _radius) {
  Mesh mesh;

  for(unsigned int r = 0; r <= _rings; ++r) {
    float theta = r * glm::pi<float>() / _rings;
    float sinTheta = std::sin(theta);
    float cosTheta = std::cos(theta);

    for(unsigned int s = 0; s <= _segments; ++s) {
      float phi = s * glm::two_pi<float>() / _segments;
      float sinPhi = std::sin(phi);
      float cosPhi = std::cos(phi);


      const glm::vec3 position = {_radius * sinTheta * cosPhi, _radius * cosTheta,_radius * sinTheta * sinPhi};

      mesh.vertices.push_back(Mesh::Vertex{
        position,
        {1,1,1, 1},
        glm::normalize(position)
      });
    }
  }

  for(unsigned int r = 0; r < _rings; ++r) {
    for(unsigned int s = 0; s < _segments; ++s) {
      unsigned int first = r * (_segments + 1) + s;
      unsigned int second = first + _segments + 1;

      mesh.indices.push_back(first);
      mesh.indices.push_back(second);
      mesh.indices.push_back(first + 1);

      mesh.indices.push_back(first + 1);
      mesh.indices.push_back(second);
      mesh.indices.push_back(second + 1);
    }
  }

  return mesh;
}

Mesh MeshHelper::GenerateGrid(const glm::ivec2 _size, float _spacing) {
  Mesh mesh(MeshType::Lines);

  const glm::vec2 halfSize = glm::vec2(_size.x / 2.0f, _size.y / 2.0f) * _spacing;

  for(int x = 0; x <= _size.x; x++) {
    mesh.vertices.push_back(Mesh::Vertex{glm::vec3{x * _spacing - halfSize.x, 0, halfSize.y}, glm::vec4{1,1,1, 1}});
    mesh.vertices.push_back(Mesh::Vertex{glm::vec3{x * _spacing - halfSize.x, 0,-halfSize.y}, glm::vec4{1,1,1, 1}});
  }
  for(int y = 0; y <= _size.y; y++) {
    mesh.vertices.push_back(Mesh::Vertex{glm::vec3{ halfSize.x, 0, y * _spacing - halfSize.x}, glm::vec4{1,1,1, 1}});
    mesh.vertices.push_back(Mesh::Vertex{glm::vec3{-halfSize.x, 0, y * _spacing - halfSize.x}, glm::vec4{1,1,1, 1}});
  }

  const uint32_t vertexCount = mesh.vertices.size();
  mesh.indices.resize(vertexCount);
  for(int i = 0; i < vertexCount; i++) mesh.indices[i] = i;
  
  return mesh;
}
Mesh MeshHelper::GenerateWireSphere(uint16_t _resolution, float _radius) {
  Mesh mesh(MeshType::Lines);

  const float phiDif = glm::two_pi<float>() / _resolution;

  for(int i = 0; i < _resolution; i++) {
    const float phi = phiDif * i;
    mesh.vertices.push_back(Mesh::Vertex{{glm::cos(phi) * _radius,0,glm::sin(phi) * _radius}, {1,1,1, 1}});
  }
  for(int i = 0; i < _resolution; i++) {
    const float phi = phiDif * i;
    mesh.vertices.push_back(Mesh::Vertex{{glm::cos(phi) * _radius,glm::sin(phi) * _radius,0}, {1,1,1, 1}});
  }
  for(int i = 0; i < _resolution; i++) {
    const float phi = phiDif * i;
    mesh.vertices.push_back(Mesh::Vertex{{0,glm::sin(phi) * _radius,glm::cos(phi) * _radius}, {1,1,1, 1}});
  }

  for(int i = 0; i < 3; i++) {
    const uint32_t start = _resolution * i;
    for(int j = 0; j < _resolution; j++) {
      mesh.indices.push_back(start + j);
      mesh.indices.push_back(start + (j + 1) % _resolution);
    }
  }

  return mesh;
}
