#version 330 core
in vec3 vNormal;
in vec4 vColor;
in vec3 vPos;

out vec4 FragColor;


struct Light {
  vec3 position;
  vec4 color;
  float radius;
};

uniform vec4 ambientLight;
uniform int lightCount;
const int MAX_LIGHTS = 10;
uniform Light lights[MAX_LIGHTS];


void main() {
  vec3 normal = normalize(vNormal);
  vec3 totalLight = vec3(0);

  for(int i = 0; i < lightCount; i++) {
    vec3 lightDir = lights[i].position - vPos;
    float dist = length(lightDir);
    float strength = (1.0 - min(dist, lights[i].radius) / lights[i].radius) * lights[i].color.a;

    if(dist > 1e-4) lightDir /= dist;
    else lightDir = vec3(0,1,0);

    float diffuse = max(dot(normal, lightDir), 0.0);
    totalLight += diffuse * strength * lights[i].color.rgb;
  }

  vec3 finalLightVal = totalLight + ambientLight.rgb * ambientLight.a;
  FragColor = vec4(clamp(vColor.rgb * finalLightVal, 0.0,1.0), vColor.a);
}