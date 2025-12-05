#include <Renderer/Line.hpp>


GLuint Line::VAO;
GLuint Line::VBO;


Line::Line() {}

void Line::update() {}
void Line::draw() const {
  const unsigned int indexC = indecies.size();
  std::vector<float> bufferData((3+4) * indexC);

  for(int i = 0; i < indexC; i++) {
    const unsigned int start = (3+4) * i;
    const unsigned int tri = indecies[i];

    const Vec3 vert = vertices[tri];
    const Color clr = colors[i];

    bufferData[start+0] = vert.x;
    bufferData[start+1] = vert.y;
    bufferData[start+2] = vert.z;

    bufferData[start+3] = clr.r * 0.003921569f;
    bufferData[start+4] = clr.g * 0.003921569f;
    bufferData[start+5] = clr.b * 0.003921569f;
    bufferData[start+6] = clr.a * 0.003921569f;
  }


  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, bufferData.size() * sizeof(float), bufferData.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(VAO);

  glDrawArrays(GL_LINES, 0, indecies.size());
}