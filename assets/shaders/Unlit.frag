#version 330 core
in vec4 vColor;
in vec2 vUV;

uniform sampler2D tex;
uniform bool useTex;

out vec4 FragColor;

void main() {
  FragColor = (useTex? texture(tex, vUV) : vec4(1)) * vColor;
}