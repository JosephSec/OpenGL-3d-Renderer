#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;

// out vec3 vNormal;
out vec4 vColor;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
  // vNormal = aNormal;
  vColor = aColor;

  gl_Position = projection * view * model * vec4(aPos, 1.0);
  // gl_Position = vec4(aPos, 1.0);
}