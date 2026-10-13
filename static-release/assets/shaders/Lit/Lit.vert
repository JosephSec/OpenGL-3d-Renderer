#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aUV;

layout (location = 4) in mat4 aModel;


out vec3 vNormal;
out vec4 vColor;
out vec3 vPos;
out vec2 vUV;

uniform mat4 uProjection;
uniform mat4 uView;
uniform mat4 uModel;
uniform bool uIsInstanced;


void main() {  
  mat4 model = uModel;
  if(uIsInstanced) model = aModel;
  
  vec4 worldPos = model * vec4(aPos, 1.0);

  vPos = worldPos.xyz;
  vColor = aColor;
  vNormal = normalize(mat3(transpose(inverse(model))) * aNormal);
  vUV = aUV;

  gl_Position = uProjection * uView * worldPos;
}