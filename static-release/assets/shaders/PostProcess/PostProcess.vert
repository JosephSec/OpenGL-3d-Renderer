#version 430 core

out vec2 vUV;
out vec2 vLocalSpaceCoords; // NEW OUT: Passes center-zero tracking coordinates down to pixels

void main() {
  // Your exact working clockwise screen-covering triangle calculation
  vec2 pos = vec2(gl_VertexID & 2, (gl_VertexID << 1) & 2);  
  vUV = pos;

  // Derive a perfect [-1.0, 1.0] center-zero local coordinate space for the optics calculations
  vLocalSpaceCoords = pos * 2.0 - 1.0;

  gl_Position = vec4(pos * 2.0 - 1.0, 0, 1);
}