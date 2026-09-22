#version 330 core
in vec3 vNormal;
in vec4 vColor;
in vec3 vPos;

out vec4 FragColor;


struct Light {
  vec4 position; // pos: xyz | radius: w
  vec4 color;    // rgb: xyz | strength: w
};
struct Material {
  vec3 baseColor;
  vec2 surface;
};

uniform vec3 viewPoint;

uniform Material material;

uniform vec4 ambientLight;
uniform int lightCount;
const int MAX_LIGHTS = 10;
uniform Light lights[MAX_LIGHTS];


float GetLightAttenuation(Light light, vec3 lightDir) {
  float dist = length(lightDir);
  return 1 - min(dist, light.position.w) / light.position.w;
}
vec3 CalculateAmbient() {
  return ambientLight.rgb * ambientLight.a;
}
vec3 CalculateDiffuse(Light light, vec3 normal, vec3 lightDirection, float attenuation) {
  float diffuse = max(dot(normal, lightDirection), 0);
  return light.color.rgb * diffuse * attenuation;
}
vec3 CalculateSpecular(Light light, vec3 normal, vec3 lightDirection, float attenuation, float roughness, float metallic, vec3 albedo) {
  if(dot(normal, lightDirection) <= 0) return vec3(0);

  vec3 viewDir = normalize(viewPoint - vPos);
  vec3 reflectDir = reflect(-lightDirection, normal);  

  float shininess = mix(1024, 1, roughness);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

  vec3 reflectivity = vec3(.04);
  vec3 specularTint = mix(reflectivity, albedo, metallic);

  float roughnessDimming = 1 - roughness;
  float specularStrength = 1 * roughnessDimming;

  return specularStrength * spec * (light.color.rgb * specularTint) * attenuation;
}
void main() {
  vec3 albedo = material.baseColor * vColor.rgb;
  float roughness = material.surface.x;
  float metallic = material.surface.y;

  vec3 normal = normalize(vNormal);
  vec3 totalLight = CalculateAmbient() * albedo;

  for(int i = 0; i < lightCount; i++) {
    vec3 direction = lights[i].position.xyz - vPos;
    float attenuation = GetLightAttenuation(lights[i], direction);
    
    if(attenuation <= 0) continue;

    vec3 normDirection = normalize(direction);

    vec3 diffuse = CalculateDiffuse(lights[i], normal, normDirection, attenuation) * (1 - metallic);
    vec3 specular = CalculateSpecular(lights[i], normal, normDirection, attenuation, roughness, metallic, albedo);

    totalLight += (diffuse + specular) * lights[i].color.w;
  }

  FragColor = vec4(clamp(vColor.rgb * totalLight, 0,1), vColor.a);
}