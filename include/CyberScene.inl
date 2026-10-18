// =====================================================================
//  CyberScene.inl  -  "Neo-Vice": a rain-slicked neon boulevard at dusk
//
//  HOW TO USE
//    1. #include "CyberScene.inl" in main.cpp AFTER the static declarations
//       (scene, litShader, unlitShader, skyboxShader, camera, keyLightBase).
//    2. Replace the old "Create Scene" block in init() with:  CreateCyberScene();
//    3. Replace the lights[0] animation in your update block (see the notes).
//
//  COMPOSITION
//    You stand in the right-hand lane of a four-lane avenue, a magenta coupe
//    stopped 6 m ahead (autofocus lands on it). The street runs straight at the
//    setting sun (+z). Neon megablocks flank it, palms and pedestrians line the
//    sidewalks, an elevated rail crosses overhead, and a patrol drone (the only
//    shadow-casting light) circles above the queue of cars.
//
//  COORDINATES
//    at(r, u, f): r = metres to the SCREEN-RIGHT of the street centre line,
//    u = up, f = forward (towards the sun). Camera yaw is 0, so the sun stays
//    horizontally centred wherever you slide the camera sideways.
// =====================================================================
#include <algorithm>
#include <random>
#include <vector>
#include <glm/gtc/quaternion.hpp>

static size_t droneObject = 0;   // index of the drone in scene->objects, moved every frame from main

namespace CyberScene {
  using glm::vec2; using glm::vec3; using glm::vec4; using glm::quat;

  constexpr bool  kFrontCW = true;      // set to false if the generated geometry renders inside-out or vanishes
  constexpr float kPi      = 3.14159265f;
  constexpr float kTau     = 6.28318531f;
  constexpr float kFace    = 15.5f;     // |r| of the building street faces (road is +-9 m, sidewalks reach ~16 m)

  static const quat kIdent(1.f, 0.f, 0.f, 0.f);
  static const vec3 kUp(0.f, 1.f, 0.f);
  static const vec3 kX(1.f, 0.f, 0.f);

  static vec3 at(float r, float u, float f) { return vec3(-r, u, f); }   // facing +z, screen-right is world -x

  // ---------------------------------------------------------------------
  //  Deterministic randomness so the city is identical every run
  // ---------------------------------------------------------------------
  struct Rng {
    std::mt19937 gen;
    explicit Rng(unsigned seed) : gen(seed) {}
    float f(float a, float b) { return std::uniform_real_distribution<float>(a, b)(gen); }
    int   i(int a, int b)     { return std::uniform_int_distribution<int>(a, b)(gen); }   // inclusive
    bool  chance(float p)     { return f(0.f, 1.f) < p; }
  };

  static vec3 neonColor(Rng &rng) {
    static const vec3 palette[] = {
      {1.00f, 0.10f, 0.65f},   // hot magenta
      {0.05f, 0.90f, 1.00f},   // cyan
      {1.00f, 0.55f, 0.08f},   // sunset orange
      {0.55f, 0.25f, 1.00f},   // violet
      {0.20f, 1.00f, 0.55f},   // acid green (rare)
    };
    return palette[rng.chance(.08f) ? 4 : rng.i(0, 3)];
  }
  static vec3 windowColor(Rng &rng) {   // peaks at 0.85 so windows glow without all of them blooming
    const float p = rng.f(0.f, 1.f);
    if(p < .55f) return {.85f, .62f, .34f};   // warm office
    if(p < .75f) return {.50f, .68f, .85f};   // cold office
    if(p < .85f) return {.05f, .70f, .80f};   // cyan
    if(p < .93f) return {.80f, .12f, .50f};   // magenta
    return {.55f, .30f, .80f};                // violet
  }

  // ---------------------------------------------------------------------
  //  MeshBuilder: merges many primitives into one mesh (one draw call each).
  //  Winding is derived from the supplied normals, so corner order only has
  //  to be cyclic. Vertex colours are the albedo (lit) or the emission (unlit).
  // ---------------------------------------------------------------------
  struct MeshBuilder {
    std::vector<Renderer::Vertex> vertices;
    std::vector<unsigned int> indices;

