#version 430 core
// =============================================================================
// water.frag  -  sky reflection, sun glitter, subsurface color, foam, refraction, fog
//
// Pairs with water.vert. Adds the fine ripples (wavelengths below what the mesh can
// resolve) per-pixel as analytic normal detail, so mesh resolution only limits the
// big shapes, not the surface sparkle.
//
// UNIFORMS (types matter - a wrong glUniform* call is silently ignored):
//   uProj, uView          mat4
//   uTime                     float
//   uCamPos               vec3
//   uSunDir               vec3   direction TOWARD the sun
//   uSunColor             vec3   linear HDR sun radiance. Try vec3(3.0, 2.7, 2.2); length 2..6.
//                                (this one is vec3, not vec4)
//   uFogColor             vec4   rgb used. In DISPLAY space (match your clear color); alpha ignored.
//   uFogDensity           float  fog = 1 - exp(-(dist * uFogDensity * 0.01)^2).
//                                0.7 -> 39% fogged at 100 units, 67% at 150, 95% at 250.
//                                Good range 0.1..1.5. For a fixed 250-unit patch use ~0.7+ to hide the edge.
//   uWindDir              vec2   (x,z); uWindSpeed float; uAmplitudeScale float; uMeshSpacing float
//                                (same meaning as in water.vert; the detail waves must match)
//   uFoamThreshold        float  in std devs of surface compression. HIGHER = LESS foam.
//                                Share of the surface that ends up foamy (wind 10 m/s, intensity 1.0):
//                                  0.5 -> ~11%   1.0 -> ~4%   1.5 -> ~1%   2.0 -> ~0.2%
//                                Real whitecaps at 10 m/s are a few percent, so 0.8..1.3 looks natural.
//                                (Doesn'uTime depend on uChoppiness once that is >= 1; below 1 foam thins out.)
//   uFoamIntensity        float  coverage/opacity gain for foam. 1.0 = nominal (4% at threshold 1.0),
//                                2.0 = ~7%, white and blobby. Good range 0.5..1.5.
//   uSceneColor           sampler2D  scene rendered WITHOUT water (bind to unit 1)
//   uSceneDepth           sampler2D  its depth texture (bind to unit 2), standard depth, not reverse-Z
//   uHasSceneTex          bool   false -> no refraction / depth tint / shore foam. Set with glUniform1i.
//                                Textures must match the viewport size.
// =============================================================================

in vec3  vWorldPos;
in vec3  vNormal;
in float vHeightZ;
in float vFoamDrive;
in float vLostSlopeVar;

out vec4 FragColor;

uniform mat4  uProj;
uniform mat4  uView;
uniform float uTime;
uniform vec3  uCamPos;
uniform vec3  uSunDir;
uniform vec3  uSunColor;
uniform vec4  uFogColor;
uniform float uFogDensity;
uniform vec2  uWindDir;
uniform float uWindSpeed;
uniform float uAmplitudeScale;
uniform float uMeshSpacing;
uniform float uFoamThreshold;
uniform float uFoamIntensity;
uniform sampler2D uSceneColor;
uniform sampler2D uSceneDepth;
uniform bool uHasSceneTex;

// 1 = shade in linear light, tonemap + gamma at the end, decode uSceneColor from display space.
// 0 = leave everything linear (use if your pipeline tonemaps later and uSceneColor is linear HDR).
#define DISPLAY_SPACE_OUTPUT 1
#define saturate(x) clamp(x, 0.0, 1.0)

// ---- look (linear colors) - tweak to taste ----------------------------------
const vec3  ZENITH_COLOR  = vec3(0.08, 0.22, 0.55);     // top of the procedural sky
const vec3  WATER_COLOR   = vec3(0.012, 0.055, 0.10);   // deep-water body color
const vec3  SSS_COLOR     = vec3(0.02, 0.16, 0.12);     // teal glow of sunlit crests
const vec3  FOAM_COLOR    = vec3(0.92, 0.94, 0.96);
const vec3  ABSORPTION    = vec3(0.45, 0.075, 0.03);    // per unit of water depth (red dies first)
const float F0            = 0.02;                       // Fresnel reflectance of water
const float BASE_ROUGHNESS = 0.03;                      // GGX alpha of the unresolvable microripples
const float REFRACTION_STRENGTH = 0.06;
const float EXPOSURE      = 1.0;

// ---- constants shared with water.vert (keep in sync) ------------------------
const float G                 = 9.81;
const float TAU               = 6.28318530718;
const float PI                = 3.14159265359;
const float PM_ALPHA          = 0.0081;
const float VERTS_PER_WAVE    = 5.0;
const float SURFACE_TENSION   = 7.4e-5;                 // sigma/rho, water, m^3/s^2
const int   DETAIL_COUNT      = 8;
const float DETAIL_MIN_WAVELENGTH = 0.04;

float hash11(uint x) {
  x = x * 747796405u + 2891336453u;
  x = ((x >> ((x >> 28u) + 4u)) ^ x) * 277803737u;
  x = (x >> 22u) ^ x;
  return float(x) * (1.0 / 4294967295.0);
}

