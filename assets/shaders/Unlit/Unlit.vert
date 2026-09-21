#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aColor;
layout (location = 3) in vec2 aUV;

layout (location = 4) in vec4 aModel1;
layout (location = 5) in vec4 aModel2;
layout (location = 6) in vec4 aModel3;
layout (location = 7) in vec4 aModel4;

out vec4 vColor;

uniform mat4 uProjection;
uniform mat4 uView;
uniform mat4 uModel;
uniform bool uIsInstanced;


void main() {
  mat4 model = uModel;
  if(uIsInstanced) model = mat4(aModel1,aModel2,aModel3,aModel4);
  
  vColor = aColor;

  gl_Position = uProjection * uView * model * vec4(aPos, 1.0);
}