    unsigned int vert(vec3 p, vec3 n, vec4 c, vec2 uv = vec2(0.f)) {
      vertices.push_back(Renderer::Vertex{p, c, n, uv});
      return (unsigned int)vertices.size() - 1;
    }
    void tri(unsigned int a, unsigned int b, unsigned int c) {
      const vec3 &A = vertices[a].position, &B = vertices[b].position, &C = vertices[c].position;
      const vec3 n = vertices[a].normal + vertices[b].normal + vertices[c].normal;
      const bool ccw = glm::dot(glm::cross(B - A, C - A), n) > 0.f;
      if(ccw == kFrontCW) std::swap(b, c);
      indices.push_back(a); indices.push_back(b); indices.push_back(c);
    }
    void quad(vec3 a, vec3 b, vec3 c, vec3 d, vec3 na, vec3 nb, vec3 nc, vec3 nd, vec4 col) {
      const unsigned int ia = vert(a, na, col, vec2(0, 0)), ib = vert(b, nb, col, vec2(1, 0));
      const unsigned int ic = vert(c, nc, col, vec2(1, 1)), id = vert(d, nd, col, vec2(0, 1));
      tri(ia, ib, ic); tri(ia, ic, id);
    }
    void quad(vec3 a, vec3 b, vec3 c, vec3 d, vec3 n, vec4 col) { quad(a, b, c, d, n, n, n, n, col); }

    void box(vec3 c, vec3 size, vec4 col, quat q = quat(1.f, 0.f, 0.f, 0.f)) {
      const vec3 h = size * .5f;
      auto P = [&](float x, float y, float z) { return c + q * vec3(h.x * x, h.y * y, h.z * z); };
      quad(P( 1,-1,-1), P( 1, 1,-1), P( 1, 1, 1), P( 1,-1, 1), q * vec3( 1, 0, 0), col);
      quad(P(-1,-1, 1), P(-1, 1, 1), P(-1, 1,-1), P(-1,-1,-1), q * vec3(-1, 0, 0), col);
      quad(P(-1, 1,-1), P(-1, 1, 1), P( 1, 1, 1), P( 1, 1,-1), q * vec3( 0, 1, 0), col);
      quad(P(-1,-1, 1), P(-1,-1,-1), P( 1,-1,-1), P( 1,-1, 1), q * vec3( 0,-1, 0), col);
      quad(P(-1,-1, 1), P( 1,-1, 1), P( 1, 1, 1), P(-1, 1, 1), q * vec3( 0, 0, 1), col);
      quad(P( 1,-1,-1), P(-1,-1,-1), P(-1, 1,-1), P( 1, 1,-1), q * vec3( 0, 0,-1), col);
    }

    // Cylinder (or cone frustum) along local +Y, centred on c.
    void cylinder(vec3 c, float r, float h, vec4 col, quat q = quat(1.f, 0.f, 0.f, 0.f), int seg = 8, float topR = -1.f) {
      if(topR < 0.f) topR = r;
      const vec3 lo(0.f, -h * .5f, 0.f), hi(0.f, h * .5f, 0.f);
      const vec3 nTop = q * vec3(0.f, 1.f, 0.f), nBot = -nTop;
      for(int i = 0; i < seg; ++i) {
        const float a0 = kTau * (float)i / (float)seg, a1 = kTau * (float)(i + 1) / (float)seg;
        const vec3 d0(std::cos(a0), 0.f, std::sin(a0)), d1(std::cos(a1), 0.f, std::sin(a1));
        const vec3 p00 = c + q * (lo + d0 * r),    p10 = c + q * (lo + d1 * r);
        const vec3 p11 = c + q * (hi + d1 * topR), p01 = c + q * (hi + d0 * topR);
        quad(p00, p10, p11, p01, q * d0, q * d1, q * d1, q * d0, col);
        { const unsigned int t0 = vert(c + q * hi, nTop, col), t1 = vert(p01, nTop, col), t2 = vert(p11, nTop, col); tri(t0, t1, t2); }
        { const unsigned int b0 = vert(c + q * lo, nBot, col), b1 = vert(p00, nBot, col), b2 = vert(p10, nBot, col); tri(b0, b1, b2); }
      }
    }

    Renderer::Mesh build() const {
      Renderer::Mesh m;
      m.setVertices(vertices);
      m.setIndices(indices);
      return m;
    }
  };

  struct Builders {
    MeshBuilder ground;      // lit, receives shadows only
    MeshBuilder buildings;   // lit, receives shadows only
    MeshBuilder props;       // lit, CASTS shadows (cars, palms, people, poles)
    MeshBuilder windows;     // unlit emissive
    MeshBuilder glow;        // unlit emissive (neon, lamps, tail lights, signs)
  };

  // ---------------------------------------------------------------------
  //  Emissive window grid on one facade. corner = bottom-left, n = outward normal.
  //  Each floor gets its own "occupancy", so you get horizontal bands of light.
  // ---------------------------------------------------------------------
  static void windowGrid(MeshBuilder &wb, Rng &rng, vec3 corner, vec3 along, float width, float height, vec3 n, float litBias) {
    const float cw = 3.f, ch = 3.6f, ww = 1.8f, wh = 1.9f;
    const int cols = (int)((width - 1.2f) / cw), rows = (int)((height - 1.0f) / ch);
    if(cols < 1 || rows < 1) return;
    const float x0 = (width - (float)cols * cw) * .5f + (cw - ww) * .5f;
    for(int j = 0; j < rows; ++j) {
      const float rowBias = std::min(.85f, litBias * rng.f(.15f, 1.7f));
      for(int i = 0; i < cols; ++i) {
        if(!rng.chance(rowBias)) continue;
        const vec3 p = corner + along * (x0 + (float)i * cw) + kUp * (.6f + (float)j * ch) + n * .06f;
        wb.quad(p, p + along * ww, p + along * ww + kUp * wh, p + kUp * wh, n, vec4(windowColor(rng), 1.f));
      }
    }
  }

