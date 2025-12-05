#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aColor;

out vec3 vNormal;
out vec4 vColor;
out vec3 position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normal;

void main() {
  vNormal = normalize(normal * aNormal);
  vColor = aColor;
  position = model[3].xyz;

  gl_Position = projection * view * model * vec4(aPos, 1.0);
}