float hash21(vec2 p) {
  p = fract(p * vec2(123.34, 456.21));
  p += dot(p, p + 45.32);
  return fract(p.x * p.y);
}

float vnoise(vec2 p) {
  vec2 i = floor(p);
  vec2 f = fract(p);
  f = f * f * (3.0 - 2.0 * f);
  float a = hash21(i);
  float b = hash21(i + vec2(1.0, 0.0));
  float c = hash21(i + vec2(0.0, 1.0));
  float d = hash21(i + vec2(1.0, 1.0));
  return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(vec2 p) {
  float v = 0.0, amp = 0.5;
  for (int i = 0; i < 4; ++i) {
    v += amp * vnoise(p);
    p = p * 2.03 + 17.1;
    amp *= 0.5;
  }
  return v / 0.9375;
}

vec3 toLinear(vec3 c) {
#if DISPLAY_SPACE_OUTPUT
  return pow(c, vec3(2.2));
#else
  return c;
#endif
}

float linearizeDepth(float d) {                  // standard OpenGL perspective, depth range 0..1
  float ndc = d * 2.0 - 1.0;
  return uProj[3][2] / (ndc + uProj[2][2]);
}

vec3 skyColor(vec3 dir, vec3 L) {
  vec3  horizon = toLinear(uFogColor.rgb);
  vec3  tint    = uSunColor / max(max(uSunColor.r, uSunColor.g), max(uSunColor.b, 1e-4));
  vec3  col     = mix(horizon, ZENITH_COLOR, pow(saturate(dir.y), 0.4));
  float sd      = saturate(dot(dir, L));
  return col + tint * (0.35 * pow(sd, 8.0) + 1.2 * pow(sd, 150.0));   // broad glow (the sun itself is the GGX highlight)
}

vec3 acesFilm(vec3 x) {                          // Narkowicz ACES fit
  return clamp((x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14), 0.0, 1.0);
}

// Fine ripples: same spectrum as the vertex waves, continued below the mesh's
// resolution. Returns the extra height gradient, and the slope variance of ripples
// too small for this pixel to resolve (which becomes roughness instead).
void detailWaves(vec2 xz, vec2 wind, float U, float spacing, float fp,
                 out vec2 grad, out float lostVar) {
  grad    = vec2(0.0);
  lostVar = 0.0;

  float lamMax = VERTS_PER_WAVE * spacing;
  float lnSpan = log(lamMax / DETAIL_MIN_WAVELENGTH);
  float wp     = 0.877 * G / U;

  for (int i = 0; i < DETAIL_COUNT; ++i) {
    float f   = float(i) / float(DETAIL_COUNT - 1);
    float lam = lamMax * exp(-lnSpan * f);
    float k   = TAU / lam;
    float wg  = sqrt(G * k);
    float r   = wp / wg;
    float S   = PM_ALPHA * G * G / (wg * wg * wg * wg * wg) * exp(-1.25 * r * r * r * r);
    float dw  = wg * 0.5 * lnSpan / float(DETAIL_COUNT - 1);
    float a   = uAmplitudeScale * sqrt(2.0 * S * dw);
    float wc  = sqrt(G * k + SURFACE_TENSION * k * k * k);   // gravity-capillary dispersion

    float spread = mix(0.9, 2.2, f);                         // small ripples go every which way
    float ang    = (hash11(uint(i) * 2u + 101u) * 2.0 - 1.0) * spread;
    float ca = cos(ang), sa = sin(ang);
    vec2  d  = vec2(ca * wind.x - sa * wind.y, sa * wind.x + ca * wind.y);

    float ph  = k * dot(d, xz) - wc * uTime + TAU * hash11(uint(i) * 2u + 102u);
    float vis = 1.0 - smoothstep(0.2, 0.5, fp / lam);
    float ka  = k * a;

    grad    += d * (ka * vis * cos(ph));
    lostVar += 0.5 * ka * ka * (1.0 - vis * vis);
  }
}

void main() {
  vec3  toCam = uCamPos - vWorldPos;
  float dist  = length(toCam);
  vec3  V     = toCam / max(dist, 1e-4);
  vec3  L     = normalize(uSunDir);
  float sunVis = smoothstep(-0.05, 0.15, L.y);               // fade the sun out as it sets

  float U       = clamp(uWindSpeed, 2.0, 30.0);
  vec2  wind    = dot(uWindDir, uWindDir) > 1e-6 ? normalize(uWindDir) : normalize(vec2(1.0, 0.3));
  float spacing = uMeshSpacing > 0.0 ? uMeshSpacing : 0.5;

  // Size of one pixel on the water (world units), from screen-space derivatives.
  float fp = max(length(dFdx(vWorldPos.xz)), length(dFdy(vWorldPos.xz)));

  // ---- normal = mesh waves + per-pixel ripples ---------------------------------
  vec3 Ng = normalize(vNormal);
  vec2 slope = -Ng.xz / max(Ng.y, 0.05);
  vec2 detailGrad;
  float lostFrag;
  detailWaves(vWorldPos.xz, wind, U, spacing, fp, detailGrad, lostFrag);
  slope += detailGrad;
  vec3 N = normalize(vec3(-slope.x, 1.0, -slope.y));

  float NV = max(dot(N, V), 0.02);
  float NL = saturate(dot(N, L));
  float F  = F0 + (1.0 - F0) * pow(1.0 - NV, 5.0);

  // Detail that is too small to resolve widens the highlight (GGX alpha^2 ~ 2 * slope variance).
  float alpha = clamp(sqrt(BASE_ROUGHNESS * BASE_ROUGHNESS + 2.0 * (vLostSlopeVar + lostFrag)), 0.02, 0.7);

  // ---- reflections --------------------------------------------------------------
  vec3 R = reflect(-V, N);
  R.y = max(R.y, 0.0);
  vec3 skyRefl = skyColor(normalize(R), L);
  vec3 skyAmb  = mix(toLinear(uFogColor.rgb), ZENITH_COLOR, 0.5);

  vec3  H  = normalize(L + V);
  float NH = saturate(dot(N, H));
  float VH = saturate(dot(V, H));
  float a2 = alpha * alpha;
  float dd = NH * NH * (a2 - 1.0) + 1.0;
  float D  = a2 / (PI * dd * dd);
  float Vis = 0.5 / (NL * sqrt(NV * NV * (1.0 - a2) + a2) + NV * sqrt(NL * NL * (1.0 - a2) + a2) + 1e-5);
  float Fs = F0 + (1.0 - F0) * pow(1.0 - VH, 5.0);
  vec3  spec = min(uSunColor * (D * Vis * Fs * NL * sunVis), vec3(60.0));

  // ---- body color + subsurface glow on sunlit crests ------------------------------
  vec3  body = WATER_COLOR * (skyAmb + uSunColor * NL * 0.25 * sunVis);

  float crest = saturate(vHeightZ * 0.4 + 0.3);
  float sssView = pow(saturate(dot(V, -normalize(L + N * 0.3))), 4.0);
  vec3  sss = SSS_COLOR * uSunColor * (sssView * crest * 0.6 * sunVis);

  // ---- refraction / depth tint / shoreline ------------------------------------------
  vec3  transmitted = body;
  float shore = 0.0;
  if (uHasSceneTex) {
    vec2  uv    = gl_FragCoord.xy / vec2(textureSize(uSceneDepth, 0));
    float surfZ = linearizeDepth(gl_FragCoord.z);

    // Only the tilt away from flat water should shift the image.
    vec2 off = (mat3(uView) * N).xy - (mat3(uView) * vec3(0.0, 1.0, 0.0)).xy;
    vec2 ruv = clamp(uv + off * REFRACTION_STRENGTH / (1.0 + 0.05 * surfZ), vec2(0.001), vec2(0.999));

    float d = texture(uSceneDepth, ruv).r;
    if (linearizeDepth(d) < surfZ) {             // refracted ray landed on something in front of the water
      ruv = uv;
      d   = texture(uSceneDepth, uv).r;
    }
    float thick = (d >= 0.99999) ? 1000.0 : max(linearizeDepth(d) - surfZ, 0.0);

    vec3 sceneCol = toLinear(texture(uSceneColor, ruv).rgb);
    vec3 trans    = exp(-ABSORPTION * thick);
    transmitted   = mix(body, sceneCol, trans);
    shore         = 1.0 - smoothstep(0.0, 1.2, thick);
  }
  transmitted += sss;

  // ---- foam ----------------------------------------------------------------------------
  float m = smoothstep(uFoamThreshold, uFoamThreshold + 1.2, vFoamDrive);
  vec2  foamPos = vWorldPos.xz * 0.9 - wind * (uTime * 0.15);
  float pat = smoothstep(0.25, 0.75, fbm(foamPos));
  pat = mix(0.6, pat, 1.0 - smoothstep(0.4, 2.0, fp));       // distant foam: no shimmer
  float cover = max(m, shore) * uFoamIntensity;
  float foam  = smoothstep(1.0 - cover, 1.25 - cover, pat);
  vec3  foamLit = FOAM_COLOR * (skyAmb * 0.9 + uSunColor * NL * 0.25 * sunVis);

  // ---- compose ---------------------------------------------------------------------------
  vec3 col = mix(transmitted, skyRefl, F);
  col = mix(col, foamLit, foam * 0.95) + spec * (1.0 - foam);

#if DISPLAY_SPACE_OUTPUT
  col = acesFilm(col * EXPOSURE);
  col = pow(col, vec3(1.0 / 2.2));
#endif

  float fogAmt = 1.0 - exp(-pow(dist * uFogDensity * 0.01, 2.0));
  col = mix(col, uFogColor.rgb, fogAmt);

  FragColor = vec4(col, 1.0);
}
