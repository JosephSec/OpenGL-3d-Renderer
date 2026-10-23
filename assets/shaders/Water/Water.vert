#version 430 core
// =============================================================================
// water.vert  -  wind-driven ocean surface (Gerstner waves, Pierson-Moskowitz spectrum)
//
// Waves are evaluated in WORLD space, so moving/rotating the water mesh does not
// slide the waves, and several patches placed side by side line up seamlessly.
// Assumes a flat, horizontal grid (waves displace along world Y).
//
// Only waves the mesh can actually resolve (>= 5 vertices per wavelength) are
// done here. Everything finer is added per-pixel in water.frag.
//
// UNIFORMS (types matter - a wrong glUniform* call is silently ignored):
//   uProj, uView, uModel  mat4
//   uTime                 float  seconds
//   uCamPos               vec3   world-space camera position
//   uWindDir              vec2   (x,z) direction the wind blows TOWARD; need not be normalized
//   uWindSpeed            float  m/s (1 unit = 1 m). Clamped to [2,30]. Good range 4..15.
//                                Peak wavelength ~ 0.83*U^2  ->  U=5: 21 m, U=10: 83 m, U=15: 187 m.
//                                Above ~15 the longest swell no longer fits a 250 m patch.
//   uAmplitudeScale       float  multiplier on the physical wave height. 1.0 = a fully developed sea
//                                for the given wind (U=10 -> ~2.2 m significant wave height).
//                                Good range 0.5..1.5.
//   uChoppiness           float  crest sharpness. 1.0 = sharpest crests that can never fold over.
//                                >1 lets crests fold (>~2 gets messy). Good range 0.3..1.5.
//   uMeshSpacing          float  OPTIONAL: distance between grid vertices. Falls back to 0.5 if unset.
// =============================================================================

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;   // unused, kept so your VAO layout still matches

out vec3  vWorldPos;
out vec3  vNormal;        // world-space normal of the Gerstner surface
out float vHeightZ;       // wave height in standard deviations (0 = mean level, +2 = tall crest)
out float vFoamDrive;     // surface compression in std devs; drives foam in water.frag
out float vLostSlopeVar;  // slope variance of waves faded out by distance -> becomes roughness

uniform mat4  uProj;
uniform mat4  uView;
uniform mat4  uModel;
uniform float uTime;
uniform vec3  uCamPos;
uniform vec2  uWindDir;
uniform float uWindSpeed;
uniform float uAmplitudeScale;
uniform float uChoppiness;
uniform float uMeshSpacing;

const int   WAVE_COUNT      = 12;
const float G               = 9.81;
const float TAU             = 6.28318530718;
const float PM_ALPHA        = 0.0081;   // Pierson-Moskowitz constant
const float VERTS_PER_WAVE  = 5.0;      // shortest geometric wavelength = this * mesh spacing
const float MAX_WAVELENGTH  = 220.0;
const float PIXEL_ANGLE     = 0.0012;   // ~ vertical fov / screen height, in radians

// ---- deterministic integer hash (same result on every GPU) -------------------
float hash11(uint x) {
  x = x * 747796405u + 2891336453u;
  x = ((x >> ((x >> 28u) + 4u)) ^ x) * 277803737u;
  x = (x >> 22u) ^ x;
  return float(x) * (1.0 / 4294967295.0);
}

struct Wave {
  vec2  d;      // travel direction
  float k;      // wave number (2*pi / wavelength)
  float w;      // angular frequency
  float a;      // amplitude
  float ph;     // random phase offset
};

// Wave i of the set: wavelengths are spaced geometrically from lamMax down to lamMin.
// Amplitude comes from the Pierson-Moskowitz spectrum: a = sqrt(2 * S(w) * dw).
Wave makeWave(int i, vec2 wind, float U, float lamMax, float lnSpan) {
  float f   = float(i) / float(WAVE_COUNT - 1);     // 0 = longest, 1 = shortest
  float lam = lamMax * exp(-lnSpan * f);
  float k   = TAU / lam;
  float w   = sqrt(G * k);                          // deep-water dispersion

  float wp  = 0.877 * G / U;                        // spectrum peak frequency
  float r   = wp / w;
  float S   = PM_ALPHA * G * G / (w * w * w * w * w) * exp(-1.25 * r * r * r * r);
  float dw  = w * 0.5 * lnSpan / float(WAVE_COUNT - 1);

  // Long waves line up with the wind, short waves scatter more.
  float spread = mix(0.25, 1.10, f);
  float ang    = (hash11(uint(i) * 2u + 1u) * 2.0 - 1.0) * spread;
  float ca = cos(ang), sa = sin(ang);
  vec2  d  = vec2(ca * wind.x - sa * wind.y, sa * wind.x + ca * wind.y);

  return Wave(d, k, w, uAmplitudeScale * sqrt(2.0 * S * dw), TAU * hash11(uint(i) * 2u + 2u));
}

