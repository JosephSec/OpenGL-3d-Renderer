#include <Renderer/Mesh.hpp>
#include <System.hpp>
#include <User.hpp>
#include <Renderer.hpp>

#include <filesystem>
namespace filesystem = std::filesystem;


GLuint Mesh::VAO;
GLuint Mesh::VBO;

Mesh Mesh::Triangle;
Mesh Mesh::Circle;
Mesh Mesh::Pyramid;
Mesh Mesh::Quad;
Mesh Mesh::Cube;


static inline uint64_t SerializeVertex(const uint32_t index, const Color& color) {
  return ((uint64_t)index << 32) | color.toInteger();
}


void Mesh::InitPrimitives() {
  if(!LoadPrimitive("Triangle", Triangle)) Debug::error("failed to load Triangle Primitive\n\n");
  if(!LoadPrimitive("Circle", Circle))     Debug::error("failed to load Circle Primitive\n\n");
  if(!LoadPrimitive("Pyramid", Pyramid))   Debug::error("failed to load Pyramid Primitive\n\n");
  if(!LoadPrimitive("Quad", Quad))         Debug::error("failed to load Quad Primitive\n\n");
  if(!LoadPrimitive("Cube", Cube))         Debug::error("failed to load Cube Primitive\n\n");

  // SaveAsPrimitive("Triangle", Triangle);
  // SaveAsPrimitive("Circle", Circle);
  // SaveAsPrimitive("Pyramid", Pyramid);
  // SaveAsPrimitive("Quad", Quad);
  // SaveAsPrimitive("Cube", Cube);
}
void Mesh::SaveAsPrimitive(const string& name, const Mesh& mesh) {
  const string folder = System::PATH+"/assets/MeshPrimitives";
  if(!filesystem::exists(folder)) filesystem::create_directories(folder);

  std::ofstream file(folder+"/"+name+".bin", std::ios::binary);

  uint16_t vertC = mesh.vertices.size();
  file.write(reinterpret_cast<char*>(&vertC), sizeof(vertC));
  for(int i = 0; i < vertC; i++) {
    const Vec3& vert = mesh.vertices[i];
    float value = vert.x;
    file.write(reinterpret_cast<char*>(&value), sizeof(value));
    value = vert.y;
    file.write(reinterpret_cast<char*>(&value), sizeof(value));
    value = vert.z;
    file.write(reinterpret_cast<char*>(&value), sizeof(value));
  }
  file << "\n";

  uint16_t indexC = mesh.indecies.size();
  file.write(reinterpret_cast<char*>(&indexC), sizeof(indexC));
  for(int i = 0; i < indexC; i++) {
    uint64_t value = SerializeVertex(mesh.indecies[i], mesh.colors[i]);
    file.write(reinterpret_cast<char*>(&value), sizeof(value));
  }
  file.close();
}
template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
ReadBytes(const string& str, unsigned int start, unsigned int size) {
  T val = 0;
  for(int i = 0; i < size; i++) val |= ((unsigned char)str[start + i] << (i * 8));
  return val;
}
bool Mesh::LoadPrimitive(const string& name, Mesh& mesh) {
  mesh = Mesh();

  const string fileName = "MeshPrimitives/"+name+".bin";
  std::ifstream file(System::PATH+"/assets/"+fileName, std::ios::binary);
  if(!file.is_open()) {
    Debug::error(fileName, Debug::Error::File);
    return false;
  } else System::assets.push_back(fileName);

  std::vector<string> lines;
  string str;
  while(std::getline(file, str)) lines.push_back(str);
  file.close();
  
  { //vertices
    std::vector<float> data;
    const unsigned int vertC = ReadBytes<uint16_t>(lines[0], 0, sizeof(uint16_t));
    for(int i = 0; i < vertC*3; i++) {
      const uint32_t value = ReadBytes<uint32_t>(lines[0], i*sizeof(uint32_t)+sizeof(uint16_t), sizeof(uint32_t));
      data.push_back(0);
      std::memcpy(&data.back(), &value, sizeof(float));
    }

    for(int i = 0; i < data.size()/3; i++) {
      const unsigned int start = i*3;
      mesh.vertices.push_back(Vec3(data[start], data[start+1], data[start+2]));
    }
  }
  { //indecies/colors
    string vertDataStr;
    for(int i = 1; i < lines.size(); i++) {
      vertDataStr += lines[i] + "\n";
    }
    
    std::vector<uint32_t> data;
    const unsigned int indexC = ReadBytes<uint16_t>(vertDataStr, 0, sizeof(uint16_t));
    for(int i = 0; i < indexC*2; i++) {
      const uint32_t value = ReadBytes<uint32_t>(vertDataStr, i*sizeof(uint32_t)+sizeof(uint16_t), sizeof(uint32_t));
      if((i % 2) == 0) mesh.colors.push_back(Color(value));
      else mesh.indecies.push_back(value);
    }
  }

  return mesh.vertices.size() > 0 && mesh.indecies.size() > 0 && mesh.colors.size() > 0;
}


Mesh::Mesh() {}

void Mesh::update() {}
void Mesh::draw() const {
  const unsigned int indexC = indecies.size();
  std::vector<float> bufferData((3+3+4) * indexC);

  for(int face = 0; face < indexC/3; face++) {
    const unsigned int* faceStart = indecies.data() + face*3;
    const Vec3& a = vertices[*(faceStart)];
    const Vec3& b = vertices[*(faceStart+1)];
    const Vec3& c = vertices[*(faceStart+2)];

    const Vec3 normal = CalculateNormal(a,b,c);

    for(int i = 0; i < 3; i++) {
      const unsigned int start = (3+3+4) * (face*3 + i) + 3;// * i + 3(vert pos offset)
      bufferData[start+0] = normal.x;
      bufferData[start+1] = normal.y;
      bufferData[start+2] = normal.z;
    }
  }

  for(int i = 0; i < indexC; i++) {
    const unsigned int start = (3+3+4) * i;
    const unsigned int tri = indecies[i];

    const Vec3 vert = vertices[tri];
    const Color clr = colors[i];

    bufferData[start+0] = vert.x;
    bufferData[start+1] = vert.y;
    bufferData[start+2] = vert.z;

    bufferData[start+6] = clr.r * 0.003921569f;
    bufferData[start+7] = clr.g * 0.003921569f;
    bufferData[start+8] = clr.b * 0.003921569f;
    bufferData[start+9] = clr.a * 0.003921569f;
  }


  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, bufferData.size() * sizeof(float), bufferData.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(VAO);

  glDrawArrays(GL_TRIANGLES, 0, indexC);
}