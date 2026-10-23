#version 430 core

// =====================================================================
//  Photoreal post-processing (single pass)
//  GTAO + screen-space GI + glossy SSR + physical DoF + height fog
//  + bloom + lens/film (CA, cos^4 vignette, grain) + ACES
//
//  Assumes a standard OpenGL perspective projection (depth range -1..1,
//  e.g. glm::perspective without GLM_FORCE_DEPTH_ZERO_TO_ONE).
//  Uses only the existing uniforms: no new C++ side setup required.
// =====================================================================

in vec2 vUV;

uniform vec2      uViewSize;
uniform sampler2D uColor;
uniform sampler2D uDepth;
uniform float     uTime;
uniform mat4      uView;
uniform mat4      uProj;
uniform vec3      uCameraWorldPos;

out vec4 fragColor;

// =====================================================================
//  SETTINGS
// =====================================================================
#define QUALITY 0   // 0 = fast, 1 = low, 2 = high, 3 = ultra

// What the per-tier sample counts below control (the names are the same in every tier):
//   AO_SLICES  - How many screen directions the ambient occlusion sweeps, where more gives smoother AO.
//   AO_STEPS   - How many depth samples are taken along each AO direction, where more gives more accurate contact shadows.
//   SSR_RAYS   - How many reflection rays each pixel traces, where more gives less noisy reflections (0 turns reflections off).
//   SSR_STEPS  - How many steps each reflection ray takes across the screen, where more catches thinner objects.
//   GI_RAYS    - How many bounce-light rays each pixel traces, where more gives less noisy colour bleeding (0 turns GI off).
//   DOF_TAPS   - How many blur samples each pixel takes for depth of field, where more gives smoother blur and bokeh.
//   BLOOM_TAPS - How many samples each of the three bloom rings takes, where more gives a smoother glow.
#if QUALITY == 0
  const int AO_SLICES = 2, AO_STEPS = 4,  SSR_RAYS = 0, SSR_STEPS = 16, GI_RAYS = 0, DOF_TAPS = 12, BLOOM_TAPS = 4;
#elif QUALITY == 1
  const int AO_SLICES = 3, AO_STEPS = 6,  SSR_RAYS = 1, SSR_STEPS = 32, GI_RAYS = 1, DOF_TAPS = 24, BLOOM_TAPS = 8;
#elif QUALITY == 2
  const int AO_SLICES = 5, AO_STEPS = 12, SSR_RAYS = 2, SSR_STEPS = 64, GI_RAYS = 2, DOF_TAPS = 48, BLOOM_TAPS = 16;
#else
  const int AO_SLICES = 10, AO_STEPS = 24, SSR_RAYS = 4, SSR_STEPS = 128, GI_RAYS = 4, DOF_TAPS = 96, BLOOM_TAPS = 32;
#endif

// ---- colour pipeline ----
const bool  SCENE_IS_LINEAR = false;  // Set true if your lit shader already writes linear light, so this shader skips its sRGB-to-linear conversion.
const bool  SCENE_IS_HDR    = false;  // Set true if the colour target is a float format (GL_RGBA16F) whose values can exceed 1.0, which disables the highlight boost.
const float LDR_HDR_BOOST   = 1.6;    // Brightens near-white pixels of an 8-bit target to fake HDR headroom, which strengthens bloom and reflected highlights.
const float EXPOSURE        = 1.0;    // Multiplies overall image brightness before tonemapping, where values above 1 are brighter.
const bool  ANIMATE_NOISE   = false;  // Makes the sampling noise change every frame, which only looks good with temporal accumulation and otherwise flickers.

// ---- ambient occlusion (GTAO) ----
const float AO_RADIUS       = 1.0;    // How far (in view-space units) a nearby object can be and still darken a surface.
const float AO_MAX_PIXELS   = 200.0;  // Caps the AO search radius in screen pixels so close surfaces shrink the radius instead of undersampling.
const float AO_FALLOFF      = 0.6;    // The fraction of the AO radius over which occluders fade out, which avoids hard halos at the edge of the radius.
const float AO_POWER        = 1.4;    // The contrast of the AO, where higher values make occluded areas darker.

