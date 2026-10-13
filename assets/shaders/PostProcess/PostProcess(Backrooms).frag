#version 430 core

in vec2 vUV;

uniform sampler2D uFrame;         // Current viewport frame buffer
uniform float uTime;             // Time elapsed in seconds
uniform mat4 uView;              // Camera view matrix
uniform mat4 uProj;              // Camera projection matrix
uniform vec3 uCameraWorldPos;    // Camera position in world space

out vec4 fragColor;

// --- RAYMARCHING PARAMETERS (HIGH QUALITY) ---
#define MAX_STEPS 200
#define MAX_DIST 50.0            
#define SURF_DIST 0.001
#define SHADOW_STEPS 25

// --- ADJUSTABLE VISUAL OVERLAYS ---
const float GRAIN_STRENGTH = 0.015;              // Global controls for camera noise overlay

// --- COLOR PALETTE (DARKENED ALBEDOS) ---
const vec3 C_WALL = vec3(0.55, 0.49, 0.25);       
const vec3 C_CARPET = vec3(0.35, 0.29, 0.18);     
const vec3 C_CEILING = vec3(0.45, 0.43, 0.38);    
const vec3 C_LIGHT_ON = vec3(1.0, 0.93, 0.75) * 1.8; 
const vec3 C_LIGHT_OFF = vec3(0.12, 0.12, 0.10);  // Dark, dead fluorescent tube look

// Hash functions for procedural generation & noise
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

float noise(in vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f*f*(3.0-2.0*f);
    return mix(mix(hash(i + vec2(0.0,0.0)), hash(i + vec2(1.0,0.0)), u.x),
               mix(hash(i + vec2(0.0,1.0)), hash(i + vec2(1.0,1.0)), u.x), u.y);
}

