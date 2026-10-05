#version 330 core
in vec3 vNormal;
in vec4 vColor;
in vec3 vPos;
in vec2 vUV;

out vec4 FragColor;


#define TYPE_POINT 0
#define TYPE_DIRECTIONAL 1
#define TYPE_SPOT 2

struct Light {
  int type; //0 - point | 1 - directional | 2 - spot

  vec3 position;
  vec3 direction;

  vec4 color;    // rgb: xyz | strength: w
  float radius;

  float innerCutOff;
  float outerCutOff;
};
struct Material {
  sampler2D diffuseTex;
  bool useDiffuseTex;

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
  if(light.type == TYPE_DIRECTIONAL) return 1.0; //directional lights give max attenuation

  float dist = length(lightDir);
  return 1 - min(dist, light.radius) / light.radius;
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
  if(gl_FrontFacing == false) normal = -normal;

  vec3 totalLight = CalculateAmbient() * albedo;

  for(int i = 0; i < lightCount; i++) {
    Light light = lights[i];

    vec3 direction;
    if(light.type == TYPE_DIRECTIONAL) direction = -normalize(light.direction);
    else direction = light.position.xyz - vPos;

    float attenuation = GetLightAttenuation(light, direction);    
    if(attenuation <= 0) continue;

    vec3 normDirection = (light.type == TYPE_DIRECTIONAL)? direction : normalize(direction);

    if(light.type == TYPE_SPOT) {
      float theta = dot(-normDirection, normalize(light.direction));
      float epsilon = light.innerCutOff - light.outerCutOff;

      float spotIntensity = clamp((theta - light.outerCutOff) / epsilon, 0.0,1.0);
      attenuation *= spotIntensity;

      if(attenuation <= 0) continue;
    }

    vec3 diffuse = CalculateDiffuse(light, normal, normDirection, attenuation) * (1 - metallic);
    vec3 specular = CalculateSpecular(light, normal, normDirection, attenuation, roughness, metallic, albedo);

    totalLight += (diffuse + specular) * lights[i].color.w;
  }

  vec4 texColor = vec4(1);
  if(material.useDiffuseTex) texColor = texture(material.diffuseTex, vUV);

  vec4 finalColor = texColor * vec4(clamp(vColor.rgb * totalLight, 0,1), vColor.a);

  finalColor.rgb = pow(finalColor.rgb, vec3(1.0/2.2));

  FragColor = finalColor;
}