// ---- reflections ----
const float SSR_F0          = 0.25;   // How reflective surfaces are when viewed head-on (real non-metals are about 0.04, higher looks wetter and glossier).
const float SSR_ROUGHNESS   = 0.30;   // How blurry reflections are (0 = mirror, 1 = very diffuse), global because there is no material buffer, and lower values are much less noisy.
const float SSR_MAX_DIST    = 40.0;   // The farthest a reflection ray travels, in view-space units.
const float SSR_THICKNESS   = 0.2;    // How thick the depth buffer assumes surfaces are, so rays do not slip behind thin objects or stick to the wrong ones.
const float SSR_MAX_RADIANCE = 2.0;   // Caps how bright a reflected pixel can be, which stops tiny bright lights from showing up as speckles across reflective surfaces.
const vec3  ENV_COLOR       = vec3(0.05, 0.06, 0.08);   // The colour a reflection ray sees when it leaves the screen or hits nothing.

// ---- screen-space GI (one diffuse bounce) ----
const float GI_STRENGTH     = 0.45;   // How strongly light bounced from nearby surfaces tints a pixel.
const float GI_MAX_DIST     = 4.0;    // How far (in view-space units) a GI ray searches for a surface to bounce light from.
const int   GI_STEPS        = 16;     // How many steps each GI ray takes, where more finds more bounce but costs more.
const float GI_THICKNESS    = 0.4;    // How thick the depth buffer assumes surfaces are for GI rays, the diffuse counterpart of SSR_THICKNESS.
const float GI_MAX_RADIANCE = 1.5;    // Caps how bright a bounced pixel can be, which removes bright speckles around strong lights.

// ---- depth of field (thin-lens model, units = metres) ----
const bool  AUTOFOCUS       = true;   // When true the focus distance follows whatever is at the centre of the screen.
const float FOCUS_DISTANCE  = 4.0;    // The distance in metres that is in focus when AUTOFOCUS is false.
const float FOCAL_MM        = 35.0;   // The lens focal length in millimetres, where longer lenses give a shallower depth of field.
const float F_STOP          = 2.8;    // The lens aperture, where lower numbers give more blur.
const float SENSOR_MM       = 24.0;   // The sensor height in millimetres, which converts physical blur into pixels (a larger sensor gives more blur).
const float DOF_MAX_PX      = 16.0;   // The largest blur radius in pixels, which also limits how far blurry foreground objects can bleed over others.
const float DOF_BOKEH_BOOST = 2.0;    // How much extra weight bright highlights get when blurred into bokeh discs (0 = plain blur, higher = punchier but noisier discs).

// ---- bloom ----
const float BLOOM_AMOUNT    = 0.14;   // The overall strength of the glow added around bright areas.
const float BLOOM_THRESH    = 0.9;    // How bright a pixel must be before it contributes any glow.
const float BLOOM_KNEE      = 0.5;    // How soft the threshold edge is, where higher values fade the glow in more gradually.
const float BLOOM_MAX       = 2.0;    // Caps how bright any single bloom sample can be, which stops tiny very bright pixels from creating speckled halos.
const vec3  BLOOM_RADII     = vec3(0.004, 0.016, 0.040);   // The radii of the small, medium and large glow rings as fractions of screen height.

// ---- fog (exponential height fog, world units) ----
const vec3  FOG_COLOR           = vec3(0.50, 0.58, 0.70);   // The colour distant objects fade toward, in sRGB.
const float FOG_DENSITY         = 0.010;   // How thick the fog is at its base height, where higher values fade things out sooner.
const float FOG_HEIGHT_FALLOFF  = 0.18;    // How quickly the fog thins out with height, where higher values keep it hugging the ground.
const float FOG_BASE_HEIGHT     = -5.0;    // The world-space height (Y) at which the fog is at full density.

// ---- lens / film / grade ----
const float CA_AMOUNT       = 0.0015;  // How far the red and blue channels shift apart toward the screen edges (chromatic aberration).
const float VIGNETTE        = 0.3;     // How much the corners of the image darken, where 0 is off.
const float GRAIN           = 0.02;    // The strength of the animated film grain.
const float SATURATION      = 1.05;    // The colour intensity of the final image, where 1.0 is unchanged and 0.0 is greyscale.
const float CONTRAST        = 0.15;    // How strongly an S-curve is applied to the final image, where 0 is off.

