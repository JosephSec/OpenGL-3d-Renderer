#version 430 core

out vec2 vUV;
out vec2 vLocalSpaceCoords;

void main() {
  vec2 pos = vec2(gl_VertexID & 2, (gl_VertexID << 1) & 2);  
  vUV = pos;

  vLocalSpaceCoords = pos * 2.0 - 1.0;

  gl_Position = vec4(pos * 2.0 - 1.0, 0, 1);
}