void main() {
  vec4 baseWorld = uModel * vec4(aPos, 1.0);
  vec2 xz = baseWorld.xz;

  // Approximate size of one screen pixel on the water here (world units).
  // Waves smaller than ~2-5 pixels would alias, so they are faded out and their
  // slope energy is handed to the fragment shader as extra roughness.
  vec3  toCam = uCamPos - baseWorld.xyz;
  float dist  = length(toCam);
  float fp    = dist * dist * PIXEL_ANGLE / max(abs(toCam.y), max(0.05 * dist, 1e-3));

  // ---- wave set -------------------------------------------------------------
  float U       = clamp(uWindSpeed, 2.0, 30.0);
  vec2  wind    = dot(uWindDir, uWindDir) > 1e-6 ? normalize(uWindDir) : normalize(vec2(1.0, 0.3));
  float spacing = uMeshSpacing > 0.0 ? uMeshSpacing : 0.5;

  float lamMin  = VERTS_PER_WAVE * spacing;
  float lamMax  = max(min(1.6 * 0.8328 * U * U, MAX_WAVELENGTH), 4.0 * lamMin);
  float lnSpan  = log(lamMax / lamMin);

  Wave  waves[WAVE_COUNT];
  float sumKA = 0.0, sumKA2 = 0.0, sumA2 = 0.0;
  for (int i = 0; i < WAVE_COUNT; ++i) {
    waves[i] = makeWave(i, wind, U, lamMax, lnSpan);
    float ka = waves[i].k * waves[i].a;
    sumKA  += ka;
    sumKA2 += ka * ka;
    sumA2  += waves[i].a * waves[i].a;
  }

  // One shared horizontal-displacement factor. sum(q*k*a) = uChoppiness, so at
  // uChoppiness <= 1 the surface can never fold over, and steeper waves get
  // proportionally sharper crests.
  float q     = uChoppiness / max(sumKA, 1e-4);
  float sigH  = sqrt(0.5 * sumA2);               // std dev of wave height
  float sigJ  = q * sqrt(0.5 * sumKA2);          // std dev of surface compression

  // ---- sum the waves ----------------------------------------------------------
  float h = 0.0, gx = 0.0, gz = 0.0;
  float dispX = 0.0, dispZ = 0.0;
  float Jxx = 1.0, Jzz = 1.0, Jxz = 0.0;         // Jacobian of the horizontal displacement
  float lostVar = 0.0;

  for (int i = 0; i < WAVE_COUNT; ++i) {
    Wave  wv  = waves[i];
    float lam = TAU / wv.k;
    float vis = 1.0 - smoothstep(0.2, 0.5, fp / lam);
    float a   = wv.a * vis;
    float ka  = wv.k * a;
    float A   = q * ka;

    float ph = wv.k * dot(wv.d, xz) - wv.w * uTime + wv.ph;
    float s  = sin(ph);
    float c  = cos(ph);

    h     += a * s;
    dispX += q * a * wv.d.x * c;                 // crests bunch up horizontally
    dispZ += q * a * wv.d.y * c;
    gx    += ka * wv.d.x * c;                    // height gradient
    gz    += ka * wv.d.y * c;
    Jxx   -= A * wv.d.x * wv.d.x * s;
    Jzz   -= A * wv.d.y * wv.d.y * s;
    Jxz   -= A * wv.d.x * wv.d.y * s;

    float kaFull = wv.k * wv.a;
    lostVar += 0.5 * kaFull * kaFull * (1.0 - vis * vis);
  }

  float J = Jxx * Jzz - Jxz * Jxz;               // < 1 where the surface is squeezed together

  // Exact normal of the displaced surface (cross product of the two tangents).
  vec3 n = normalize(vec3(gz * Jxz - Jzz * gx,
                          max(J, 0.02),
                          Jxz * gx - gz * Jxx));

  vec3 worldPos = baseWorld.xyz + vec3(dispX, h, dispZ);

  vWorldPos     = worldPos;
  vNormal       = n;
  vHeightZ      = h / max(sigH, 1e-4);
  // Scaled down below uChoppiness = 1 so a nearly flat-crested sea has less foam.
  vFoamDrive    = (1.0 - J) / max(sigJ, 1e-4) * min(uChoppiness, 1.0);
  vLostSlopeVar = lostVar;

  gl_Position = uProj * uView * vec4(worldPos, 1.0);
}