// =====================================================================
//  CONSTANTS / GLOBALS
// =====================================================================
const float PI            = 3.14159265359;   // Half a turn in radians.
const float HALF_PI       = 1.57079632679;   // A quarter turn in radians.
const float TAU           = 6.28318530718;   // A full turn in radians.
const float GOLDEN_ANGLE  = 2.39996323;      // The angle in radians that spreads sample points evenly around a disc without lining up.

ivec2 gSize;
uvec2 gPix;
uint  gFrame;

// =====================================================================
//  HELPERS
// =====================================================================
float luma(vec3 c) { return dot(c, vec3(0.2126, 0.7152, 0.0722)); }
float max3(vec3 c) { return max(c.r, max(c.g, c.b)); }

// Scales a colour down so its brightest channel is at most m, keeping its hue.
vec3 clampRadiance(vec3 c, float m) { return c * min(1.0, m / max(max3(c), 1e-4)); }

// Interleaved gradient noise: a fine, evenly spread per-pixel pattern that looks much
// smoother than white noise when used to rotate sample patterns.
float ign(vec2 p) { return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715)))); }

uint pcg(uint v) {
  uint state = v * 747796405u + 2891336453u;
  uint word  = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
  return (word >> 22u) ^ word;
}
float rnd(uint k) {
  return float(pcg(gPix.x + pcg(gPix.y + pcg(k + gFrame * 7919u)))) * 2.3283064e-10;
}

vec3 srgbToLinear(vec3 c) {
  c = max(c, 0.0);
  return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), step(vec3(0.04045), c));
}
vec3 linearToSrgb(vec3 c) {
  c = max(c, 0.0);
  return mix(c * 12.92, 1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055, step(vec3(0.0031308), c));
}

// Scene colour in linear HDR-ish space
vec3 sceneColor(vec2 uv) {
  vec3 c = texture(uColor, uv).rgb;
  if(!SCENE_IS_LINEAR) c = srgbToLinear(c);
  if(!SCENE_IS_HDR) {
    float m = clamp(max3(c), 0.0, 1.0);
    c *= 1.0 + LDR_HDR_BOOST * pow(m, 6.0);
  }
  return c;
}

void basis(vec3 N, out vec3 T, out vec3 B) {
  vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
  T = normalize(cross(up, N));
  B = cross(N, T);
}

vec3 sampleGGX(vec2 xi, vec3 N, float alpha) {
  float phi  = TAU * xi.x;
  float cosT = sqrt((1.0 - xi.y) / (1.0 + (alpha * alpha - 1.0) * xi.y));
  float sinT = sqrt(max(1.0 - cosT * cosT, 0.0));
  vec3 T, B; basis(N, T, B);
  return normalize(T * (sinT * cos(phi)) + B * (sinT * sin(phi)) + N * cosT);
}

vec3 cosineHemisphere(vec2 xi, vec3 N) {
  float phi = TAU * xi.x;
  float r   = sqrt(xi.y);
  vec3 T, B; basis(N, T, B);
  return normalize(T * (r * cos(phi)) + B * (r * sin(phi)) + N * sqrt(max(1.0 - xi.y, 0.0)));
}

float edgeFade(vec2 uv) {
  vec2 e = min(uv, 1.0 - uv);
  return smoothstep(0.0, 0.08, min(e.x, e.y));
}

// =====================================================================
//  DEPTH / POSITION / NORMAL
// =====================================================================
float depthAt(ivec2 p) {
  return texelFetch(uDepth, clamp(p, ivec2(0), gSize - 1), 0).r;
}

// Linear view-space z (negative in front of the camera)
float viewZ(float d) {
  return -uProj[3][2] / (d * 2.0 - 1.0 + uProj[2][2]);
}

vec3 viewPosPx(ivec2 p, float d) {
  float z = viewZ(d);
  vec2 ndc = (vec2(p) + 0.5) / vec2(gSize) * 2.0 - 1.0;
  return vec3((ndc + vec2(uProj[2][0], uProj[2][1])) * (-z) / vec2(uProj[0][0], uProj[1][1]), z);
}

