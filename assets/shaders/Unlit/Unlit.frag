#version 330 core
in vec4 vColor;

out vec4 FragColor;


struct Material {
  vec3 baseColor;
};

uniform Material material;


void main() {
  FragColor = vec4(material.baseColor * vColor.rgb, vColor.a);
}