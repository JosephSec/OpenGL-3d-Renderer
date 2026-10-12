#include <Renderer/Pipeline/Skybox.hpp>
using namespace Renderer;

#include <Renderer/Core.hpp>
#include <Renderer/Component/Transform.hpp>


Skybox::Skybox(int _size, GLenum _internalFormat, int _levels) : m_cubemap(_size, _internalFormat, _levels) {
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