#include <Renderer/Pipeline/Skybox.hpp>
using namespace Renderer;

#include <Renderer/Core.hpp>
#include <Renderer/Component/Transform.hpp>

#include <iostream>


bool Skybox::SplitToFaces(const std::filesystem::path &_file) {
  // GL cubemap face order: +X, -X, +Y, -Y, +Z, -Z
  static constexpr std::array<const char*, 6> names = {"right", "left", "top", "bottom", "front", "back"};

  // Refuse to overwrite the source if it is itself named like a face
  const std::string stem = _file.stem().string();
  for(const char *n : names) {
    if(stem == n) {
      std::cout << "[SKYBOX ERROR]: rename the source file, '" << stem << "' would be overwritten\n";
      return false;
    }
  }

  sf::Image src;
  if(!src.loadFromFile(_file)) {
    std::cout << "[SKYBOX ERROR]: failed to load " << _file << "\n";
    return false;
  }

  const int W = (int)src.getSize().x, H = (int)src.getSize().y;
  std::array<sf::Image, 6> faces;
  int size = 0;

  struct Cell { int col, row; bool rot180; };

  // Copies one grid cell into a face image
  auto cutCell = [&](int f, Cell c, int s) {
    faces[f] = sf::Image(sf::Vector2u{(unsigned)s, (unsigned)s}, sf::Color::Black);
    for(int y = 0; y < s; y++) {
      for(int x = 0; x < s; x++) {
        const int sx = c.col * s + (c.rot180 ? s - 1 - x : x);
        const int sy = c.row * s + (c.rot180 ? s - 1 - y : y);
        faces[f].setPixel({(unsigned)x, (unsigned)y}, src.getPixel({(unsigned)sx, (unsigned)sy}));
      }
    }
  };

  if(W * 3 == H * 4) {            // horizontal cross
    size = W / 4;
    const Cell cells[6] = {{2,1,false}, {0,1,false}, {1,0,false}, {1,2,false}, {1,1,false}, {3,1,false}};
    for(int f = 0; f < 6; f++) cutCell(f, cells[f], size);
  }
  else if(W * 4 == H * 3) {       // vertical cross
    size = W / 3;
    const Cell cells[6] = {{2,1,false}, {0,1,false}, {1,0,false}, {1,2,false}, {1,1,false}, {1,3,true}};
    for(int f = 0; f < 6; f++) cutCell(f, cells[f], size);
  }
  else if(W == H * 6) {           // horizontal strip
    size = H;
    for(int f = 0; f < 6; f++) cutCell(f, {f, 0, false}, size);
  }
  else if(H == W * 6) {           // vertical strip
    size = W;
    for(int f = 0; f < 6; f++) cutCell(f, {0, f, false}, size);
  }
  else if(W == H * 2) {           // equirectangular
    size = H / 2;

    // Bilinear sample: wraps horizontally, clamps vertically
    auto sample = [&](float fx, float fy) {
      fx -= 0.5f; fy -= 0.5f;
      const int x0 = (int)std::floor(fx), y0 = (int)std::floor(fy);
      const float tx = fx - x0, ty = fy - y0;
      auto px = [&](int x, int y) {
        x = ((x % W) + W) % W;
        y = std::clamp(y, 0, H - 1);
        return src.getPixel({(unsigned)x, (unsigned)y});
      };
      const sf::Color c00 = px(x0, y0),     c10 = px(x0 + 1, y0);
      const sf::Color c01 = px(x0, y0 + 1), c11 = px(x0 + 1, y0 + 1);
      auto mix = [&](std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d) {
        const float top = a + (b - a) * tx, bot = c + (d - c) * tx;
        return (std::uint8_t)std::lround(top + (bot - top) * ty);
      };
      return sf::Color(mix(c00.r, c10.r, c01.r, c11.r), mix(c00.g, c10.g, c01.g, c11.g),
                       mix(c00.b, c10.b, c01.b, c11.b), mix(c00.a, c10.a, c01.a, c11.a));
    };

    constexpr float PI = 3.14159265359f;
    for(int f = 0; f < 6; f++) {
      faces[f] = sf::Image(sf::Vector2u{(unsigned)size, (unsigned)size}, sf::Color::Black);
      for(int y = 0; y < size; y++) {
        for(int x = 0; x < size; x++) {
          const float u = 2.0f * (x + 0.5f) / size - 1.0f;   // -1..1, right
          const float v = 2.0f * (y + 0.5f) / size - 1.0f;   // -1..1, down

          // Direction for each face, same convention as the GL cubemap spec
          float dx, dy, dz;
          switch(f) {
            case 0:  dx =  1; dy = -v; dz = -u; break;   // +X
            case 1:  dx = -1; dy = -v; dz =  u; break;   // -X
            case 2:  dx =  u; dy =  1; dz =  v; break;   // +Y
            case 3:  dx =  u; dy = -1; dz = -v; break;   // -Y
            case 4:  dx =  u; dy = -v; dz =  1; break;   // +Z
            default: dx = -u; dy = -v; dz = -1; break;   // -Z
          }

          const float len = std::sqrt(dx*dx + dy*dy + dz*dz);
          const float lon = std::atan2(dx, -dz);               // 0 = image centre = -Z
          const float lat = std::asin(dy / len);
          const float fx = (lon / (2.0f * PI) + 0.5f) * W;
          const float fy = (0.5f - lat / PI) * H;
          faces[f].setPixel({(unsigned)x, (unsigned)y}, sample(fx, fy));
        }
      }
    }
  }
  else {
    std::cout << "[SKYBOX ERROR]: unsupported layout " << W << "x" << H
              << " (expected 4:3 cross, 3:4 cross, 6:1 or 1:6 strip, or 2:1 equirectangular)\n";
    return false;
  }

  // Save next to the source file
  const std::filesystem::path outDir = _file.parent_path();
  for(int f = 0; f < 6; f++) {
    const std::filesystem::path out = outDir / (std::string(names[f]) + ".png");
    if(!faces[f].saveToFile(out)) {
      std::cout << "[SKYBOX ERROR]: failed to write " << out << "\n";
      return false;
    }
  }
  return true;
}


