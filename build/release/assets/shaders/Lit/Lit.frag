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

uniform vec3 uViewPoint;

uniform Material uMaterial;

uniform vec4 uAmbientLight;
uniform int uLightCount;
const int MAX_LIGHTS = 10;
uniform Light lights[MAX_LIGHTS];

// ---- Point light shadow (applies to lights[0]) ----
uniform samplerCube uShadowCube;
uniform vec3  uShadowLightPos;
uniform float uShadowFar;
uniform bool  uHasShadow;     // false = no shadow pass ran this frame


// =====================================================================
//  TUNING (compile-time constants, not uniforms)
// =====================================================================
const float LIGHT_UNIT          = 4.0;    // overall brightness of every light
const float MIN_ROUGHNESS       = 0.045;  // avoids an infinitely sharp highlight
const float ATTEN_SOFTNESS      = 1.0;    // inverse-square softening, higher = flatter close to the light
const float AMBIENT_GROUND      = 0.45;   // ambient brightness facing straight down (1.0 = straight up)

// specular anti-aliasing (stops shimmering highlights on curved or low-poly surfaces)
const bool  SPEC_AA             = true;
const float SPEC_AA_VARIANCE    = 0.15;
const float SPEC_AA_THRESHOLD   = 0.10;

// shadows
const bool  SHADOW_SOFT         = true;   // true = contact-hardening penumbra, false = fixed-width filter
const float SHADOW_LIGHT_SIZE   = 0.12;   // world-space radius of the light, bigger = softer shadows
const float SHADOW_MIN_FILTER   = 1.25;   // smallest filter radius, in shadow texels
const float SHADOW_MAX_FILTER   = 10.0;   // largest filter radius, in shadow texels (caps noise and cost)
const float SHADOW_FIXED_FILTER = 2.0;    // filter radius in texels when SHADOW_SOFT = false
const float SHADOW_NORMAL_OFFSET = 1.5;   // receiver offset along the normal, in texels
const float SHADOW_BIAS         = 1.0;    // constant depth bias, in texels
const float SHADOW_MAX_SLOPE    = 2.0;    // cap on the slope-scaled bias
const int   SHADOW_TAPS         = 16;     // final filter samples
const int   SHADOW_BLOCKER_TAPS = 8;      // blocker search samples (SHADOW_SOFT only)

// ---- output stage ----
// For the current 8-bit pipeline leave these as they are.
// If you move to a GL_RGBA16F post buffer and tonemap/gamma in the post shader, set
// TONEMAP = 0, OUTPUT_GAMMA = false and OUTPUT_DITHER = false.
const int   TONEMAP             = 1;      // 0 = none | 1 = hue-preserving ACES | 2 = Khronos PBR Neutral
const bool  OUTPUT_GAMMA        = true;
const bool  OUTPUT_DITHER       = true;

const float PI           = 3.14159265359;
const float GOLDEN_ANGLE = 2.39996323;


// =====================================================================
//  COLOUR / NOISE HELPERS
// =====================================================================
vec3 SRGBToLinear(vec3 c) {
  c = max(c, 0.0);
  vec3 lo = c / 12.92;
  vec3 hi = pow((c + 0.055) / 1.055, vec3(2.4));
  return mix(hi, lo, step(c, vec3(0.04045)));
}
vec3 LinearToSRGB(vec3 c) {
  c = max(c, 0.0);
  vec3 lo = c * 12.92;
  vec3 hi = 1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055;
  return mix(hi, lo, step(c, vec3(0.0031308)));
}

// Interleaved gradient noise (Jimenez), 0..1, stable per pixel
float InterleavedGradientNoise(vec2 p) {
  return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715))));
}

// Narkowicz ACES approximation
vec3 ACESFilm(vec3 x) {
  const float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}
// ACES on the brightest channel so hues don't skew, easing back to per-channel
// (which desaturates toward white) only for very bright highlights
vec3 ToneMapACES(vec3 c) {
  float peak = max(c.r, max(c.g, c.b));
  if(peak <= 1e-5) return vec3(0.0);

  vec3 hueKept    = c * (ACESFilm(vec3(peak)).x / peak);
  vec3 perChannel = ACESFilm(c);
  return mix(hueKept, perChannel, smoothstep(1.0, 8.0, peak));
}
// Khronos PBR Neutral: leaves most colours untouched, only compresses the top end
vec3 ToneMapNeutral(vec3 color) {
  const float startCompression = 0.8 - 0.04;
  const float desaturation = 0.15;

  float x = min(color.r, min(color.g, color.b));
  float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
  color -= offset;

  float peak = max(color.r, max(color.g, color.b));
  if(peak < startCompression) return color;

  const float d = 1.0 - startCompression;
  float newPeak = 1.0 - d * d / (peak + d - startCompression);
  color *= newPeak / peak;

  float g = 1.0 - 1.0 / (desaturation * (peak - newPeak) + 1.0);
  return mix(color, newPeak * vec3(1.0), g);
}

