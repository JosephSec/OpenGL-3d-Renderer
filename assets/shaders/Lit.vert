#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aColor;
layout (location = 3) in vec2 aUV;

out vec3 vNormal;
out vec4 vColor;
out vec3 vPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normal;

void main() {
  vec4 worldPos = model * vec4(aPos, 1.0);

  vPos = worldPos.xyz;
  vNormal = normalize(normal * aNormal);
  vColor = aColor;

  gl_Position = projection * view * worldPos;
}