  // ---------------------------------------------------------------------
  //  Cars. Local +z is the front. facingCamera = true yaws it 180 degrees so
  //  you see headlights; false shows the tail (brake lights on).
  // ---------------------------------------------------------------------
  static void addCar(Builders &B, vec3 pos, bool facingCamera, vec4 paint, vec3 underglow) {
    const quat q = glm::angleAxis(facingCamera ? kPi : 0.f, kUp);
    auto P = [&](float x, float y, float z) { return pos + q * vec3(x, y, z); };
    const vec4 glass(.015f, .02f, .03f, 1.f), tyre(.02f, .02f, .022f, 1.f);

    B.props.box(P(0, .52f, 0),      vec3(1.86f, .50f, 4.5f), paint, q);        // lower body
    B.props.box(P(0, 1.0f, -.2f),   vec3(1.50f, .44f, 2.0f), glass, q);        // glasshouse
    B.props.box(P(0, 1.24f, -.2f),  vec3(1.46f, .05f, 1.6f), paint, q);        // roof
    B.props.box(P(0, 1.06f, -2.15f), vec3(1.70f, .06f, .42f), paint, q);       // spoiler
    const quat wq = q * glm::angleAxis(kPi * .5f, vec3(0.f, 0.f, 1.f));        // wheel axis -> x
    for(float sx : {-.93f, .93f}) for(float sz : {-1.4f, 1.4f})
      B.props.cylinder(P(sx, .36f, sz), .36f, .26f, tyre, wq, 10);

    B.glow.box(P(0, .70f, -2.27f),  vec3(1.50f, .09f, .05f), vec4(1.f, .03f, .02f, 1.f), q);    // tail light bar
    B.glow.box(P(0, 1.14f, -1.32f), vec3(.60f, .05f, .05f),  vec4(1.f, .03f, .02f, 1.f), q);    // third brake light
    for(float sx : {-.62f, .62f})
      B.glow.box(P(sx, .64f, 2.27f), vec3(.50f, .10f, .05f), vec4(.90f, .96f, 1.f, 1.f), q);    // headlights
    B.glow.box(P(0, .17f, 0),       vec3(1.70f, .03f, 4.0f), vec4(underglow, 1.f), q);          // underglow strip
  }

  // ---------------------------------------------------------------------
  //  Palms: a tapered, slightly leaning trunk with drooping two-segment fronds
  // ---------------------------------------------------------------------
  static void addPalm(Builders &B, Rng &rng, vec3 base) {
    const float h = rng.f(6.5f, 9.f);
    const quat lean = glm::angleAxis(rng.f(-.1f, .1f), vec3(0.f, 0.f, 1.f)) * glm::angleAxis(rng.f(-.1f, .1f), kX);
    const vec3 crown = base + lean * vec3(0.f, h, 0.f);
    B.props.cylinder(base + lean * vec3(0.f, h * .5f, 0.f), .22f, h, vec4(.16f, .12f, .09f, 1.f), lean, 8, .13f);
    for(int k = 0; k < 9; ++k) {
      const float yaw = kTau * (float)k / 9.f + rng.f(-.25f, .25f);
      const float d1 = rng.f(.1f, .35f), d2 = d1 + rng.f(.5f, .8f), len = 1.9f;
      const quat qy = glm::angleAxis(yaw, kUp);
      const quat q1 = qy * glm::angleAxis(d1, kX), q2 = qy * glm::angleAxis(d2, kX);
      const vec3 e1 = crown + q1 * vec3(0.f, 0.f, len);
      const vec4 leaf(.03f, rng.f(.10f, .20f), .06f, 1.f);
      B.props.box(crown + q1 * vec3(0.f, 0.f, len * .5f), vec3(.7f, .05f, len), leaf, q1);
      B.props.box(e1 + q2 * vec3(0.f, 0.f, len * .5f),    vec3(.5f, .04f, len), leaf, q2);
    }
  }

  static void addLamp(Builders &B, int side, float f) {
    const float r = (float)side * 9.7f, arm = 2.2f;
    const vec4 metal(.05f, .05f, .06f, 1.f);
    B.props.cylinder(at(r, 4.5f, f), .11f, 9.f, metal, kIdent, 8, .08f);
    B.props.box(at(r - (float)side * arm * .5f, 8.9f, f), vec3(arm, .12f, .12f), metal);
    B.glow.box(at(r - (float)side * (arm - .3f), 8.78f, f), vec3(.9f, .08f, .4f), vec4(1.f, .72f, .42f, 1.f));
  }