vec3 EncodeOutput(vec3 c) {
  if(TONEMAP == 1)      c = ToneMapACES(c);
  else if(TONEMAP == 2) c = ToneMapNeutral(c);
  else if(OUTPUT_GAMMA) c = clamp(c, 0.0, 1.0);

  c = max(c, 0.0);
  if(OUTPUT_GAMMA) c = LinearToSRGB(c);

  if(OUTPUT_DITHER) {
    // Triangular-PDF dither, +-1 LSB of an 8-bit target, removes banding in dark gradients
    float n = InterleavedGradientNoise(gl_FragCoord.xy)
            + InterleavedGradientNoise(gl_FragCoord.xy + vec2(5.588238)) - 1.0;
    c += n / 255.0;
  }
  return c;
}


// =====================================================================
//  LIGHT HELPERS
// =====================================================================
// Windowed inverse-square: bright up close, reaches exactly 0 at the radius.
// radius <= 0 means unlimited range.
float GetLightAttenuation(Light light, float dist) {
  if(light.type == TYPE_DIRECTIONAL) return 1.0;

  float falloff = 1.0 / (dist * dist + ATTEN_SOFTNESS);
  if(light.radius <= 0.0) return falloff;

  float ratio = dist / light.radius;
  float r2 = ratio * ratio;
  float window = clamp(1.0 - r2 * r2, 0.0, 1.0);
  return window * window * falloff;
}

// Sky-to-ground gradient built from uAmbientLight. Rougher lookups see a flatter gradient.
float AmbientGradient(vec3 dir, float roughness) {
  float t = dir.y * 0.5 + 0.5;                              // 0 = down, 1 = up
  float g = mix(smoothstep(0.35, 0.65, t), t, roughness);
  return mix(AMBIENT_GROUND, 1.0, g);
}

// GGX / Trowbridge-Reitz, numerically stable form
float D_GGX(float NoH, float alpha) {
  float a2 = alpha * alpha;
  float f = (NoH * a2 - NoH) * NoH + 1.0;
  return a2 / (PI * f * f + 1e-7);
}
// Height-correlated Smith visibility (already includes the 1 / (4 NoV NoL) term)
float V_SmithGGXCorrelated(float NoV, float NoL, float alpha) {
  float a2 = alpha * alpha;
  float ggxV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
  float ggxL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
  return 0.5 / max(ggxV + ggxL, 1e-5);
}
vec3 F_Schlick(float VoH, vec3 F0) {
  float f = 1.0 - VoH;
  float f2 = f * f;
  return F0 + (1.0 - F0) * (f2 * f2 * f);
}

// Analytic environment BRDF (Karis, "Physically Based Shading on Mobile"):
// returns scale (x) and bias (y) so that specular albedo = F0 * x + y
vec2 EnvBRDFApprox(float roughness, float NoV) {
  const vec4 c0 = vec4(-1.0, -0.0275, -0.572,  0.022);
  const vec4 c1 = vec4( 1.0,  0.0425,  1.04,  -0.04);
  vec4 r = roughness * c0 + c1;
  float a004 = min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
  return vec2(-1.04, 1.04) * a004 + r.zw;
}

// Widens roughness where the normal changes quickly across a pixel (Kaplanyan / Tokuyoshi).
// Must be called from uniform control flow because it uses derivatives.
float FilterRoughness(float roughness, vec3 N) {
  vec3 du = dFdx(N);
  vec3 dv = dFdy(N);
  float variance = SPEC_AA_VARIANCE * (dot(du, du) + dot(dv, dv));
  float alpha = roughness * roughness;
  float kernel = min(2.0 * variance, SPEC_AA_THRESHOLD);
  float alpha2 = clamp(alpha * alpha + kernel, 0.0, 1.0);
  return sqrt(sqrt(alpha2));
}

