#version 450 core
in vec3 vWorldPos;

uniform vec3  uLightPos;
uniform float uShadowFar;


void main() {
  gl_FragDepth = length(vWorldPos - uLightPos) / uShadowFar;
}