  // Overhead traffic signal: red for our direction, which is why the lane ahead is a queue of brake lights
  static void addSignal(Builders &B, float f) {
    const vec4 metal(.04f, .04f, .05f, 1.f);
    B.props.cylinder(at(10.4f, 3.7f, f), .15f, 7.4f, metal, kIdent, 8);
    B.props.box(at(4.45f, 7.4f, f), vec3(11.9f, .22f, .22f), metal);
    for(float r : {6.2f, -.3f}) {
      B.props.box(at(r, 6.4f, f), vec3(.55f, 1.4f, .42f), metal);
      B.glow.box(at(r, 6.8f, f - .24f), vec3(.34f, .34f, .06f), vec4(1.f, .05f, .04f, 1.f));    // red (on)
      B.glow.box(at(r, 6.4f, f - .24f), vec3(.34f, .34f, .06f), vec4(.16f, .10f, .0f, 1.f));    // amber (off)
      B.glow.box(at(r, 6.0f, f - .24f), vec3(.34f, .34f, .06f), vec4(.0f, .14f, .06f, 1.f));    // green (off)
    }
  }

  static void addPerson(Builders &B, Rng &rng, vec3 pos, bool umbrella) {
    static const vec3 coat[] = {{.5f, .05f, .3f}, {.05f, .3f, .4f}, {.08f, .08f, .1f}, {.35f, .2f, .05f}, {.2f, .2f, .25f}};
    const quat q = glm::angleAxis(rng.f(0.f, kTau), kUp);
    B.props.box(pos + vec3(0.f, .42f, 0.f),  vec3(.34f, .84f, .22f), vec4(.03f, .03f, .04f, 1.f), q);
    B.props.box(pos + vec3(0.f, 1.15f, 0.f), vec3(.44f, .62f, .26f), vec4(coat[rng.i(0, 4)], 1.f), q);
    B.props.box(pos + vec3(0.f, 1.6f, 0.f),  vec3(.19f, .22f, .20f), vec4(.3f, .2f, .15f, 1.f), q);
    if(umbrella) {   // LED umbrella: glowing underside is what you see from eye level
      B.props.box(pos + vec3(0.f, 1.75f, 0.f), vec3(.03f, .9f, .03f), vec4(.03f, .03f, .03f, 1.f));
      B.props.cylinder(pos + vec3(0.f, 2.07f, 0.f), .64f, .03f, vec4(.03f, .03f, .04f, 1.f), kIdent, 10);
      B.glow.cylinder(pos + vec3(0.f, 2.0f, 0.f), .55f, .02f, vec4(neonColor(rng), 1.f), kIdent, 10);
    }
  }

  // Floating holographic billboard hung over the sidewalk, angled towards the street and the camera
  static void addBillboard(Builders &B, int side, float f, float y, int style) {
    const float W = 13.f, H = 10.f;
    const quat q = glm::angleAxis((float)side * kPi * 2.f / 3.f, kUp);
    const vec3 c(-(float)side * 11.8f, y, f);
    if(style == 0) {   // retro sunset stripes, gaps widening downwards
      float top = H * .5f;
      for(int i = 0; i < 8; ++i) {
        const float h = 1.5f - (float)i * .12f, gap = .12f + (float)i * .09f, t = (float)i / 7.f;
        const vec3 col = glm::mix(vec3(1.f, .8f, .1f), vec3(1.f, .08f, .55f), t);
        B.glow.box(c + vec3(0.f, top - h * .5f, 0.f), vec3(W, h, .3f), vec4(col, 1.f), q);
        top -= h + gap;
      }
    } else {           // cyan ring logo on a deep violet panel
      B.glow.box(c, vec3(W, H, .3f), vec4(.12f, .05f, .4f, 1.f), q);
      const quat dq = q * glm::angleAxis(kPi * .5f, kX);   // cylinder axis -> panel normal
      B.glow.cylinder(c + q * vec3(0.f, 0.f, .20f), 3.6f, .20f, vec4(.05f, .9f, 1.f, 1.f), dq, 24);
      B.glow.cylinder(c + q * vec3(0.f, 0.f, .25f), 2.9f, .30f, vec4(.12f, .05f, .4f, 1.f), dq, 24);
      B.glow.cylinder(c + q * vec3(0.f, 0.f, .30f), 1.3f, .40f, vec4(1.f, .1f, .65f, 1.f), dq, 20);
    }
  }