// Cook-Torrance direct lighting for one light, already multiplied by NoL (radiance applied by the caller)
vec3 DirectBRDF(vec3 N, vec3 V, vec3 L, float NoV, float NoL, float alpha,
                vec3 F0, vec3 diffuseColor, vec3 energyComp) {
  vec3 H = V + L;
  H *= inversesqrt(max(dot(H, H), 1e-8));
  float NoH = clamp(dot(N, H), 0.0, 1.0);
  float VoH = clamp(dot(V, H), 0.0, 1.0);

  float D   = D_GGX(NoH, alpha);
  float Vis = V_SmithGGXCorrelated(NoV, NoL, alpha);
  vec3  F   = F_Schlick(VoH, F0);

  vec3 specular = (D * Vis) * F * energyComp;
  vec3 diffuse  = diffuseColor / PI;
  return (diffuse + specular) * NoL;
}


// =====================================================================
//  POINT LIGHT SHADOW
//  The cube stores linear distance / uShadowFar, so everything below compares in world units.
// =====================================================================
// Golden-angle spiral: evenly spread points in a unit disk for any count, with a per-pixel rotation
vec2 VogelDisk(int i, int n, float rot) {
  float r = sqrt((float(i) + 0.5) / float(n));
  float a = float(i) * GOLDEN_ANGLE + rot;
  return r * vec2(cos(a), sin(a));
}

float PointShadow(vec3 worldPos, vec3 N, float NoL) {
  vec3 fromLight = worldPos - uShadowLightPos;
  float dist = length(fromLight);
  if(dist >= uShadowFar) return 1.0;

  // One shadow texel, as the tangent of a view angle, and in world units at this distance
  float angTexel = 2.0 / float(textureSize(uShadowCube, 0).x);
  float texel = dist * angTexel;

  // Bias scales with the real texel size, so it holds up as the light moves
  float sinT = sqrt(max(1.0 - NoL * NoL, 0.0));
  float slope = min(sinT / max(NoL, 0.1), SHADOW_MAX_SLOPE);

  vec3 p = fromLight + N * (texel * SHADOW_NORMAL_OFFSET * sinT);
  float cur = max(length(p), 1e-4);
  vec3 dir = p / cur;
  float bias = texel * (SHADOW_BIAS + slope);

  // Tangent frame around the lookup direction, for sampling a disk of offsets
  vec3 up = abs(dir.y) < 0.99 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
  vec3 T = normalize(cross(up, dir));
  vec3 B = cross(dir, T);
  float rot = InterleavedGradientNoise(gl_FragCoord.xy) * 2.0 * PI;

  float filterAng;   // final filter radius, as a tangent of the angle from dir
  if(SHADOW_SOFT) {
    // 1) Blocker search: average depth of whatever sits between this point and the light
    float searchAng = clamp(2.0 * SHADOW_LIGHT_SIZE / cur, 2.0 * angTexel, SHADOW_MAX_FILTER * angTexel);
    float searchBias = bias + 0.5 * cur * searchAng * slope;

    float blockerSum = 0.0;
    float blockers = 0.0;
    for(int i = 0; i < SHADOW_BLOCKER_TAPS; i++) {
      vec2 o = VogelDisk(i, SHADOW_BLOCKER_TAPS, rot) * searchAng;
      float closest = textureLod(uShadowCube, dir + T * o.x + B * o.y, 0.0).r * uShadowFar;
      if(closest < cur - searchBias) {
        blockerSum += closest;
        blockers += 1.0;
      }
    }
    if(blockers < 0.5) return 1.0;   // nothing in the way

    // 2) Penumbra width from similar triangles: sharp at contact, softer with distance from the caster
    float avgBlocker = blockerSum / blockers;
    float penumbra = SHADOW_LIGHT_SIZE * (cur - avgBlocker) / max(avgBlocker, 1e-3);
    filterAng = clamp(penumbra / cur, SHADOW_MIN_FILTER * angTexel, SHADOW_MAX_FILTER * angTexel);
  } else {
    filterAng = SHADOW_FIXED_FILTER * angTexel;
  }

  // 3) Final filter. A wider kernel needs more bias on tilted surfaces, or it shadows itself.
  float filterBias = bias + 0.5 * cur * filterAng * slope;
  float lit = 0.0;
  for(int i = 0; i < SHADOW_TAPS; i++) {
    vec2 o = VogelDisk(i, SHADOW_TAPS, rot) * filterAng;
    float closest = textureLod(uShadowCube, dir + T * o.x + B * o.y, 0.0).r * uShadowFar;
    lit += (cur - filterBias <= closest) ? 1.0 : 0.0;
  }
  float shadow = lit / float(SHADOW_TAPS);

  // Fade out toward the far plane so the shadow doesn't cut off abruptly
  return mix(shadow, 1.0, smoothstep(0.85, 1.0, dist / uShadowFar));
}


