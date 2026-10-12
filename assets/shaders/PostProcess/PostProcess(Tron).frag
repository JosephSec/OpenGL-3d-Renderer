#version 430 core

in vec2 vUV;
in vec2 vLocalSpaceCoords; // Range: [-1.0, 1.0] across screen bounds

uniform sampler2D uFrame;   // Skybox backdrop texture
uniform float uTime;

// --- 3D CAMERA & VIEWPORT UNIFORMS (INVERSED & ROW-MAJOR) ---
uniform mat4 uView;           // Camera-to-World Matrix (Row-Major Representation)
uniform mat4 uProj;           // Camera Projection Matrix (Row-Major Representation)
uniform vec3 uCameraWorldPos; // True World Space Camera coordinate
uniform vec2 uViewSize;       // Window dimension limits

out vec4 fragColor;

// --- RAYMARCHING CONSTRAINTS ---
const int MAX_RAY_STEPS  = 100;
const float MAX_DISTANCE = 150.0;
const float SURF_DIST    = 0.0025;

// --- QUANTUM SANCTUARY PALETTE ---
const vec3 VOID_BLACK      = vec3(0.005, 0.005, 0.01);
const vec3 LASER_VIOLET    = vec3(1.2, 0.0, 2.0);   // High-energy grid lines
const vec3 SINGULARITY_AMBER = vec3(2.0, 0.8, 0.1); // Core kinetic artifacts
const vec3 MIRROR_STEEL    = vec3(0.10, 0.12, 0.15); // Reflective brutalist pillars

// --- MATH UTILITIES ---
float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123); }

// 2D Rotation matrix
mat2 rot2D(float a) {
    float c = cos(a), s = sin(a);
    return mat2(c, -s, s, c);
}