  // ---------------------------------------------------------------------
  //  Megablock: podium + 1..3 setback tiers, window grids on the two faces the
  //  camera can see (street side and the face towards you), neon trim, corner
  //  strips, shopfronts, a blade sign and a spire with an aircraft beacon.
  //  side = +1 screen-right, -1 screen-left.
  // ---------------------------------------------------------------------
  static void addBuilding(Builders &B, Rng &rng, int side, float f0, float len, float depth, float height) {
    const float s = (float)side;
    const vec3 nStreet(s, 0.f, 0.f);
    const float shade = rng.f(.05f, .09f);
    const vec4 wall(shade, shade, shade * 1.15f, 1.f);
    const float hp = rng.f(6.f, 10.f);
    const int tiers = height > 90.f ? 3 : (height > 45.f ? 2 : 1);
    const float litBias = rng.f(.22f, .5f);

    float y0 = 0.f, setback = 0.f, inset = 0.f;
    float fa = f0, fb = f0 + len, rS = kFace, D = depth;

    for(int t = 0; t <= tiers; ++t) {
      const float h = (t == 0) ? hp : (height - hp) / (float)tiers;
      if(t > 0) {
        setback = std::min(setback + rng.f(1.f, 3.5f), 9.f);
        inset   = std::min(inset + rng.f(1.f, 2.5f), (len - 8.f) * .5f);
      }
      fa = f0 + inset; fb = f0 + len - inset;
      rS = kFace + setback; D = std::max(10.f, depth - setback);
      const float x = -s * rS;

      B.buildings.box(vec3(-s * (rS + D * .5f), y0 + h * .5f, (fa + fb) * .5f), vec3(D, h, fb - fa), wall);

      const float winBase = (t == 0) ? 4.2f : y0;
      const float winH = y0 + h - winBase;
      windowGrid(B.windows, rng, vec3(x, winBase, fa), vec3(0.f, 0.f, 1.f), fb - fa, winH, nStreet, litBias);   // street face
      windowGrid(B.windows, rng, vec3(x, winBase, fa), vec3(-s, 0.f, 0.f), D, winH, vec3(0.f, 0.f, -1.f), litBias);   // face towards the camera

      if(t == 0) {   // glowing shopfronts along the ground floor
        const float sx = -s * (kFace - .05f);
        for(float z = fa + 1.5f; z < fb - 4.f; ) {
          const float w = rng.f(5.f, 9.f), e = std::min(z + w - .8f, fb - 1.5f);
          B.glow.quad(vec3(sx, .4f, z), vec3(sx, .4f, e), vec3(sx, 3.3f, e), vec3(sx, 3.3f, z), nStreet, vec4(neonColor(rng) * .5f, 1.f));
          z += w;
        }
      }
      if(t == 0 || rng.chance(.5f))   // neon trim along the top edge of the tier
        B.glow.box(vec3(-s * (rS - .15f), y0 + h - .2f, (fa + fb) * .5f), vec3(.3f, .35f, fb - fa - .6f), vec4(neonColor(rng), 1.f));
      if(t > 0 && rng.chance(.7f))    // vertical neon strip on the corner nearest the camera
        B.glow.box(vec3(-s * (rS - .12f), y0 + h * .5f, fa + .12f), vec3(.28f, h, .28f), vec4(neonColor(rng), 1.f));
      y0 += h;
    }

    if(height > 40.f) {   // spire + red aircraft beacon
      const float sh = rng.f(6.f, 20.f), cx = -s * (rS + D * .5f), cz = (fa + fb) * .5f;
      B.buildings.box(vec3(cx, y0 + sh * .5f, cz), vec3(.7f, sh, .7f), wall);
      B.glow.box(vec3(cx, y0 + sh + .4f, cz), vec3(1.f), vec4(1.f, .05f, .04f, 1.f));
    }

    if(rng.chance(.65f)) {   // blade sign facing down the street
      const float zf = f0 + rng.f(1.f, 3.f), yc = hp + rng.f(3.5f, 8.f), xr = -s * (kFace - 1.6f);
      const vec3 c1 = neonColor(rng);
      B.buildings.box(vec3(xr, yc, zf), vec3(2.8f, 7.2f, .3f), vec4(.03f, .03f, .04f, 1.f));
      B.glow.box(vec3(xr, yc, zf - .18f), vec3(2.4f, 6.8f, .10f), vec4(c1 * .35f, 1.f));
      for(int k = 0; k < 5; ++k)   // abstract "lettering"
        B.glow.box(vec3(xr, yc + 2.6f - (float)k * 1.3f, zf - .26f), vec3(1.7f * rng.f(.5f, 1.f), .75f, .08f), vec4((k % 2) ? vec3(.95f) : c1, 1.f));
    }
  }