// =====================================================================
//  MAIN
// =====================================================================
void main() {
  // ---- Surface inputs (everything in linear light) ----
  vec3  albedo   = SRGBToLinear(uMaterial.baseColor) * SRGBToLinear(vColor.rgb);
  float outAlpha = vColor.a;
  if(uMaterial.useDiffuseTex) {
    vec4 tex = texture(uMaterial.diffuseTex, vUV);
    albedo   *= SRGBToLinear(tex.rgb);
    outAlpha *= tex.a;
  }

  vec3 N = normalize(vNormal);
  if(gl_FrontFacing == false) N = -N;
  vec3 V = normalize(uViewPoint - vPos);

  float metallic  = clamp(uMaterial.surface.y, 0.0, 1.0);
  float roughness = clamp(uMaterial.surface.x, 0.0, 1.0);
  if(SPEC_AA) roughness = FilterRoughness(roughness, N);
  roughness = clamp(roughness, MIN_ROUGHNESS, 1.0);
  float alpha = roughness * roughness;

  float NoV = abs(dot(N, V)) + 1e-5;
  vec3  F0  = mix(vec3(0.04), albedo, metallic);

  // Directional albedo of the specular lobe, used for energy conservation and ambient reflection
  vec2  ab = EnvBRDFApprox(roughness, NoV);
  vec3  specAlbedo = F0 * ab.x + ab.y;
  float Ess = max(ab.x + ab.y, 0.05);
  // Multiple-scattering compensation: rough metals lose energy in a single-scatter model
  vec3  energyComp = max(1.0 + F0 * (1.0 / Ess - 1.0), vec3(1.0));
  // Whatever the specular lobe reflects can't also be diffused
  vec3  diffuseColor = albedo * (1.0 - metallic) * max(1.0 - specAlbedo, 0.0);

  // ---- Ambient: sky/ground gradient for diffuse, plus a rough-aware fake reflection ----
  vec3 ambientColor = SRGBToLinear(uAmbientLight.rgb) * uAmbientLight.a;
  vec3 R = reflect(-V, N);
  vec3 color = diffuseColor * ambientColor * AmbientGradient(N, 1.0)
             + specAlbedo * energyComp * ambientColor * AmbientGradient(R, roughness);

  // ---- Direct lights ----
  int lightCount = min(uLightCount, MAX_LIGHTS);
  for(int i = 0; i < lightCount; i++) {
    Light light = lights[i];

    vec3 L;
    float dist = 0.0;
    if(light.type == TYPE_DIRECTIONAL) {
      float len = length(light.direction);
      L = (len > 1e-5) ? -light.direction / len : vec3(0.0, 1.0, 0.0);
    } else {
      vec3 toLight = light.position - vPos;
      dist = max(length(toLight), 1e-4);
      L = toLight / dist;
    }

    float attenuation = GetLightAttenuation(light, dist);
    if(attenuation <= 0.0) continue;

    if(light.type == TYPE_SPOT) {
      float len = length(light.direction);
      vec3 spotDir = (len > 1e-5) ? light.direction / len : vec3(0.0, -1.0, 0.0);

      float theta = dot(-L, spotDir);
      float epsilon = max(light.innerCutOff - light.outerCutOff, 1e-4);   // safe if zero or inverted
      float t = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
      attenuation *= t * t * (3.0 - 2.0 * t);                              // smoothstep edge

      if(attenuation <= 0.0) continue;
    }

    float NoL = dot(N, L);
    if(NoL <= 0.0) continue;   // facing away: nothing to light, and no shadow lookups wasted

    float shadow = 1.0;
    if(uHasShadow && i == 0 && light.type != TYPE_DIRECTIONAL) {
      shadow = PointShadow(vPos, N, NoL);
      if(shadow <= 0.0) continue;
    }

    vec3 radiance = SRGBToLinear(light.color.rgb) * (light.color.w * LIGHT_UNIT * attenuation * shadow);
    color += DirectBRDF(N, V, L, NoV, NoL, alpha, F0, diffuseColor, energyComp) * radiance;
  }

  FragColor = vec4(EncodeOutput(color), outAlpha);
}