// Normal from the best-fitting one-sided neighbours. Hardware depth is affine in
// screen space on planes, so extrapolating it predicts which side lies on the
// same surface. Clean at silhouettes and on close, shallow-angle surfaces.
vec3 reconstructNormal(ivec2 p, float d0, vec3 P) {
  float l1 = depthAt(p + ivec2(-1, 0)), l2 = depthAt(p + ivec2(-2, 0));
  float r1 = depthAt(p + ivec2( 1, 0)), r2 = depthAt(p + ivec2( 2, 0));
  float b1 = depthAt(p + ivec2(0, -1)), b2 = depthAt(p + ivec2(0, -2));
  float t1 = depthAt(p + ivec2(0,  1)), t2 = depthAt(p + ivec2(0,  2));

  bool useL = abs((2.0 * l1 - l2) - d0) < abs((2.0 * r1 - r2) - d0);
  bool useB = abs((2.0 * b1 - b2) - d0) < abs((2.0 * t1 - t2) - d0);

  vec3 PH = useL ? viewPosPx(p + ivec2(-1, 0), l1) : viewPosPx(p + ivec2(1, 0), r1);
  vec3 PV = useB ? viewPosPx(p + ivec2(0, -1), b1) : viewPosPx(p + ivec2(0, 1), t1);

  vec3 dx = useL ? P - PH : PH - P;   // always points to +x
  vec3 dy = useB ? P - PV : PV - P;   // always points to +y
  return normalize(cross(dx, dy));    // faces the camera
}

// =====================================================================
//  GTAO  (ground-truth AO, analytic cosine-weighted horizon integral)
// =====================================================================
vec3 multiBounce(float vis, vec3 albedo) {
  vec3 a = 2.0404 * albedo - 0.3324;
  vec3 b = -4.7951 * albedo + 0.6417;
  vec3 c = 2.7552 * albedo + 0.6903;
  return max(vec3(vis), ((vis * a + b) * vis + c) * vis);
}

float gtao(vec2 fragPx, vec3 P, vec3 N, float noiseSlice, float noiseStep) {
  vec3 V = normalize(-P);

  float pxPerUnit  = 0.5 * float(gSize.y) * uProj[1][1] / -P.z;
  float radius     = min(AO_RADIUS, AO_MAX_PIXELS / pxPerUnit);
  float pixelRadius = radius * pxPerUnit;
  if(pixelRadius < 2.0) return 1.0;

  float falloffRange = AO_FALLOFF * radius;
  float falloffMul   = -1.0 / falloffRange;
  float falloffAdd   = (radius - falloffRange) / falloffRange + 1.0;

  float vis = 0.0;
  for(int i = 0; i < AO_SLICES; i++) {
    float phi = (float(i) + noiseSlice) / float(AO_SLICES) * PI;
    vec2 dir = vec2(cos(phi), sin(phi));

    vec3 dirVec   = vec3(dir, 0.0);
    vec3 orthoDir = dirVec - dot(dirVec, V) * V;
    vec3 axis     = normalize(cross(orthoDir, V));
    vec3 projN    = N - axis * dot(N, axis);
    float projLen = length(projN);
    if(projLen < 1e-4) continue;

    float cosN = clamp(dot(projN, V) / projLen, 0.0, 1.0);
    float n    = sign(dot(orthoDir, projN)) * acos(cosN);

    float lowP = cos(n + HALF_PI);
    float lowN = cos(n - HALF_PI);
    float hcP = lowP, hcN = lowN;

    for(int s = 0; s < AO_STEPS; s++) {
      float t = (float(s) + noiseStep) / float(AO_STEPS);
      float offPx = max(t * t * pixelRadius, float(s) + 1.0);
      vec2 off = dir * offPx;

      ivec2 pP = ivec2(floor(fragPx + off));
      ivec2 pN = ivec2(floor(fragPx - off));

      if(all(greaterThanEqual(pP, ivec2(0))) && all(lessThan(pP, gSize))) {
        float dP = depthAt(pP);
        if(dP < 1.0) {
          vec3 delta = viewPosPx(pP, dP) - P;
          float len = max(length(delta), 1e-5);
          float w = clamp(len * falloffMul + falloffAdd, 0.0, 1.0);
          hcP = max(hcP, mix(lowP, dot(delta, V) / len, w));
        }
      }
      if(all(greaterThanEqual(pN, ivec2(0))) && all(lessThan(pN, gSize))) {
        float dN = depthAt(pN);
        if(dN < 1.0) {
          vec3 delta = viewPosPx(pN, dN) - P;
          float len = max(length(delta), 1e-5);
          float w = clamp(len * falloffMul + falloffAdd, 0.0, 1.0);
          hcN = max(hcN, mix(lowN, dot(delta, V) / len, w));
        }
      }
    }

    float h0 = -acos(clamp(hcN, -1.0, 1.0));
    float h1 =  acos(clamp(hcP, -1.0, 1.0));
    h0 = n + clamp(h0 - n, -HALF_PI, HALF_PI);
    h1 = n + clamp(h1 - n, -HALF_PI, HALF_PI);

    float iarc0 = (cosN + 2.0 * h0 * sin(n) - cos(2.0 * h0 - n)) * 0.25;
    float iarc1 = (cosN + 2.0 * h1 * sin(n) - cos(2.0 * h1 - n)) * 0.25;
    vis += projLen * (iarc0 + iarc1);
  }
  return clamp(vis / float(AO_SLICES), 0.0, 1.0);
}