  // Stepped corporate ziggurat with glowing tier bands and an apex beacon
  static void addZiggurat(Builders &B, float r, float f) {
    const vec4 stone(.05f, .05f, .065f, 1.f);
    for(int i = 0; i < 6; ++i) {
      const float w = 66.f - (float)i * 10.f, y0 = (float)i * 12.f;
      B.buildings.box(at(r, y0 + 6.f, f), vec3(w, 12.f, w), stone);
      B.glow.box(at(r, y0 + 10.6f, f), vec3(w + .4f, .9f, w + .4f), vec4(1.f, .62f + (float)i * .03f, .2f, 1.f));
    }
    B.glow.box(at(r, 75.f, f), vec3(2.5f, 6.f, 2.5f), vec4(1.f, .05f, .04f, 1.f));
  }

  static void addMesh(const char *name, const MeshBuilder &mb, Renderer::Shader *shader, bool castShadows) {
    scene->meshes.push_back(mb.build());
    scene->objects.push_back(Renderer::SceneObject{
      name,
      Renderer::MeshRenderer(&scene->meshes.back(), shader),
      Renderer::Transform({0,0,0})
    });
    scene->objects.back().meshRenderer.castShadows = castShadows;
  }
}   // namespace CyberScene


// =====================================================================
//  The scene
// =====================================================================
static void CreateCyberScene() {
  using namespace CyberScene;

  scene.emplace();
  scene->skybox.loadFromFolder(System::PATH + "/assets/skyboxs/DimTwilight");
  scene->skyboxShader = &skyboxShader;
  scene->meshes.reserve(32);   // meshes are referenced by pointer, so this vector must never reallocate

  Rng rng(2077);
  Builders B;

  // ---------- road, sidewalks, markings ----------
  const vec4 asphalt(.028f, .030f, .036f, 1.f), concrete(.11f, .11f, .125f, 1.f);
  const vec4 paintW(.50f, .50f, .46f, 1.f), paintY(.62f, .46f, .06f, 1.f);

  B.ground.box(at(0, -.5f, 225.f), vec3(440.f, 1.f, 550.f), asphalt);                       // dark wet asphalt: this is what the SSR reflects in
  for(int sd : {-1, 1}) B.ground.box(at((float)sd * 12.5f, .08f, 225.f), vec3(7.f, .16f, 550.f), concrete);

  auto line = [&](float r, float w, float f0, float f1, vec4 col) {
    B.ground.box(at(r, .015f, (f0 + f1) * .5f), vec3(w, .03f, f1 - f0), col);
  };
  for(float r : {-.16f, .16f}) { line(r, .10f, -50.f, 26.f, paintY); line(r, .10f, 36.f, 500.f, paintY); }   // double yellow
  for(float r : {-8.6f, 8.6f}) line(r, .16f, -50.f, 500.f, paintW);                                          // edge lines
  for(float f = -10.f; f < 260.f; f += 9.f) {                                                                // lane dashes
    if(f > 24.f && f < 36.f) continue;
    line(-4.5f, .14f, f, f + 3.f, paintW); line(4.5f, .14f, f, f + 3.f, paintW);
  }
  for(int k = 0; k < 15; ++k) B.ground.box(at(-7.7f + (float)k * 1.1f, .015f, 31.f), vec3(.55f, .03f, 3.4f), paintW);   // crosswalk
  B.ground.box(at(4.5f, .015f, 27.4f), vec3(8.6f, .03f, .5f), paintW);                                       // stop line

  // ---------- megablocks lining both sides (central corridor stays clear so the sun is visible) ----------
  for(int sd : {-1, 1}) {
    float f = (sd > 0) ? 3.f : 8.f;
    bool billboard = false;
    while(f < 230.f) {
      float len = rng.f(15.f, 26.f);
      if(f < 62.f && f + len > 45.f) {                    // keep a gap for the elevated rail
        if(44.f - f >= 12.f) len = 44.f - f; else { f = 62.f; continue; }
      }
      const float h = (f < 60.f) ? rng.f(30.f, 62.f) : rng.f(55.f, 150.f);
      addBuilding(B, rng, sd, f, len, rng.f(20.f, 30.f), h);
      if(!billboard && f + len * .5f > 24.f) { addBillboard(B, sd, f + len * .5f, 24.f, sd > 0 ? 0 : 1); billboard = true; }
      f += len + rng.f(3.f, 7.f);
    }
  }

  // ---------- skyline: needles and a ziggurat that fade into the dusk haze ----------
  for(int k = 0; k < 7; ++k) {
    const int sd = (k % 2) ? 1 : -1;
    const float r = (float)sd * rng.f(30.f, 75.f), f = 120.f + (float)k * 14.f + rng.f(-6.f, 6.f), h = rng.f(110.f, 190.f);
    const float w = rng.f(7.f, 12.f), d = rng.f(7.f, 12.f);
    B.buildings.box(at(r, h * .5f, f), vec3(w, h, d), vec4(.05f, .05f, .065f, 1.f));
    B.glow.box(at(r, h + 1.f, f), vec3(1.4f), vec4(1.f, .05f, .04f, 1.f));
    B.glow.box(at(r, h * .5f, f - d * .5f - .05f), vec3(.35f, h * .9f, .2f), vec4(neonColor(rng), 1.f));
  }
  addZiggurat(B, -60.f, 175.f);

  // ---------- elevated rail crossing the avenue ----------
  {
    const float of = 53.f;
    const vec4 slab(.07f, .07f, .085f, 1.f);
    B.buildings.box(at(0, 15.2f, of), vec3(110.f, 2.4f, 11.f), slab);
    for(int sd : {-1, 1}) B.buildings.box(at((float)sd * 11.8f, 7.f, of), vec3(2.4f, 14.f, 2.4f), slab);
    B.glow.box(at(0, 13.95f, of - 3.f), vec3(104.f, .12f, .25f), vec4(.55f, .25f, 1.f, 1.f));   // violet under-deck strip
    B.glow.box(at(0, 13.95f, of + 3.f), vec3(104.f, .12f, .25f), vec4(.05f, .9f, 1.f, 1.f));    // cyan under-deck strip
    B.glow.box(at(0, 16.3f, of - 5.55f), vec3(110.f, .15f, .12f), vec4(1.f, .1f, .65f, 1.f));   // magenta fascia
    B.buildings.box(at(-6.f, 18.4f, of), vec3(34.f, 3.f, 3.6f), vec4(.08f, .08f, .10f, 1.f));    // monorail car
    for(int k = 0; k < 10; ++k)                                                                  // lit train windows
      B.glow.box(at(-20.f + (float)k * 3.2f, 18.9f, of - 1.83f), vec3(2.8f, .9f, .06f), vec4(.75f, .9f, 1.f, 1.f));
  }

  // ---------- street furniture ----------
  for(int sd : {-1, 1}) {
    for(float f = 9.f + ((sd > 0) ? 0.f : 6.f); f < 150.f; f += 13.f) {
      if(f > 47.f && f < 59.f) continue;
      addPalm(B, rng, at((float)sd * 10.8f, .16f, f));
    }
    for(float f = 18.f; f < 140.f; f += 22.f) addLamp(B, sd, f);
  }
  addSignal(B, 29.f);
  for(float f = 3.f; f < 100.f; f += 6.f) {   // lane delineator posts with glowing caps along the centre line
    B.props.cylinder(at(0, .45f, f), .08f, .9f, vec4(.04f, .04f, .05f, 1.f), kIdent, 6);
    B.glow.cylinder(at(0, .92f, f), .09f, .06f, vec4(.05f, .9f, 1.f, 1.f), kIdent, 6);
  }
  for(int k = 0; k < 16; ++k) {
    const int sd = (k % 2) ? 1 : -1;
    addPerson(B, rng, at((float)sd * rng.f(10.8f, 14.f), .16f, rng.f(10.f, 80.f)), rng.chance(.4f));
  }

  // ---------- traffic ----------
  const vec4 magenta(.50f, .04f, .26f, 1.f), white(.55f, .55f, .58f, 1.f), black(.035f, .035f, .04f, 1.f);
  const vec4 teal(.03f, .30f, .33f, 1.f), orange(.60f, .22f, .03f, 1.f), silver(.32f, .33f, .36f, 1.f);
  const vec3 cyanGlow(.05f, .9f, 1.f), pinkGlow(1.f, .1f, .65f), violetGlow(.45f, .3f, 1.f);
  addCar(B, at(2.25f, 0, 6.2f),   false, magenta, violetGlow);   // HERO: autofocus lands on this one
  addCar(B, at(2.25f, 0, 13.0f),  false, white,   cyanGlow);
  addCar(B, at(2.25f, 0, 19.8f),  false, black,   pinkGlow);
  addCar(B, at(6.75f, 0, 9.0f),   false, teal,    pinkGlow);
  addCar(B, at(6.75f, 0, 15.8f),  false, silver,  cyanGlow);
  addCar(B, at(6.75f, 0, 22.6f),  false, orange,  violetGlow);
  addCar(B, at(-2.25f, 0, 24.f),  true,  black,   cyanGlow);     // oncoming: headlights become bokeh
  addCar(B, at(-2.25f, 0, 47.f),  true,  silver,  pinkGlow);
  addCar(B, at(-2.25f, 0, 72.f),  true,  white,   violetGlow);
  addCar(B, at(-6.75f, 0, 16.f),  true,  orange,  violetGlow);
  addCar(B, at(-6.75f, 0, 36.f),  true,  teal,    cyanGlow);
  addCar(B, at(-6.75f, 0, 62.f),  true,  magenta, pinkGlow);

  addMesh("Ground",    B.ground,    &litShader,   false);
  addMesh("Buildings", B.buildings, &litShader,   false);
  addMesh("Props",     B.props,     &litShader,   true);    // the only lit mesh that casts shadows
  addMesh("Windows",   B.windows,   &unlitShader, false);
  addMesh("Neon",      B.glow,      &unlitShader, false);

  // ---------- a giant glowing orb on the skyline, gradient painted into the vertex colours ----------
  {
    Renderer::Mesh orb = Renderer::MeshHelper::GenerateUVSphere(48, 32, 16.f);
    orb.editVertices([](std::vector<Renderer::Vertex> &_vertices) {
      for(Renderer::Vertex &vertex : _vertices) {
        const float t = glm::clamp(vertex.position.y / 16.f * .5f + .5f, 0.f, 1.f);
        vertex.color = vec4(glm::mix(vec3(1.f, .25f, .55f), vec3(.35f, .3f, 1.f), t), 1.f);
      }
    });
    scene->meshes.push_back(std::move(orb));
    scene->objects.push_back(Renderer::SceneObject{
      "Orb",
      Renderer::MeshRenderer(&scene->meshes.back(), &unlitShader),
      Renderer::Transform(at(-40.f, 38.f, 125.f))
    });
    scene->objects.back().meshRenderer.castShadows = false;
  }

  // ---------- patrol drone: carries the shadow-casting light, animated from main ----------
  {
    MeshBuilder d;
    const vec4 hull(.05f, .05f, .06f, 1.f), rotor(.10f, .10f, .12f, 1.f);
    d.cylinder(vec3(0.f), .5f, .22f, hull, kIdent, 12);
    d.box(vec3(0.f, .02f, 0.f), vec3(1.9f, .07f, .12f), hull, glm::angleAxis(kPi * .25f, kUp));
    d.box(vec3(0.f, .02f, 0.f), vec3(1.9f, .07f, .12f), hull, glm::angleAxis(-kPi * .25f, kUp));
    for(float x : {-.67f, .67f}) for(float z : {-.67f, .67f}) d.cylinder(vec3(x, .1f, z), .34f, .025f, rotor, kIdent, 12);
    d.cylinder(vec3(0.f, -.16f, 0.f), .24f, .06f, vec4(1.f, .92f, .75f, 1.f), kIdent, 12);     // lens = the light
    d.box(vec3(.5f, 0.f, 0.f),  vec3(.1f), vec4(1.f, .05f, .05f, 1.f));                         // nav lights
    d.box(vec3(-.5f, 0.f, 0.f), vec3(.1f), vec4(.05f, 1.f, .2f, 1.f));
    scene->meshes.push_back(d.build());
    scene->objects.push_back(Renderer::SceneObject{
      "Drone",
      Renderer::MeshRenderer(&scene->meshes.back(), &unlitShader),
      Renderer::Transform(at(2.25f, 7.f, 16.f), quat(1.f, 0.f, 0.f, 0.f), vec3(2.2f))
    });
    scene->objects.back().meshRenderer.castShadows = false;   // the light sits inside the drone's footprint
    droneObject = scene->objects.size() - 1;
  }

  // ---------- lighting ----------
  scene->ambientLighting = {.5f, .45f, .9f, .03f};   // dim violet dusk fill
  keyLightBase = at(2.25f, 7.f, 16.f);               // centre of the drone's orbit
  scene->lights = {
    new Renderer::PointLight(keyLightBase - vec3(0.f, .4f, 0.f), {1.00f, .82f, .60f, 1}, 24),   // 0: drone, warm white, CASTS SHADOWS
    new Renderer::PointLight(at(-11.f, 10.f, 24.f),              {1.00f, .10f, .60f, 1}, 26),   // 1: magenta wash on the left facade
    new Renderer::PointLight(at( 11.f,  9.f, 40.f),              {.05f,  .85f, 1.0f, 1}, 26),   // 2: cyan wash on the right facade
    new Renderer::PointLight(at(0.f,    4.f, 85.f),              {1.00f, .45f, .12f, 1}, 40),   // 3: amber sunset glow down the avenue
    new Renderer::PointLight(at(2.25f,  .35f, 6.2f),             {.40f,  .30f, 1.0f, 1}, 7),    // 4: violet underglow of the hero car
  };

  // ---------- camera: right lane, low, a few metres behind the hero car, aimed straight down the avenue ----------
  const vec3 camPos = at(2.25f, 1.35f, 0.f), camTarget = at(2.25f, .85f, 6.2f);   // same r => yaw 0 => sun stays centred
  camera->transform = Renderer::Transform(
    camPos,
    Renderer::Transform::LookAtRotation(camPos, camTarget, vec3(0.f, 1.f, 0.f)),
    {1,1,1}
  );
  Renderer::Core::UpdateViewMatrix();

  Renderer::Core::UpdateDynamicLighting(&litShader, *scene);
}
