#include <Mesh.hpp>

#include <cstring>


Mesh::Mesh(MeshType _type) : type(_type) {
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);
  
  glBindBuffer(GL_ARRAY_BUFFER, vbo);  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (void*)(0 * sizeof(float)));
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::draw() const {
  const unsigned int indexC = indeces.size();
  std::vector<float> bufferData((3+4) * indexC);

  for(int i = 0; i < indexC; i++) {
    const unsigned int start = (3+4) * i;
    const unsigned int tri = indeces[i];

    const glm::vec3 vert = vertices[tri].position;
    const glm::vec4 clr = vertices[i].color;

    bufferData[start+0] = vert.x;
    bufferData[start+1] = vert.y;
    bufferData[start+2] = vert.z;

    bufferData[start+3] = clr.r;
    bufferData[start+4] = clr.g;
    bufferData[start+5] = clr.b;
    bufferData[start+6] = clr.a;
  }


  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, bufferData.size() * sizeof(float), bufferData.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(vao);

  glDrawArrays(GL_TRIANGLES, 0, indexC);
}
// void Mesh::draw() const {
//   glBindVertexArray(vao);

//   glBindBuffer(GL_ARRAY_BUFFER, vbo);
//   glBufferData(GL_ARRAY_BUFFER, vertices.size() * VERTEX_SIZE, vertices.data(), GL_STATIC_DRAW);

//   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
//   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indeces.size() * sizeof(unsigned int), indeces.data(), GL_STATIC_DRAW);

//   glDrawElements(GL_TRIANGLES, indeces.size(), GL_UNSIGNED_INT, 0);
//   glBindVertexArray(0);
// }