// =====================================================================
//  SCREEN-SPACE RAY TRACE
//  Marches in pixel space along the projected ray (perspective-correct z),
//  step size grows with distance, binary refinement on hit.
// =====================================================================
bool traceRay(vec3 org, vec3 dir, float maxDist, int maxSteps, float jitter, float thickness,
              out ivec2 hitPx, out float hitDist)
{
  hitPx = ivec2(0);
  hitDist = 0.0;

  float nearZ = -uProj[3][2] / (-1.0 + uProj[2][2]);   // = -near
  float len = maxDist;
  if(org.z + dir.z * len > nearZ) len = (nearZ - org.z) / max(dir.z, 1e-6);   // clip to near plane
  if(len <= 0.0) return false;

  vec3 endV = org + dir * len;
  vec4 H0 = uProj * vec4(org, 1.0);
  vec4 H1 = uProj * vec4(endV, 1.0);
  float k0 = 1.0 / H0.w, k1 = 1.0 / H1.w;
  vec2 P0 = (H0.xy * k0 * 0.5 + 0.5) * uViewSize;
  vec2 P1 = (H1.xy * k1 * 0.5 + 0.5) * uViewSize;
  float zk0 = org.z * k0, zk1 = endV.z * k1;

  float pxLen = length(P1 - P0);
  if(pxLen < 2.0) return false;
  vec2 dirPx = (P1 - P0) / pxLen;

  float prevTravel = 0.0;
  float prevZ = org.z;

  for(int i = 0; i < maxSteps; i++) {
    float t = (float(i) + jitter) / float(maxSteps);
    float traveled = max(pxLen * pow(t, 1.6), prevTravel + 1.0);
    if(traveled >= pxLen) break;

    vec2 px = P0 + dirPx * traveled;
    if(px.x < 0.0 || px.y < 0.0 || px.x >= uViewSize.x || px.y >= uViewSize.y) break;

    float f = traveled / pxLen;
    float rayZ = mix(zk0, zk1, f) / mix(k0, k1, f);

    float d = depthAt(ivec2(px));
    if(d < 1.0) {
      float sceneZ = viewZ(d);
      float diff   = sceneZ - rayZ;                       // > 0: ray is behind the surface
      float thick  = thickness + abs(prevZ - rayZ);       // allow for large far-away steps
      if(diff > 0.002 * -rayZ && diff < thick) {
        float lo = prevTravel, hi = traveled;
        for(int k = 0; k < 8; k++) {
          float mid = 0.5 * (lo + hi);
          float fm  = mid / pxLen;
          float rz  = mix(zk0, zk1, fm) / mix(k0, k1, fm);
          float sz  = viewZ(depthAt(ivec2(P0 + dirPx * mid)));
          if(sz - rz > 0.0) hi = mid; else lo = mid;
        }
        hitPx = ivec2(P0 + dirPx * hi);
        hitDist = length(viewPosPx(hitPx, depthAt(hitPx)) - org);
        return true;
      }
    }
    prevTravel = traveled;
    prevZ = rayZ;
  }
  return false;
}

// =====================================================================
//  DEPTH OF FIELD
// =====================================================================
float cocRadiusPx(float dist, float focus) {
  float s1 = max(focus * 1000.0, FOCAL_MM * 1.1);   // mm
  float s2 = max(dist * 1000.0, 10.0);
  float c  = (FOCAL_MM * FOCAL_MM) / (F_STOP * (s1 - FOCAL_MM)) * abs(s2 - s1) / s2;   // mm on sensor
  return 0.5 * c / SENSOR_MM * float(gSize.y);
}