Skybox::Skybox(int _size, GLenum _internalFormat, int _levels)
: m_cubemap(_size, _internalFormat, _levels), m_size(_size), m_internalFormat(_internalFormat), m_levels(_levels) {
  const std::vector<glm::vec3> faceColors = {
    {1,0,0}, {0,0,0}, // +X red,   -X black
    {0,1,0}, {0,0,0}, // +Y green, -Y black
    {0,0,1}, {0,0,0}, // +Z blue,  -Z black
  };

  std::vector<float> px(_size * _size * 3);
  for(int f = 0; f < 6; f++) {
    for(int i = 0; i < _size * _size; i++) {
      const int start = i * 3;

      px[start + 0] = faceColors[f].r;
      px[start + 1] = faceColors[f].g;
      px[start + 2] = faceColors[f].b;
    }

    m_cubemap.uploadFace(f, px.data(), GL_RGB, GL_FLOAT);
  }
  { //Create VAO
    glCreateVertexArrays(1, &m_vao);

    const glm::vec3 vertices[8] = {
      {-0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f},
      { 0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f},
      {-0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f},
      { 0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f},
    };
    const GLubyte indices[36] = {
      1,2,6, 1,6,5, //x+
      4,7,3, 4,3,0, //x-
      3,7,6, 3,6,2, //y+
      0,1,5, 0,5,4, //y-
      4,5,6, 4,6,7, //z+
      1,0,3, 1,3,2, //z-
    };

    GLuint vbo, ebo;
    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, sizeof(vertices), vertices, 0);
    glCreateBuffers(1, &ebo);
    glNamedBufferStorage(ebo, sizeof(indices), indices, 0);

    glVertexArrayVertexBuffer(m_vao, 0, vbo, 0, sizeof(glm::vec3));
    glVertexArrayElementBuffer(m_vao, ebo);

    glEnableVertexArrayAttrib(m_vao, 0);
    glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(m_vao, 0, 0);
  } //Create VAO
}
Skybox::~Skybox() {
  if(m_vao != 0) glDeleteVertexArrays(1, &m_vao);
}

void Skybox::draw(const Shader *_shader) const noexcept {  
  _shader->use();
  _shader->setUniform("uViewRot", Renderer::Transform({0,0,0}, Renderer::Core::camera->transform.rotation, {1,1,1}).getViewMatrix());
  _shader->setUniform("uProjection", Renderer::Core::camera->getProjectionMatrix(Renderer::Core::windowSize));
  _shader->setUniform("uSky", 0);

  m_cubemap.bind(0);
  glBindVertexArray(m_vao);

  glDepthMask(GL_FALSE);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, nullptr);
  glDepthMask(GL_TRUE);
}

bool Skybox::loadFromFolder(const std::filesystem::path &_folder) {
  // GL cubemap face order: +X, -X, +Y, -Y, +Z, -Z
  static constexpr std::array<const char*, 6> names = {"right", "left", "top", "bottom", "front", "back"};
  static constexpr std::array<const char*, 4> exts  = {".png", ".jpg", ".jpeg", ".bmp"};

  std::array<sf::Image, 6> images;
  unsigned int size = 0;

  // Load and validate everything first, so a failure leaves the current skybox untouched
  for(int f = 0; f < 6; f++) {
    bool loaded = false;
    for(const char *ext : exts) {
      const std::filesystem::path file = _folder / (std::string(names[f]) + ext);
      if(!std::filesystem::exists(file)) continue;

      if(images[f].loadFromFile(file)) { loaded = true; break; }
      std::cout << "[SKYBOX ERROR]: failed to decode " << file << "\n";
      return false;
    }
    if(!loaded) {
      std::cout << "[SKYBOX ERROR]: missing face '" << names[f] << "' in " << _folder << "\n";
      return false;
    }

    const sf::Vector2u s = images[f].getSize();
    if(s.x != s.y) {
      std::cout << "[SKYBOX ERROR]: face '" << names[f] << "' is not square (" << s.x << "x" << s.y << ")\n";
      return false;
    }
    if(f == 0) size = s.x;
    else if(s.x != size) {
      std::cout << "[SKYBOX ERROR]: face '" << names[f] << "' is " << s.x << "px, expected " << size << "px\n";
      return false;
    }
  }

  // Recreate the cubemap if the size differs from the placeholder
  if((int)size != m_size) {
    m_cubemap = Cubemap((int)size, m_internalFormat, m_levels);
    m_size = (int)size;
  }

  for(int f = 0; f < 6; f++) {
    // No vertical flip: standard skybox face images are already in cubemap orientation
    m_cubemap.uploadFace(f, images[f].getPixelsPtr(), GL_RGBA, GL_UNSIGNED_BYTE);
  }

  if(m_levels > 1) m_cubemap.generateMipmaps();

  return true;
}
