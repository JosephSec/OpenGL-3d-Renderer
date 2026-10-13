#version 430 core
layout (location = 0) in vec3 aPos;

uniform mat4 uViewRot;
uniform mat4 uProjection;

out vec3 vDir;


void main() {
  vDir = aPos;
  gl_Position = uProjection * uViewRot * vec4(aPos, 1);
}