float focusDistance() {
  if(!AUTOFOCUS) return FOCUS_DISTANCE;
  float sum = 0.0, cnt = 0.0;
  for(int y = -1; y <= 1; y++) {
    for(int x = -1; x <= 1; x++) {
      float d = depthAt(gSize / 2 + ivec2(x, y) * 4);
      if(d < 1.0) { sum += -viewZ(d); cnt += 1.0; }
    }
  }
  return cnt > 0.0 ? sum / cnt : 20.0;
}

// Scatter-as-gather. Even taps sample the pixel's own blur disc, odd taps reach
// out to catch blurry foreground objects bleeding over this pixel.
vec3 dofGather(vec2 uv, float centerCoc, float centerDepth, float focus, float rot) {
  vec3 sum = sceneColor(uv);
  float wsum = 1.0;
  float selfR = min(max(centerCoc, 1.0), DOF_MAX_PX);

  for(int i = 0; i < DOF_TAPS; i++) {
    float t = (float(i / 2) + 0.5) / float(DOF_TAPS / 2);
    float reach = ((i & 1) == 0) ? selfR : DOF_MAX_PX;
    float r = reach * sqrt(t);
    float a = float(i) * GOLDEN_ANGLE + rot * TAU;

    vec2 tuv = uv + vec2(cos(a), sin(a)) * r / vec2(gSize);
    float td = -viewZ(depthAt(ivec2(tuv * vec2(gSize))));
    float tc = cocRadiusPx(td, focus);
    if(td > centerDepth) tc = min(tc, max(centerCoc, 0.5) * 2.0);   // background must not bleed over foreground

    float w = smoothstep(r - 0.5, r + 0.5, tc) * smoothstep(0.4, 1.0, tc);
    if(w > 0.0) {
      vec3 c = sceneColor(tuv);
      float bw = 1.0 + DOF_BOKEH_BOOST * smoothstep(1.0, 4.0, luma(c));   // bright highlights form bokeh discs
      sum  += c * (w * bw);
      wsum += w * bw;
    }
  }
  return sum / wsum;
}

// =====================================================================
//  BLOOM
// =====================================================================
vec3 brightPass(vec3 c) {
  float br = max3(c);
  float soft = clamp(br - BLOOM_THRESH + BLOOM_KNEE, 0.0, 2.0 * BLOOM_KNEE);
  soft = soft * soft / (4.0 * BLOOM_KNEE + 1e-4);
  float contrib = max(soft, br - BLOOM_THRESH) / max(br, 1e-4);
  return c * contrib;
}

vec3 bloomRing(float radiusPx, float rot) {
  vec3 sum = vec3(0.0);
  float wsum = 0.0;
  for(int i = 0; i < BLOOM_TAPS; i++) {
    float t = (float(i) + 0.5) / float(BLOOM_TAPS);
    float a = float(i) * GOLDEN_ANGLE + rot * TAU;
    vec2 uv = vUV + vec2(cos(a), sin(a)) * (radiusPx * sqrt(t)) / vec2(gSize);
    float w = exp(-2.5 * t);
    sum  += clampRadiance(brightPass(sceneColor(uv)), BLOOM_MAX) * w;
    wsum += w;
  }
  return sum / wsum;
}

// =====================================================================
//  FOG
// =====================================================================
vec3 applyFog(vec3 c, float dist, vec3 rayW) {
  float b    = FOG_HEIGHT_FALLOFF;
  float camH = uCameraWorldPos.y - FOG_BASE_HEIGHT;
  float base = FOG_DENSITY * exp(clamp(-b * camH, -60.0, 60.0));
  float k    = b * rayW.y;
  float tau;
  if(abs(k * dist) > 1e-3) tau = base * (1.0 - exp(clamp(-k * dist, -60.0, 60.0))) / k;
  else                     tau = base * dist;
  float T = exp(-max(tau, 0.0));
  return mix(srgbToLinear(FOG_COLOR), c, T);
}

// =====================================================================
//  TONEMAP (ACES, Hill fit)
// =====================================================================
const mat3 ACES_IN = mat3(          // Converts linear sRGB into the colour space the ACES curve is fitted in.
  0.59719, 0.07600, 0.02840,
  0.35458, 0.90834, 0.13383,
  0.04823, 0.01566, 0.83777);
