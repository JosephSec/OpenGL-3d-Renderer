#include <Engine/EngineAPI.hpp>

#include <Engine/Renderer/Mesh.hpp>

#include <glm/trigonometric.hpp>


class ZENGINE_API MeshGenerator {
public:
  static Mesh Circle(uint16_t _resolution = 12, float _radius = .5f) {
    Mesh mesh;
    
    const float phiDif = glm::two_pi<float>() / _resolution;
    
    mesh.vertices.resize(1 + _resolution);
    mesh.vertices[0] = Mesh::Vertex{glm::vec3(0), {1,1,1, 1}};
    for(int i = 0; i < _resolution; i++) {
      mesh.vertices[i + 1] = Mesh::Vertex{
        glm::vec3{glm::cos(phiDif * i), -glm::sin(phiDif * i), 0} * _radius,
        glm::vec4{1,1,1, 1}
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
  static Mesh Cube(const glm::vec3 _size = glm::vec3(1)) {
    Mesh mesh;

    mesh.vertices = {
      Mesh::Vertex{{-.5 * _size.x, -.5 * _size.y,  .5 * _size.z}, {1,1,1, 1}},
      Mesh::Vertex{{-.5 * _size.x,  .5 * _size.y,  .5 * _size.z}, {1,1,1, 1}},
      Mesh::Vertex{{ .5 * _size.x,  .5 * _size.y,  .5 * _size.z}, {1,1,1, 1}},
      Mesh::Vertex{{ .5 * _size.x, -.5 * _size.y,  .5 * _size.z}, {1,1,1, 1}},
      Mesh::Vertex{{-.5 * _size.x, -.5 * _size.y, -.5 * _size.z}, {1,1,1, 1}},
      Mesh::Vertex{{-.5 * _size.x,  .5 * _size.y, -.5 * _size.z}, {1,1,1, 1}},
      Mesh::Vertex{{ .5 * _size.x,  .5 * _size.y, -.5 * _size.z}, {1,1,1, 1}},
      Mesh::Vertex{{ .5 * _size.x, -.5 * _size.y, -.5 * _size.z}, {1,1,1, 1}},
    };
    mesh.indices = {
      0,1,2, 0,2,3, //+z
      7,6,5, 7,5,4, //-z
      
      0,4,5, 0,5,1, //-x
      3,2,6, 3,6,7, //+x
      
      0,3,7, 0,7,4, //-y
      1,5,6, 1,6,2, //+y
    };

    return mesh;
  }
  static Mesh Cylinder(uint16_t _resolution = 12, float _height = 1, float _radius = .5f) {
    Mesh mesh;

    const float phiDif = glm::two_pi<float>() / _resolution;

    mesh.vertices.push_back(Mesh::Vertex{glm::vec3{0,_height / 2.0,0}, {1,1,1, 1}});
    for(int i = 0; i < _resolution; i++) {
      mesh.vertices.push_back(Mesh::Vertex{{glm::cos(phiDif * i) * _radius, _height / 2.0, glm::sin(phiDif * i) * _radius}, {1,1,1, 1}});
    }
    mesh.vertices.push_back(Mesh::Vertex{glm::vec3{0,-_height / 2.0,0}, {1,1,1, 1}});
    for(int i = 0; i < _resolution; i++) {
      mesh.vertices.push_back(Mesh::Vertex{{glm::cos(phiDif * i) * _radius,-_height / 2.0, glm::sin(phiDif * i) * _radius}, {1,1,1, 1}});
    }

    const unsigned int top = 0;
    const unsigned int bottom = 1 + _resolution;
    for(int i = 0; i < _resolution; i++) {
      const unsigned int a = i + 1;
      const unsigned int b = a % _resolution + 1;
      const unsigned int c = bottom + a;
      const unsigned int d = bottom + b;

      mesh.indices.push_back(top);
      mesh.indices.push_back(a);
      mesh.indices.push_back(b);

      mesh.indices.push_back(d);
      mesh.indices.push_back(c);
      mesh.indices.push_back(bottom);

      mesh.indices.push_back(a);
      mesh.indices.push_back(c);
      mesh.indices.push_back(d);

      mesh.indices.push_back(a);
      mesh.indices.push_back(d);
      mesh.indices.push_back(b);
    }

    return mesh;
  }
  static Mesh Quad(const glm::vec2 _size = glm::vec2(1)) {
    Mesh mesh;

    mesh.vertices = {
      Mesh::Vertex{{-.5 * _size.x, -.5 * _size.y, 0}, {1,1,1, 1}},
      Mesh::Vertex{{-.5 * _size.x,  .5 * _size.y, 0}, {1,1,1, 1}},
      Mesh::Vertex{{ .5 * _size.x,  .5 * _size.y, 0}, {1,1,1, 1}},
      Mesh::Vertex{{ .5 * _size.x, -.5 * _size.y, 0}, {1,1,1, 1}},
    };
    mesh.indices = {0,1,2, 0,2,3};

    return mesh;
  }
  static Mesh Pyramid(uint16_t _resolution = 4, float _height = 1, float _radius = .5f) {
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
  static Mesh UVSphere(uint16_t _segments = 16, uint16_t _rings = 16, float _radius = .5f) {
    Mesh mesh;

    for(unsigned int r = 0; r <= _rings; ++r) {
      float theta = r * glm::pi<float>() / _rings;
      float sinTheta = std::sin(theta);
      float cosTheta = std::cos(theta);

      for(unsigned int s = 0; s <= _segments; ++s) {
        float phi = s * glm::two_pi<float>() / _segments;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);


        mesh.vertices.push_back(Mesh::Vertex{
          {_radius * sinTheta * cosPhi, _radius * cosTheta,_radius * sinTheta * sinPhi},
          {1,1,1, 1}
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

  static Mesh Grid(const glm::ivec2 _size, float _spacing = 1) {
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
  static Mesh WireSphere(uint16_t _resolution, float _radius) {
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
};