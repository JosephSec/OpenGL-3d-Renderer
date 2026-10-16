#version 450 core
layout(location = 0) in vec3 aPos;

uniform mat4 uModel;
uniform mat4 uLightSpace;

out vec3 vWorldPos;


void main() {
  vec4 w = uModel * vec4(aPos, 1.0);
  vWorldPos = w.xyz;
  gl_Position = uLightSpace * w;
}