const mat3 ACES_OUT = mat3(         // Converts from the ACES fitting space back to linear sRGB after the curve is applied.
   1.60475, -0.10208, -0.00327,
  -0.53108,  1.10813, -0.07276,
  -0.07367, -0.00605,  1.07602);

vec3 aces(vec3 c) {
  c = ACES_IN * c;
  vec3 a = c * (c + 0.0245786) - 0.000090537;
  vec3 b = c * (0.983729 * c + 0.4329510) + 0.238081;
  return clamp(ACES_OUT * (a / b), 0.0, 1.0);
}

// =====================================================================
//  MAIN
// =====================================================================
void main() {
  gSize  = ivec2(uViewSize + 0.5);
  gPix   = uvec2(gl_FragCoord.xy);
  gFrame = ANIMATE_NOISE ? uint(floor(uTime * 60.0)) : 0u;

  ivec2 ip     = ivec2(gl_FragCoord.xy);
  vec2  fragPx = gl_FragCoord.xy;
  float frame  = float(gFrame);

  float d0  = depthAt(ip);
  bool  sky = d0 >= 0.99999;
  vec3  P   = viewPosPx(ip, d0);
  float viewDepth = -P.z;

  // ---------- depth of field on the raw scene ----------
  float focus  = focusDistance();
  float coc    = cocRadiusPx(viewDepth, focus);
  float dofRot = ign(fragPx + vec2(13.0, 7.0) + 5.588238 * frame);   // FIX: IGN rotation instead of white noise
  vec3  col    = dofGather(vUV, coc, viewDepth, focus, dofRot);
  float sharp  = 1.0 - smoothstep(1.0, 6.0, coc);   // fade surface effects where the image is blurred

  vec3 base = sceneColor(vUV);

  if(!sky) {
    vec3  N   = reconstructNormal(ip, d0, P);
    vec3  V   = normalize(-P);
    float NoV = clamp(dot(N, V), 0.001, 1.0);

    vec3  chroma = base / max(max3(base), 1e-3);
    vec3  albedo = chroma * 0.6;                         // chroma-only albedo estimate
    float directness = smoothstep(0.25, 2.0, luma(base));
    vec3  rayOrg = P + N * (0.003 * viewDepth + 0.005);  // bias off the surface

    // ---------- ambient occlusion ----------
    float vis = gtao(fragPx, P, N, ign(fragPx + 5.588238 * frame), rnd(1u));
    vis = pow(vis, AO_POWER);
    vis = mix(1.0, vis, sharp);
    vec3 aoTerm = multiBounce(vis, albedo);
    col *= mix(aoTerm, vec3(1.0), 0.4 * directness);    // lit surfaces rely less on ambient occlusion

    // ---------- one-bounce screen-space GI ----------
    if(GI_RAYS > 0) {
      vec3 gi = vec3(0.0);
      float ga = rnd(5u), gb = rnd(6u), gj = rnd(7u);
      for(int r = 0; r < GI_RAYS; r++) {
        vec2 xi = vec2(fract(ga + float(r) * 0.7548776662), fract(gb + float(r) * 0.5698402910));
        vec3 L = cosineHemisphere(xi, N);
        ivec2 hp; float hd;
        if(traceRay(rayOrg, L, GI_MAX_DIST, GI_STEPS, fract(gj + float(r) * 0.618034), GI_THICKNESS, hp, hd)) {
          vec2 huv = (vec2(hp) + 0.5) / vec2(gSize);
          float w = edgeFade(huv) * (1.0 - smoothstep(0.5 * GI_MAX_DIST, GI_MAX_DIST, hd));
          gi += clampRadiance(sceneColor(huv), GI_MAX_RADIANCE) * w;   // FIX: hue-preserving clamp
        }
      }
      col += gi / float(max(GI_RAYS, 1)) * albedo * GI_STRENGTH * sharp;
    }

    // ---------- glossy screen-space reflections ----------
    if(SSR_RAYS > 0) {   // FIX: the fast tier has 0 rays, which used to divide by zero and trip the NaN guard
      float alpha = SSR_ROUGHNESS * SSR_ROUGHNESS;
      float sa = rnd(3u), sb = rnd(4u), sj = rnd(2u);
      vec3 refl = vec3(0.0);
      for(int r = 0; r < SSR_RAYS; r++) {
        vec2 xi = vec2(fract(sa + float(r) * 0.7548776662), fract(sb + float(r) * 0.5698402910));
        vec3 H = sampleGGX(xi, N, alpha);
        vec3 R = reflect(-V, H);
        if(dot(R, N) < 0.01) { refl += ENV_COLOR; continue; }

        ivec2 hp; float hd;
        if(traceRay(rayOrg, R, SSR_MAX_DIST, SSR_STEPS, fract(sj + float(r) * 0.618034), SSR_THICKNESS, hp, hd)) {
          vec2 huv = (vec2(hp) + 0.5) / vec2(gSize);
          float w = edgeFade(huv) * (1.0 - clamp(hd / SSR_MAX_DIST, 0.0, 1.0));
          refl += mix(ENV_COLOR, clampRadiance(sceneColor(huv), SSR_MAX_RADIANCE), w);   // FIX: hue-preserving clamp
        } else {
          refl += ENV_COLOR;
        }
      }
      refl /= float(SSR_RAYS);

      float fres    = SSR_F0 + (1.0 - SSR_F0) * pow(1.0 - NoV, 5.0);
      float specOcc = clamp(pow(NoV + vis, exp2(-16.0 * SSR_ROUGHNESS * SSR_ROUGHNESS - 1.0)) - 1.0 + vis, 0.0, 1.0);
      col = mix(col, refl, clamp(fres * specOcc * sharp, 0.0, 1.0));
    }

    // ---------- fog ----------
    vec3 rayW = transpose(mat3(uView)) * normalize(P);
    col = applyFog(col, length(P), rayW);
  }

  if(any(isnan(col)) || any(isinf(col))) col = base;   // never let a NaN poison the bloom/grade

  // ---------- bloom (3 scales) ----------
  {
    float rot = ign(fragPx + vec2(29.0, 3.0) + 5.588238 * frame);   // FIX: IGN rotation instead of white noise
    float H = float(gSize.y);
    vec3 bloom = 0.50 * bloomRing(BLOOM_RADII.x * H, rot)
               + 0.30 * bloomRing(BLOOM_RADII.y * H, rot + 0.37)
               + 0.20 * bloomRing(BLOOM_RADII.z * H, rot + 0.71);
    col += bloom * BLOOM_AMOUNT;
  }

  // ---------- lateral chromatic aberration (grows with radius) ----------
  {
    vec2 cc  = vUV - 0.5;
    vec2 off = cc * CA_AMOUNT;
    vec3 c0 = sceneColor(vUV);
    col.r += (sceneColor(vUV + off).r - c0.r) * sharp;
    col.b += (sceneColor(vUV - off).b - c0.b) * sharp;
    col = max(col, 0.0);
  }

  // ---------- natural (cos^4) vignette ----------
  {
    vec2 ndc = vUV * 2.0 - 1.0;
    vec3 rd  = normalize(vec3((ndc + vec2(uProj[2][0], uProj[2][1])) / vec2(uProj[0][0], uProj[1][1]), -1.0));
    float c2 = rd.z * rd.z;
    col *= mix(1.0, c2 * c2, VIGNETTE);
  }

  // ---------- tonemap + grade ----------
  col = aces(col * EXPOSURE / 0.6);
  col = linearToSrgb(col);

  float l = luma(col);
  col = mix(vec3(l), col, SATURATION);
  col = mix(col, smoothstep(0.0, 1.0, col), CONTRAST);
  col *= mix(vec3(0.97, 1.0, 1.03), vec3(1.03, 1.0, 0.97), smoothstep(0.15, 0.85, l));   // cool shadows, warm highlights

  // ---------- film grain (animated, strongest in midtones) ----------
  {
    float gt = floor(uTime * 24.0);
    float g1 = ign(fragPx + 13.37 * gt) - 0.5;
    float g2 = ign(fragPx.yx + 7.77 * gt + 91.0) - 0.5;
    float lm = luma(col);
    float gAmt = GRAIN * (0.4 + 0.6 * (1.0 - abs(2.0 * lm - 1.0)));
    col += (g1 + vec3(g2, -g2, g2 * 0.5) * 0.35) * gAmt;
  }

  // ---------- triangular dither (kills 8-bit banding) ----------
  col += (rnd(30u) - rnd(31u)) / 255.0;

  fragColor = vec4(clamp(col, 0.0, 1.0), 1.0);
}