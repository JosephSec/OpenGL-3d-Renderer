#include <Renderer/Pipeline/MeshHelper.hpp>
using namespace Renderer;

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>

#include <fstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

//MOST OF THIS FILE IS WRITTEN BY CLAUDE
//MOST OF THIS FILE IS WRITTEN BY CLAUDE
//MOST OF THIS FILE IS WRITTEN BY CLAUDE
//MOST OF THIS FILE IS WRITTEN BY CLAUDE
//MOST OF THIS FILE IS WRITTEN BY CLAUDE


static std::vector<std::string> split_string(const std::string &_str, char _ch) {
  std::vector<std::string> tokens;

  std::string buffer;
  for(const char &ch : _str) {
    if(ch == _ch && buffer.empty() == false) {
      tokens.push_back(buffer);
      buffer.clear();
      continue;
    }

    buffer.push_back(ch);
  }

  if(buffer.empty() == false) tokens.push_back(buffer);

  return tokens;
}



bool MeshHelper::SaveMesh(const Mesh &_mesh, const std::filesystem::path &_path) {
  if(std::filesystem::exists(_path.parent_path()) == false) {
    std::filesystem::create_directories(_path.parent_path());
  }

  std::ofstream file(_path, std::ios::binary);
  if(!file.is_open()) {
    std::cout << "[MESH HELPER ERROR]: Failed to open file for writing: " << _path.string() << '\n';
    return false;
  }

  const size_t vertexCount = _mesh.vertices.size();
  const size_t indexCount = _mesh.indices.size();

  file.write(reinterpret_cast<const char*>(&vertexCount), sizeof(size_t));
  file.write(reinterpret_cast<const char*>(_mesh.vertices.data()), sizeof(Mesh::Vertex) * vertexCount);

  file.write(reinterpret_cast<const char*>(&indexCount), sizeof(size_t));
  file.write(reinterpret_cast<const char*>(_mesh.indices.data()), sizeof(size_t) * indexCount);

  file.close();
  return true;
}
bool MeshHelper::LoadMesh(Mesh &_mesh, const std::filesystem::path &_path) {
  if(std::filesystem::exists(_path) == false) {
    std::cout << "[MESH HELPER ERROR]: File does not exist: " << _path.string() << '\n';
    return false;
  }

  std::ifstream file(_path, std::ios::binary);
  if(file.is_open() == false) {
    std::cout << "[MESH HELPER ERROR]: Failed to open file for reading: " << _path.string() << '\n';
    return false;
  }

  size_t vertexCount = 0;
  file.read(reinterpret_cast<char*>(&vertexCount), sizeof(size_t));

  std::vector<Mesh::Vertex> vertices(vertexCount);
  file.read(reinterpret_cast<char*>(vertices.data()), sizeof(Mesh::Vertex) * vertexCount);
  _mesh.vertices = vertices;
  
  
  size_t indexCount = 0;
  file.read(reinterpret_cast<char*>(&indexCount), sizeof(size_t));

  std::vector<unsigned int> indices(indexCount);
  file.read(reinterpret_cast<char*>(indices.data()), sizeof(unsigned int) * indexCount);
  _mesh.indices = indices;

  file.close();
  return true;
}