// Signed Distance Function for a 3D Box
float sdBox(vec3 p, vec3 b) {
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

// Signed Distance Function for a Torus (Ring)
float sdTorus(vec3 p, vec2 t) {
    vec2 q = vec2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}

// --- WORLD SCENE MAP ---
// Returns: vec4(distance, materialID, objectRefPropertiesX, objectRefPropertiesY)
vec4 MapVoidSanctuary(vec3 p) {
    float d = 1e10;
    float matID = 0.0;
    vec2 props = vec2(0.0);

    // 1. REPEATING BRUTALIST PILLARS (Grid repetition along X and Z)
    vec3 pillarPos = p;
    vec2 gridScale = vec2(12.0, 12.0);
    vec2 cellID = floor(p.xz / gridScale);
    pillarPos.xz = mod(p.xz, gridScale) - gridScale * 0.5;

    // Generate random seed for each pillar structure
    float cellSeed = hash21(cellID);
    
    // Procedurally animate the vertical movement of pillars based on cell ID
    float verticalShift = sin(uTime * 1.5 + cellSeed * 20.0) * 4.0;
    vec3 boxSpace = pillarPos - vec3(0.0, verticalShift, 0.0);
    
    // Render infinite pillars extending vertically
    float pillarD = sdBox(boxSpace, vec3(1.8, 25.0, 1.8));

    // Carve geometric horizontal notches into the pillars
    float horizontalNotches = step(0.90, sin(p.y * 1.5));
    pillarD += horizontalNotches * 0.08;

    if (pillarD < d) {
        d = pillarD;
        matID = 1.0; // Mirrored Brutalist Pillar
        props = vec2(cellSeed, boxSpace.y);
    }

    // 2. KINETIC QUANTUM RINGS (Suspended in the center spaces of the grid)
    vec3 ringPos = pillarPos; // Reuse local cell coordinates
    ringPos.y -= (verticalShift * -0.5); // Counter-animate vertically
    
    // Rotate the rings continuously over time
    ringPos.xy *= rot2D(uTime * 1.2 + cellSeed);
    ringPos.yz *= rot2D(uTime * 0.6);

    float ringD = sdTorus(ringPos, vec2(3.2, 0.15));
    
    // Mask out random cells to keep empty open pathways
    if (cellSeed < 0.25) ringD = 1e10;

    if (ringD < d) {
        d = ringD;
        matID = 2.0; // Glowing Quantum Ring
        props.x = cellSeed;
    }

    // 3. INFINITE LASER ENERGY FLOOR
    float floorD = p.y - (-12.0);
    if (floorD < d) {
        d = floorD;
        matID = 3.0; // Energy Laser Floor
    }

    return vec4(d, matID, props.x, props.y);
}

// Finite differences normal vector sampler
vec3 GetSanctuaryNormal(vec3 p) {
    vec2 e = vec2(0.005, 0.0);
    return normalize(vec3(
        MapVoidSanctuary(p + e.xyy).x - MapVoidSanctuary(p - e.xyy).x,
        MapVoidSanctuary(p + e.yxy).x - MapVoidSanctuary(p - e.yxy).x,
        MapVoidSanctuary(p + e.yyx).x - MapVoidSanctuary(p - e.yyx).x
    ));
}

void main() {
    // ==========================================
    // 1. INVERSED ROW-MAJOR VIEW-RAY RECONSTRUCTION
    // ==========================================
    float aspect = uViewSize.x / (uViewSize.y + 0.0001);
    vec3 viewDir = vec3(vLocalSpaceCoords.x * aspect, vLocalSpaceCoords.y, -1.0);

    // Vector-on-left multiplication executes Row-Major Camera-to-World conversion stably
    vec3 rayDir = normalize((vec4(viewDir, 0.0) * uView).xyz);
    vec3 rayOrigin = uCameraWorldPos;
    vec3 sceneSky = texture(uFrame, vUV).rgb;

    // ==========================================
    // 2. CORE RAYMARCHING LOOP EXECUTION
    // ==========================================
    float totalDistance = 0.0;
    vec4 sceneData = vec4(0.0);
    bool hitSurface = false;

    for (int i = 0; i < MAX_RAY_STEPS; i++) {
        vec3 samplePoint = rayOrigin + rayDir * totalDistance;
        sceneData = MapVoidSanctuary(samplePoint);
        
        if (sceneData.x < SURF_DIST) {
            hitSurface = true;
            break;
        }
        if (totalDistance > MAX_DISTANCE) break;
        totalDistance += sceneData.x;
    }

    // ==========================================
    // 3. ABSTRACT SURFACE AND LIGHT SHADING
    // ==========================================
    vec3 outputColor = vec3(0.0);

    if (hitSurface) {
        vec3 hitPoint = rayOrigin + rayDir * totalDistance;
        float materialType = sceneData.y;
        vec2 objProps = sceneData.zw;
        vec3 normal = GetSanctuaryNormal(hitPoint);

        // MATERIAL 1: BRUTALIST PILLARS
        if (materialType == 1.0) {
            vec3 lightDir = normalize(vec3(0.5, 0.8, -0.2));
            float diffuse = max(0.06, dot(normal, lightDir));
            outputColor = MIRROR_STEEL * diffuse;

            // Sharp mirror fresnel specular sheen on the pillar edges
            float specular = pow(max(0.0, dot(reflect(rayDir, normal), lightDir)), 16.0);
            outputColor += vec3(specular * 0.4);

            // Emissive circuit matrix lines running up structural edges
            float edgeLines = step(0.94, max(abs(sin(hitPoint.x * 0.5)), abs(sin(hitPoint.z * 0.5))));
            float wavePulse = step(0.2, sin(objProps.y * 0.2 - uTime * 3.0));
            outputColor += LASER_VIOLET * edgeLines * wavePulse * 2.0;
        }

        // MATERIAL 2: QUANTUM RINGS
        if (materialType == 2.0) {
            // Animate internal energy cores inside the rings
            float pulseGlow = 0.7 + 0.3 * sin(uTime * 5.0 + objProps.x * 12.0);
            outputColor = SINGULARITY_AMBER * pulseGlow * 5.0;
        }

        // MATERIAL 3: LASER FIELD FLOOR
        if (materialType == 3.0) {
            // Render a geometric neon network mapping grid across the floor coordinates
            float laserGrid = step(0.97, max(fract(hitPoint.x * 0.2), fract(hitPoint.z * 0.2)));
            float fadeFactor = smoothstep(40.0, 0.0, length(hitPoint.xz - rayOrigin.xz));
            
            outputColor = LASER_VIOLET * laserGrid * fadeFactor * 6.0;
        }

        // ==========================================
        // 4. ATMOSPHERIC SHADOW FOG
        // ==========================================
        float voidFog = smoothstep(20.0, MAX_DISTANCE, totalDistance);
        outputColor = mix(outputColor, VOID_BLACK, voidFog);
    } else {
        outputColor = mix(sceneSky, VOID_BLACK, 0.75);
    }

    // Exposure tone mapping and final gamma management layout
    outputColor = outputColor / (outputColor + vec3(0.14));
    outputColor = pow(outputColor, vec3(1.0 / 2.2));
    
    fragColor = vec4(outputColor, 1.0);
}