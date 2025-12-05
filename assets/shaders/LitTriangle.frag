#version 330 core
in vec3 vNormal;
in vec4 vColor;
in vec3 position;

out vec4 FragColor;


struct Light {
  vec3 position;
  vec3 color;
  float range;
};

uniform int lightCount;
const int MAX_LIGHTS = 10;
uniform Light lights[MAX_LIGHTS];


void main() {
  vec3 totalLight = vec3(0,0,0);

  for(int i = 0; i < lightCount; i++) {
    vec3 lightDir = lights[i].position - position;
    float strength = 1-length(lightDir)/lights[i].range;
    lightDir = normalize(lightDir);

    float dif = max(dot(vNormal, lightDir) * strength, 0);
    totalLight = clamp(totalLight + dif * lights[i].color, 0, 1);
  }

  FragColor = vec4(vColor.rgb * totalLight, vColor.a);
}