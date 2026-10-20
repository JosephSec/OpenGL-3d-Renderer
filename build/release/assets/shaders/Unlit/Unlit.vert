#version 430 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec4 aColor;

out vec4 vColor;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;


void main() {
  vColor = aColor;
  gl_Position = uProj * uView * uModel * vec4(aPosition, 1);
}