#version 330 core
in vec3 vNormal;
in vec4 vColor;
out vec4 FragColor;

void main() {
  FragColor = vColor;
}