bool MeshHelper::LoadMeshObj(Mesh &_mesh, const std::filesystem::path &_path) {
  if(std::filesystem::exists(_path) == false) {
    std::cout << "[MESH HELPER ERROR]: File does not exist: " << _path.string() << '\n';
    return false;
  }

  std::ifstream file(_path);
  if(file.is_open() == false) {
    std::cout << "[MESH HELPER ERROR]: Failed to open file for reading: " << _path.string() << '\n';
    return false;
  }

  std::vector<glm::vec3> temp_positions;
  std::vector<glm::vec2> temp_uvs;
  std::vector<glm::vec3> temp_normals;

  std::vector<Mesh::Vertex> vertices;
  std::vector<unsigned int> indices;

  std::string str;
  while(std::getline(file, str)) {
    const uint32_t strSize = str.size();

    for(int i = 0; i < strSize; i++) {
      char &ch = str[i];
      if(ch != ' ') continue;

      const std::string prefix = str.substr(0,i);
      if(prefix == "v") {
        const std::vector<std::string> parts = split_string(str.substr(i + 1), ' ');
        temp_positions.push_back({std::stof(parts[0]),std::stof(parts[1]),std::stof(parts[2])});
      }
      else if(prefix == "vt") {
        const std::vector<std::string> parts = split_string(str.substr(i + 1), ' ');
        temp_uvs.push_back({std::stof(parts[0]),std::stof(parts[1])});
      }
      else if(prefix == "vn") {
        const std::vector<std::string> parts = split_string(str.substr(i + 1), ' ');
        temp_normals.push_back({std::stof(parts[0]),std::stof(parts[1]),std::stof(parts[2])});
      }
      else if(prefix == "f") {
        const std::vector<std::string> parts = split_string(str.substr(i + 1), ' ');

        for(const std::string &part : parts) {
          const std::vector<std::string> subParts = split_string(part, '/');

          int vIndex = std::stoi(subParts[0]) - 1;
          int uIndex = std::stoi(subParts[1]) - 1;
          int nIndex = std::stoi(subParts[2]) - 1;

          vertices.push_back(Mesh::Vertex{temp_positions[vIndex], {1,1,1,1}, -temp_normals[nIndex], temp_uvs[uIndex]});
          indices.push_back(vertices.size() - 1);
        }
      }

      break;
    }
  }

  _mesh.vertices = vertices;
  _mesh.indices = indices;

  file.close();
  return true;
}

