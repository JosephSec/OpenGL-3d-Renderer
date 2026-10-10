#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aUV;

layout (location = 4) in mat4 aModel;

out vec4 vColor;
out vec3 vNormal;
out vec2 vUV;

uniform mat4 uProjection;
uniform mat4 uView;
uniform mat4 uModel;
uniform bool uIsInstanced;


void main() {
  vColor  = aColor;
  vNormal = aNormal;
  vUV     = aUV;

  mat4 model = uModel;
  if(uIsInstanced == true) model = aModel;

  gl_Position = uProjection * uView * model * vec4(aPos, 1);
}