// Signed Distance Field Primitives
float sdBox(vec3 p, vec3 b) {
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

// Helper to determine the behavior state of a specific light instance
// 0.0 = completely dead, 0.1 to 0.9 = erratic high-frequency flicker, 1.0 = fully working
float getLightStatus(vec2 lightID) {
    float hStatus = hash(lightID + vec2(42.31, 87.19));
    
    if (hStatus < 0.15) {
        return 0.0; // 15% of all lights are entirely burnt out
    } 
    else if (hStatus < 0.35) {
        // 20% of lights are dying, sparking violently using high-frequency noise
        float spark = hash(lightID + vec2(floor(uTime * 35.0))); 
        return step(0.7, spark) * mix(0.2, 1.0, hash(lightID + vec2(floor(uTime * 8.0))));
    }
    
    return 1.0; // Remaining lights work normally
}

// --- MAP GENERATION (THE BACKROOMS) ---
vec2 map(vec3 p) {
    float floorDist = p.y + 1.5;
    float ceilDist = 1.5 - p.y;
    
    vec2 res = vec2(floorDist, 1.0); 
    if (ceilDist < res.x) res = vec2(ceilDist, 2.0); 
    
    vec3 pMod = p;
    float cellWidth = 6.0;
    vec2 id = floor((pMod.xz + vec2(cellWidth * 0.5)) / cellWidth);
    
    pMod.xz = mod(pMod.xz + vec2(cellWidth * 0.5), vec2(cellWidth)) - vec2(cellWidth * 0.5);
    
    float h = hash(id);
    float wallMask = 0.0;
    
    if (h < 0.35) {
        wallMask = sdBox(vec3(pMod.x, pMod.y, pMod.z), vec3(0.4, 1.5, 2.5));
    } else if (h < 0.7) {
        wallMask = sdBox(vec3(pMod.x, pMod.y, pMod.z), vec3(2.5, 1.5, 0.4));
    } else {
        wallMask = sdBox(vec3(pMod.x + 2.0, pMod.y, pMod.z - 2.0), vec3(0.4, 1.5, 0.4));
    }
    
    if (length(id) < 1.5) {
        wallMask = max(wallMask, -sdBox(p, vec3(4.0, 2.0, 4.0)));
    }

    if (wallMask < res.x) res = vec2(wallMask, 3.0);
    
    vec3 pLight = p;
    pLight.xz = mod(pLight.xz + vec2(3.0), vec2(6.0)) - vec2(3.0); 
    float lights = sdBox(vec3(pLight.x, pLight.y - 1.48, pLight.z), vec3(1.2, 0.03, 0.6));
    
    if (lights < res.x) res = vec2(lights, 4.0);

    return res;
}

vec3 getNormal(vec3 p) {
    vec2 e = vec2(0.0005, 0.0);
    return normalize(vec3(
        map(p + e.xyy).x - map(p - e.xyy).x,
        map(p + e.yxy).x - map(p - e.yxy).x,
        map(p + e.yyx).x - map(p - e.yyx).x
    ));
}

float getShadow(vec3 ro, vec3 rd, float mint, float maxt, float k) {
    float res = 1.0;
    float t = mint;
    for(int i = 0; i < SHADOW_STEPS; i++) {
        float h = map(ro + rd * t).x;
        if(h < 0.001) return 0.0;
        res = min(res, k * h / t);
        t += clamp(h, 0.01, 0.2);
        if(t > maxt) break;
    }
    return clamp(res, 0.0, 1.0);
}

float getAO(vec3 p, vec3 n) {
    float occ = 0.0;
    float sca = 1.0;
    for(int i = 0; i < 5; i++) {
        float hr = 0.01 + 0.12 * float(i) / 4.0;
        vec3 aopos = n * hr + p;
        float dd = map(aopos).x;
        occ += -(dd - hr) * sca;
        sca *= 0.95;
    }
    return clamp(1.0 - 3.5 * occ, 0.0, 1.0);
}

void main() {
    mat4 invProj = inverse(uProj);
    mat4 invView = inverse(uView);
    
    vec4 clipPos = vec4(vUV * 2.0 - 1.0, -1.0, 1.0);
    vec4 eyeRayDir = invProj * clipPos;
    eyeRayDir = vec4(eyeRayDir.xy, -1.0, 0.0);
    vec3 rd = normalize((invView * eyeRayDir).xyz);
    vec3 ro = uCameraWorldPos;

    // --- RAY MARCHING LOOP ---
    float dO = 0.0;
    vec2 matData = vec2(0.0);
    bool hit = false;
    
    for(int i = 0; i < MAX_STEPS; i++) {
        vec3 p = ro + rd * dO;
        matData = map(p);
        float dS = matData.x;
        dO += dS;
        if(dO >= MAX_DIST || abs(dS) < SURF_DIST) {
            if(dO < MAX_DIST) hit = true;
            break;
        }
    }

    vec3 sceneColor = vec3(0.01, 0.008, 0.003); // Deep, inkier baseline dark room glow

    if(hit) {
        vec3 p = ro + rd * dO;
        vec3 n = getNormal(p);
        float matID = matData.y;
        
        vec3 albedo = vec3(1.0);
        
        if (matID == 1.0) { // Carpet
            float nTile = noise(p.xz * 12.0) * 0.15;
            float nStain = noise(p.xz * 0.4) * 0.3;
            albedo = mix(C_CARPET, vec3(0.12, 0.09, 0.05), nTile + nStain);
        } 
        else if (matID == 2.0) { // Ceiling
            float grid = smoothstep(0.45, 0.48, max(fract(p.x * 0.33), fract(p.z * 0.33)));
            albedo = mix(C_CEILING, vec3(0.15), grid);
        } 
        else if (matID == 3.0) { // Wallpaper
            float stripPattern = smoothstep(0.1, 0.15, abs(sin(p.y * 40.0 + noise(p.xz * 5.0) * 0.5)));
            albedo = mix(C_WALL, C_WALL * 0.8, stripPattern);
            albedo -= vec3(noise(p.xy * 25.0 + vec2(p.z * 25.0)) * 0.04); 
        }
        else if (matID == 4.0) { // Dynamic Light Fixture Albedo
            vec2 lID = floor((p.xz + vec2(3.0)) / 6.0);
            float currentFixtureStatus = getLightStatus(lID);
            albedo = mix(C_LIGHT_OFF, C_LIGHT_ON, currentFixtureStatus);
        }

        if(matID != 4.0) {
            vec3 lightAcc = vec3(0.0);
            vec3 centerRoom = vec3(floor((p.x + 3.0) / 6.0) * 6.0, 1.4, floor((p.z + 3.0) / 6.0) * 6.0);
            
            for(int x = -1; x <= 1; x++) {
                for(int z = -1; z <= 1; z++) {
                    vec3 lPos = vec3(centerRoom.x + float(x)*6.0, 1.35, centerRoom.z + float(z)*6.0);
                    vec3 lDir = lPos - p;
                    float lDist = length(lDir);
                    lDir = normalize(lDir);
                    
                    float atten = 1.0 / (1.0 + lDist * 0.5 + lDist * lDist * 0.4);
                    float diff = max(dot(n, lDir), 0.0);
                    float shadowVal = getShadow(p + n * 0.01, lDir, 0.02, lDist, 12.0);
                    
                    // Evaluate dynamic light modulation state
                    vec2 specificLightID = floor((lPos.xz + vec2(3.0)) / 6.0);
                    float conditionModifier = getLightStatus(specificLightID);
                    
                    // Subtle background hum flicker combined with physical operational failure state
                    float microFlicker = mix(0.9, 1.0, hash(specificLightID + vec2(floor(uTime * 14.0))));
                    
                    lightAcc += C_LIGHT_ON * diff * atten * shadowVal * conditionModifier * microFlicker;
                }
            }
            
            float ao = getAO(p, n);
            sceneColor = albedo * (lightAcc + vec3(0.002)) * ao; 
        } else {
            sceneColor = albedo;
        }
        
        float fogFactor = smoothstep(0.0, MAX_DIST * 0.7, dO);
        sceneColor = mix(sceneColor, vec3(0.01, 0.008, 0.003), fogFactor);
    }
    
    // --- HEAVY ANALOG VIGNETTE ---
    vec2 uvVignette = vUV * (1.0 - vUV.yx);
    float vig = uvVignette.x * uvVignette.y * 15.0;
    vig = clamp(pow(vig, 0.45), 0.0, 1.0);
    sceneColor *= vig;
    
    // --- GRAINY NOISE OVERLAY ---
    float staticGrain = hash(vUV + vec2(sin(uTime), cos(uTime)));
    sceneColor += vec3(staticGrain * GRAIN_STRENGTH * (1.0 - length(sceneColor)));

    // Tone mapping and gamma correction
    sceneColor = sceneColor / (sceneColor + vec3(1.0));
    sceneColor = pow(sceneColor, vec3(1.0 / 2.2));

    fragColor = vec4(sceneColor, 1.0);
}