GLuint MeshHelper::LoadTexture(const std::filesystem::path &_path) {
  GLuint textureId;
  glGenTextures(1, &textureId);

  int width, height, nrComponents;

  stbi_set_flip_vertically_on_load(true);
  unsigned char *data = stbi_load(_path.string().c_str(), &width, &height, &nrComponents, 0);

  if(data != nullptr) {
    GLenum format = GL_RGB;
         if(nrComponents == 1) format = GL_RED;
    else if(nrComponents == 3) format = GL_RGB;
    else if(nrComponents == 4) format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  else if(data == nullptr) {
    std::cout << "[TEXTURE ERROR]: file was not found or could not be opened\n";
    glDeleteTextures(1, &textureId);
    textureId = 0;
  }

  stbi_image_free(data);

  return textureId;
}


void MeshHelper::SetMeshColor(Mesh &_mesh, const glm::vec4 _color) {
  _mesh.editVertices([&](std::vector<Mesh::Vertex> &_vertices) {
    for(Mesh::Vertex &vertex : _vertices) vertex.color = _color;
  });
}

Mesh MeshHelper::GenerateNormalGizmo(const Mesh &_mesh, const glm::vec4 _color, float _length) {
  Mesh gizmo(GL_LINES);

  const auto &vertices = _mesh.vertices;
  const auto &indices  = _mesh.indices;;
  if(_mesh.type != GL_TRIANGLES || indices.size() < 3) return gizmo;

  const size_t triangleCount = indices.size() / 3;

  std::vector<Mesh::Vertex> outVertices;
  std::vector<unsigned int> outIndices;
  outVertices.reserve(triangleCount * 2);
  outIndices.reserve(triangleCount * 2);

  for(size_t i = 0; i < triangleCount; i++) {
    const glm::vec3 &p0 = vertices[indices[i*3 + 0]].position;
    const glm::vec3 &p1 = vertices[indices[i*3 + 1]].position;
    const glm::vec3 &p2 = vertices[indices[i*3 + 2]].position;

    const glm::vec3 cross = glm::cross(p1 - p0, p2 - p0);
    const float lenSq = glm::dot(cross, cross);
    if(lenSq < 1e-12f) continue;                       // degenerate triangle

    const glm::vec3 normal = cross / glm::sqrt(lenSq);
    const glm::vec3 center = (p0 + p1 + p2) / 3.0f;

    const unsigned int base = static_cast<unsigned int>(outVertices.size());
    outVertices.push_back(Mesh::Vertex{center, _color});
    outVertices.push_back(Mesh::Vertex{center + normal * _length, _color});
    outIndices.push_back(base);
    outIndices.push_back(base + 1);
  }

  gizmo.vertices = outVertices;
  gizmo.indices = outIndices;
  return gizmo;
}


glm::vec3 MeshHelper::CalculateFaceNormal(const glm::vec3 _a, const glm::vec3 _b, const glm::vec3 _c) {
  glm::vec3 edge1 = _b - _a;
  glm::vec3 edge2 = _c - _a;
  glm::vec3 normal = glm::cross(edge1, edge2);

  if(glm::length2(normal) < 0.00001f) return glm::vec3(0); 

  return -glm::normalize(normal);
}
void MeshHelper::CalculateSmoothNormals(Mesh &_mesh) {
  const std::vector<unsigned int> indices = _mesh.indices;;
  if(indices.size() < 3) return;

  _mesh.editVertices([&](std::vector<Mesh::Vertex> &vertices) {
    for(Mesh::Vertex &v : vertices) v.normal = glm::vec3(0);

    for(size_t i = 0; i + 2 < indices.size(); i += 3) {
      Mesh::Vertex &a = vertices[indices[i + 0]];
      Mesh::Vertex &b = vertices[indices[i + 1]];
      Mesh::Vertex &c = vertices[indices[i + 2]];

      const glm::vec3 faceNormal = glm::cross(b.position - a.position, c.position - a.position);
      a.normal += faceNormal;
      b.normal += faceNormal;
      c.normal += faceNormal;
    }

    for(Mesh::Vertex &v : vertices) {
      const float lenSq = glm::dot(v.normal, v.normal);
      v.normal = lenSq > 1e-12f ? v.normal / glm::sqrt(lenSq) : glm::vec3(0, 1, 0);
    }
  });
}
void MeshHelper::CalculateFlatNormals(Mesh &_mesh) {
  const std::vector<Mesh::Vertex> &src = _mesh.vertices;
  const std::vector<unsigned int> &idx = _mesh.indices;;
  const bool indexed = !idx.empty();
  const size_t count = indexed ? idx.size() : src.size();

  std::vector<Mesh::Vertex> outVertices;
  std::vector<unsigned int> outIndices;
  outVertices.reserve(count);
  outIndices.reserve(count);

  for(size_t i = 0; i + 2 < count; i += 3) {
    Mesh::Vertex a = src[indexed ? idx[i + 0] : i + 0];
    Mesh::Vertex b = src[indexed ? idx[i + 1] : i + 1];
    Mesh::Vertex c = src[indexed ? idx[i + 2] : i + 2];

    const glm::vec3 cross = glm::cross(b.position - a.position, c.position - a.position);
    const float lenSq = glm::dot(cross, cross);
    const glm::vec3 normal = lenSq > 1e-12f ? cross / glm::sqrt(lenSq) : glm::vec3(0, 1, 0);

    a.normal = b.normal = c.normal = normal;

    const unsigned int base = static_cast<unsigned int>(outVertices.size());
    outVertices.push_back(a);
    outVertices.push_back(b);
    outVertices.push_back(c);
    outIndices.push_back(base + 0);
    outIndices.push_back(base + 1);
    outIndices.push_back(base + 2);
  }

  _mesh.vertices = outVertices;
  _mesh.indices = outIndices;
}

Mesh MeshHelper::GenerateCircle(uint16_t _resolution, float _radius) {
  Mesh mesh;
  if(_resolution < 3) return mesh;

  const float phiDif = glm::two_pi<float>() / _resolution;

  std::vector<Mesh::Vertex> vertices;
  vertices.reserve(1 + _resolution);
  vertices.push_back(Mesh::Vertex{glm::vec3(0), {1,1,1,1}, {0,0,1}, {.5f,.5f}});

  for(unsigned int i = 0; i < _resolution; i++) {
    const glm::vec2 dir{glm::cos(phiDif * i), glm::sin(phiDif * i)};  // CCW seen from +Z

    vertices.push_back(Mesh::Vertex{
      glm::vec3(dir * _radius, 0),
      {1,1,1,1},
      {0,0,1},
      dir * 0.5f + 0.5f
    });
  }

  std::vector<unsigned int> indices;
  indices.reserve(_resolution * 3);

  for(unsigned int i = 0; i < _resolution; i++) {
    const unsigned int b = (i + 1 == _resolution) ? 1 : i + 2;
    const unsigned int c = i + 1;

    indices.push_back(0);
    indices.push_back(b);
    indices.push_back(c);
  }

  mesh.vertices = vertices;
  mesh.indices = indices;
  return mesh;
}
Mesh MeshHelper::GenerateCube(const glm::vec3 _size) {
  Mesh mesh;
  const glm::vec3 half = _size * 0.5f;

  // n = outward normal, u/v = face axes, chosen so cross(u, v) == n
  struct Face { glm::vec3 n, u, v; };
  const Face faces[6] = {
    {{ 0, 0, 1}, { 1, 0, 0}, {0, 1, 0}},  // +z
    {{ 0, 0,-1}, {-1, 0, 0}, {0, 1, 0}},  // -z
    {{ 1, 0, 0}, { 0, 1, 0}, {0, 0, 1}},  // +x
    {{-1, 0, 0}, { 0, 0, 1}, {0, 1, 0}},  // -x
    {{ 0, 1, 0}, { 0, 0, 1}, {1, 0, 0}},  // +y
    {{ 0,-1, 0}, { 1, 0, 0}, {0, 0, 1}},  // -y
  };
  const glm::vec2 corners[4] = {{-1,-1}, { 1,-1}, { 1, 1}, {-1, 1}};
  const glm::vec2 uvs[4]     = {{ 0, 0}, { 1, 0}, { 1, 1}, { 0, 1}};

  std::vector<Mesh::Vertex> vertices;
  std::vector<unsigned int> indices;
  vertices.reserve(24);
  indices.reserve(36);

  for(const Face &f : faces) {
    const unsigned int base = static_cast<unsigned int>(vertices.size());

    for(int i = 0; i < 4; i++) {
      const glm::vec3 position = (f.n + f.u * corners[i].x + f.v * corners[i].y) * half;
      vertices.push_back(Mesh::Vertex{position, {1,1,1,1}, f.n, uvs[i]});
    }

    indices.insert(indices.end(), {base, base+2, base+1, base, base+3, base+2});
  }

  mesh.vertices = vertices;
  mesh.indices = indices;
  return mesh;
}
Mesh MeshHelper::GenerateCylinder(uint16_t _resolution, float _height, float _radius) {
  Mesh mesh;
  if(_resolution < 3) return mesh;

  const float phiDif = glm::two_pi<float>() / _resolution;
  const float half = _height * 0.5f;

  std::vector<Mesh::Vertex> vertices;
  std::vector<unsigned int> indices;
  vertices.reserve(4 * _resolution + 4);
  indices.reserve(12 * _resolution);

  // side wall: resolution + 1 columns, so the u seam has its own vertices
  for(unsigned int i = 0; i <= _resolution; i++) {
    const glm::vec2 dir{glm::cos(phiDif * i), glm::sin(phiDif * i)};
    const float u = static_cast<float>(i) / _resolution;
    const glm::vec3 normal{dir.x, 0, dir.y};

    vertices.push_back(Mesh::Vertex{{dir.x * _radius,  half, dir.y * _radius}, {1,1,1,1}, normal, {u, 1}});
    vertices.push_back(Mesh::Vertex{{dir.x * _radius, -half, dir.y * _radius}, {1,1,1,1}, normal, {u, 0}});
  }
  for(unsigned int i = 0; i < _resolution; i++) {
    const unsigned int t0 = i * 2, b0 = t0 + 1, t1 = t0 + 2, b1 = t0 + 3;

    indices.insert(indices.end(), {t0, b0, b1});
    indices.insert(indices.end(), {t0, b1, t1});
  }

  // caps: own vertices so they get flat normals and planar UVs
  for(int cap = 0; cap < 2; cap++) {
    const bool top = (cap == 0);
    const float y = top ? half : -half;
    const glm::vec3 normal{0, top ? 1.0f : -1.0f, 0};

    const unsigned int center = static_cast<unsigned int>(vertices.size());
    vertices.push_back(Mesh::Vertex{{0, y, 0}, {1,1,1,1}, normal, {.5f, .5f}});

    for(unsigned int i = 0; i < _resolution; i++) {
      const glm::vec2 dir{glm::cos(phiDif * i), glm::sin(phiDif * i)};
      vertices.push_back(Mesh::Vertex{{dir.x * _radius, y, dir.y * _radius}, {1,1,1,1}, normal, dir * 0.5f + 0.5f});
    }

    for(unsigned int i = 0; i < _resolution; i++) {
      const unsigned int a = center + 1 + i;
      const unsigned int b = center + 1 + (i + 1) % _resolution;

      if(top) indices.insert(indices.end(), {center, b, a});
      else    indices.insert(indices.end(), {center, a, b});
    }
  }

  mesh.vertices = vertices;
  mesh.indices = indices;
  return mesh;
}
Mesh MeshHelper::GenerateQuad(const glm::vec2 _size) {
  Mesh mesh;

  const glm::vec2 half = _size / 2.0f;

  mesh.vertices = {
    Mesh::Vertex{{-half.x, -half.y, 0}, {1,1,1,1}, {0,0,1}, {0,0}},
    Mesh::Vertex{{-half.x,  half.y, 0}, {1,1,1,1}, {0,0,1}, {0,1}},
    Mesh::Vertex{{ half.x,  half.y, 0}, {1,1,1,1}, {0,0,1}, {1,1}},
    Mesh::Vertex{{ half.x, -half.y, 0}, {1,1,1,1}, {0,0,1}, {1,0}},
  };
  mesh.indices = {0,1,2, 0,2,3};

  return mesh;
}
Mesh MeshHelper::GeneratePlane(const glm::vec2 _size, const glm::ivec2 _grid) {
  Mesh mesh;
  if(_grid.x < 1 || _grid.y < 1) return mesh;

  const glm::vec2 cellScalar = _size / glm::vec2(_grid);
  const glm::vec2 uvScalar = glm::vec2(1) / glm::vec2(_grid);

  std::vector<Mesh::Vertex> vertices;
  std::vector<unsigned int> indices;
  vertices.reserve((_grid.x + 1) * (_grid.y + 1));
  indices.reserve(_grid.x * _grid.y * 6);

  for(int z = 0; z <= _grid.y; z++) {
    for(int x = 0; x <= _grid.x; x++) {
      vertices.push_back(Mesh::Vertex{
        {x * cellScalar.x - _size.x / 2.0f, 0, -z * cellScalar.y + _size.y / 2.0f},
        {1,1,1,1},
        {0,1,0},
        glm::vec2{x, _grid.y - z} * uvScalar
      });
    }
  }

  for(int z = 0; z < _grid.y; z++) {
    for(int x = 0; x < _grid.x; x++) {
      const unsigned int a = x + z * (_grid.x + 1);
      const unsigned int b = a + _grid.x + 1;
      const unsigned int c = b + 1;
      const unsigned int d = a + 1;

      indices.insert(indices.end(), {a, b, c,  a, c, d});  // CW from +Y
    }
  }

  mesh.vertices = vertices;
  mesh.indices = indices;
  return mesh;
}
Mesh MeshHelper::GeneratePyramid(uint16_t _resolution, float _height, float _radius) {
  Mesh mesh;
  if(_resolution < 3) return mesh;

  const float phiDif = glm::two_pi<float>() / _resolution;
  const float half = _height * 0.5f;

  std::vector<Mesh::Vertex> vertices;
  std::vector<unsigned int> indices;
  vertices.reserve(3 * _resolution + 2 + _resolution);
  indices.reserve(_resolution * 6);

  // side ring: resolution + 1 columns so the u seam has its own vertices
  for(unsigned int i = 0; i <= _resolution; i++) {
    const glm::vec2 dir{glm::cos(phiDif * i), glm::sin(phiDif * i)};
    const glm::vec3 normal = glm::normalize(glm::vec3(_height * dir.x, _radius, _height * dir.y));

    vertices.push_back(Mesh::Vertex{
      {dir.x * _radius, -half, dir.y * _radius}, {1,1,1,1}, normal,
      {static_cast<float>(i) / _resolution, 0}
    });
  }

  // one apex per side face, with the normal of the face's middle angle
  const unsigned int apexBase = static_cast<unsigned int>(vertices.size());
  for(unsigned int i = 0; i < _resolution; i++) {
    const float mid = phiDif * (i + 0.5f);
    const glm::vec3 normal = glm::normalize(glm::vec3(_height * glm::cos(mid), _radius, _height * glm::sin(mid)));

    vertices.push_back(Mesh::Vertex{
      {0, half, 0}, {1,1,1,1}, normal,
      {(i + 0.5f) / _resolution, 1}
    });
  }

  for(unsigned int i = 0; i < _resolution; i++) {
    indices.insert(indices.end(), {apexBase + i, i, i + 1});  // same order as your original (CW)
  }

  // base: own vertices for the flat normal and planar UVs
  const unsigned int center = static_cast<unsigned int>(vertices.size());
  vertices.push_back(Mesh::Vertex{{0, -half, 0}, {1,1,1,1}, {0,-1,0}, {.5f,.5f}});

  for(unsigned int i = 0; i < _resolution; i++) {
    const glm::vec2 dir{glm::cos(phiDif * i), glm::sin(phiDif * i)};
    vertices.push_back(Mesh::Vertex{
      {dir.x * _radius, -half, dir.y * _radius}, {1,1,1,1}, {0,-1,0}, dir * 0.5f + 0.5f
    });
  }

  for(unsigned int i = 0; i < _resolution; i++) {
    const unsigned int b = center + 1 + i;
    const unsigned int c = center + 1 + (i + 1) % _resolution;
    indices.insert(indices.end(), {center, c, b});            // same order as your original (CW)
  }

  mesh.vertices = vertices;
  mesh.indices = indices;
  return mesh;
}
Mesh MeshHelper::GenerateUVSphere(uint16_t _segments, uint16_t _rings, float _radius) {
  Mesh mesh;
  if(_segments < 3 || _rings < 2 || _radius <= 0.0f) return mesh;

  std::vector<Mesh::Vertex> vertices;
  std::vector<unsigned int> indices;
  vertices.reserve((_rings + 1) * (_segments + 1));
  indices.reserve(_rings * _segments * 6);

  for(unsigned int r = 0; r <= _rings; ++r) {
    const float v = static_cast<float>(r) / _rings;
    const float theta = v * glm::pi<float>();
    const float sinTheta = glm::sin(theta);
    const float cosTheta = glm::cos(theta);

    for(unsigned int s = 0; s <= _segments; ++s) {
      const float u = static_cast<float>(s) / _segments;
      const float phi = u * glm::two_pi<float>();

      const glm::vec3 normal = {sinTheta * glm::cos(phi), cosTheta, sinTheta * glm::sin(phi)};

      vertices.push_back(Mesh::Vertex{
        normal * _radius,
        {1,1,1,1},
        normal,
        {u, 1.0f - v}      // v = 1 at the top pole, matching the cylinder
      });
    }
  }

  for(unsigned int r = 0; r < _rings; ++r) {
    for(unsigned int s = 0; s < _segments; ++s) {
      const unsigned int first  = r * (_segments + 1) + s;
      const unsigned int second = first + _segments + 1;

      if(r != 0)
        indices.insert(indices.end(), {first, second, first + 1});
      if(r != _rings - 1u)
        indices.insert(indices.end(), {first + 1, second, second + 1});
    }
  }

  mesh.vertices = vertices;
  mesh.indices = indices;
  return mesh;
}

Mesh MeshHelper::GenerateGrid(const glm::ivec2 _size, float _spacing) {
  Mesh mesh(GL_LINES);
  if(_size.x < 1 || _size.y < 1) return mesh;

  const glm::vec2 halfSize = glm::vec2(_size) * 0.5f * _spacing;
  const glm::vec4 white{1,1,1,1};

  std::vector<Mesh::Vertex> vertices;
  vertices.reserve((_size.x + 1 + _size.y + 1) * 2);

  for(int x = 0; x <= _size.x; x++) {
    const float px = x * _spacing - halfSize.x;
    vertices.push_back(Mesh::Vertex{{px, 0,  halfSize.y}, white});
    vertices.push_back(Mesh::Vertex{{px, 0, -halfSize.y}, white});
  }
  for(int y = 0; y <= _size.y; y++) {
    const float pz = y * _spacing - halfSize.y;   // was halfSize.x
    vertices.push_back(Mesh::Vertex{{ halfSize.x, 0, pz}, white});
    vertices.push_back(Mesh::Vertex{{-halfSize.x, 0, pz}, white});
  }

  std::vector<unsigned int> indices(vertices.size());
  for(unsigned int i = 0; i < indices.size(); i++) indices[i] = i;

  mesh.vertices = vertices;
  mesh.indices = indices;
  return mesh;
}
Mesh MeshHelper::GenerateWireSphere(uint16_t _resolution, float _radius) {
  Mesh mesh(GL_LINES);
  if(_resolution < 3) return mesh;

  const float phiDif = glm::two_pi<float>() / _resolution;
  const glm::vec4 white{1,1,1,1};

  std::vector<Mesh::Vertex> vertices;
  std::vector<unsigned int> indices;
  vertices.reserve(_resolution * 3);
  indices.reserve(_resolution * 6);

  for(unsigned int i = 0; i < _resolution; i++) {
    const float c = glm::cos(phiDif * i) * _radius;
    const float s = glm::sin(phiDif * i) * _radius;
    vertices.push_back(Mesh::Vertex{{c, 0, s}, white});   // XZ circle
  }
  for(unsigned int i = 0; i < _resolution; i++) {
    const float c = glm::cos(phiDif * i) * _radius;
    const float s = glm::sin(phiDif * i) * _radius;
    vertices.push_back(Mesh::Vertex{{c, s, 0}, white});   // XY circle
  }
  for(unsigned int i = 0; i < _resolution; i++) {
    const float c = glm::cos(phiDif * i) * _radius;
    const float s = glm::sin(phiDif * i) * _radius;
    vertices.push_back(Mesh::Vertex{{0, s, c}, white});   // YZ circle
  }

  for(unsigned int i = 0; i < 3; i++) {
    const unsigned int start = _resolution * i;
    for(unsigned int j = 0; j < _resolution; j++) {
      indices.push_back(start + j);
      indices.push_back(start + (j + 1) % _resolution);
    }
  }

  mesh.vertices = vertices;
  mesh.indices = indices;
  return mesh;
}
Mesh MeshHelper::GenerateWireCube(const glm::vec3 _size) {
  Mesh mesh(GL_LINES);
  const glm::vec3 h = _size * 0.5f;
  const glm::vec4 white{1,1,1,1};

  mesh.vertices = {
    Mesh::Vertex{{-h.x, -h.y,  h.z}, white},
    Mesh::Vertex{{-h.x,  h.y,  h.z}, white},
    Mesh::Vertex{{ h.x,  h.y,  h.z}, white},
    Mesh::Vertex{{ h.x, -h.y,  h.z}, white},
    Mesh::Vertex{{-h.x, -h.y, -h.z}, white},
    Mesh::Vertex{{-h.x,  h.y, -h.z}, white},
    Mesh::Vertex{{ h.x,  h.y, -h.z}, white},
    Mesh::Vertex{{ h.x, -h.y, -h.z}, white},
  };
  mesh.indices = {
    0,1, 1,2, 2,3, 3,0,
    4,5, 5,6, 6,7, 7,4,
    0,4, 1,5, 2,6, 3,7
  };

  return mesh;
}
Mesh MeshHelper::GenerateWireCone(uint16_t _baseResolution, uint16_t _apexResolution, float _degrees, float _length) {
  Mesh mesh(GL_LINES);
  if(_baseResolution < 3) return mesh;

  const unsigned int apexCount = glm::min<unsigned int>(_apexResolution, _baseResolution);
  const float radians = glm::radians(glm::clamp(_degrees, 0.0f, 89.0f));
  const float radius = glm::tan(radians) * _length;
  const float phiDif = glm::two_pi<float>() / _baseResolution;
  const glm::vec4 white{1,1,1,1};

  std::vector<Mesh::Vertex> vertices;
  std::vector<unsigned int> indices;
  vertices.reserve(_baseResolution + 1);
  indices.reserve(_baseResolution * 2 + apexCount * 2);

  // base ring
  for(unsigned int i = 0; i < _baseResolution; i++) {
    const float a = phiDif * i;
    vertices.push_back(Mesh::Vertex{{glm::cos(a) * radius, glm::sin(a) * radius, -_length}, white});
    indices.push_back(i);
    indices.push_back((i + 1) % _baseResolution);
  }

  // apex, plus spokes spread evenly around the ring
  const unsigned int apex = _baseResolution;
  vertices.push_back(Mesh::Vertex{{0, 0, 0}, white});

  for(unsigned int i = 0; i < apexCount; i++) {
    indices.push_back(apex);
    indices.push_back(i * _baseResolution / apexCount);
  }

  mesh.vertices = vertices;
  mesh.indices = indices;
  return mesh;
}
