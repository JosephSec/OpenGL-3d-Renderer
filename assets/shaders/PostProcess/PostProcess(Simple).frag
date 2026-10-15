#version 430 core
in vec2 vUV;

uniform vec2      uViewSize;
uniform sampler2D uColor;
uniform sampler2D uDepth;
uniform float     uTime;
uniform mat4      uView;
uniform mat4      uProj;
uniform vec3      uCameraWorldPos;

out vec4 fragColor;


void main() {
  vec3 color = texture(uColor, vUV).rgb;
  fragColor = vec4(color, 1);
}