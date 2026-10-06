#pragma once

#include <filesystem>
#include <glm/trigonometric.hpp>

#include <Renderer/Core/API.hpp>
#include <Renderer/Component/Mesh.hpp>
#include <Renderer/Component/Transform.hpp>


namespace Renderer {
  class HORDE3D_API MeshHelper {
  public:  
    static bool SaveMesh(const Mesh &_mesh, const std::filesystem::path &_path);
    static bool LoadMesh(Mesh &_mesh, const std::filesystem::path &_path);

    static bool LoadMeshObj(Mesh &_mesh, const std::filesystem::path &_path);

    static GLuint LoadTexture(const std::filesystem::path &_path);


    static void SetMeshColor(Mesh &_mesh, const glm::vec4 _color);
    static inline void SetMeshColor(Mesh &_mesh, const glm::vec3 _color) {
      SetMeshColor(_mesh, glm::vec4{_color, 1});
    }

    
    static Mesh GenerateNormalGizmo(const Mesh &_mesh, const glm::vec4 _color, float _length = 1);

    static glm::vec3 CalculateFaceNormal(const glm::vec3 _a, const glm::vec3 _b, const glm::vec3 _c);
    static void CalculateSmoothNormals(Mesh &_mesh);
    static void CalculateFlatNormals(Mesh &_mesh);


    static Mesh GenerateCircle(uint16_t _resolution = 12, float _radius = .5f);
    static Mesh GenerateCube(const glm::vec3 _size = glm::vec3(1));
    static Mesh GenerateCylinder(uint16_t _resolution = 12, float _height = 1, float _radius = .5f);
    static Mesh GenerateQuad(const glm::vec2 _size = glm::vec2(1));
    static Mesh GeneratePlane(const glm::vec2 _size = glm::vec2(10), const glm::ivec2 _grid = glm::ivec2(10));
    static Mesh GeneratePyramid(uint16_t _resolution = 4, float _height = 1, float _radius = .5f);
    static Mesh GenerateUVSphere(uint16_t _segments = 16, uint16_t _rings = 16, float _radius = .5f);

    static Mesh GenerateGrid(const glm::ivec2 _size, float _spacing = 1);
    static Mesh GenerateWireSphere(uint16_t _resolution, float _radius);
    static Mesh GenerateWireCube(const glm::vec3 _size = glm::vec3(1));
    static Mesh GenerateWireCone(uint16_t _baseResolution = 16, uint16_t _apexResolution = 4, float _degrees = 45, float _length = 1);
  };
}
