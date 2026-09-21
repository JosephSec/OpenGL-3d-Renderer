#version 330 core
in vec3 vNormal;
in vec4 vColor;
in vec3 vPos;

out vec4 FragColor;


struct Light {
  vec3 position;
  vec3 color;
  float radius;
};

uniform vec4 ambientLight;
uniform int lightCount;
const int MAX_LIGHTS = 10;
uniform Light lights[MAX_LIGHTS];


void main() {
  vec3 normal = normalize(vNormal);
  vec3 totalLight = ambientLight.rgb * ambientLight.a;

  for(int i = 0; i < lightCount; i++) {
    vec3 lightDir = lights[i].position - vPos;
    float dist = length(lightDir);
    float strength = 1 - min(dist, lights[i].radius) / lights[i].radius;

    float diffuse = max(dot(normal, lightDir), 0);
    totalLight += lights[i].color * diffuse * strength;
  }
  
  FragColor = vec4(clamp(vColor.rgb * totalLight, 0,1), vColor.a);
}