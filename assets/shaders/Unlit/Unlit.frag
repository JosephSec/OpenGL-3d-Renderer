#version 330 core
in vec4 vColor;
in vec2 vUV;

out vec4 FragColor;


struct Material {
  sampler2D diffuseTex;
  bool useDiffuseTex;

  vec3 baseColor;
};

uniform Material material;


void main() {
  vec4 texColor = vec4(1.0);

  if(material.useDiffuseTex) texColor = texture(material.diffuseTex, vUV);

  FragColor = texColor * vec4(material.baseColor * vColor.rgb, vColor.a);
}