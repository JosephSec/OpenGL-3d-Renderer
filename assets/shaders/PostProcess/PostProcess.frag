#version 430 core

in vec2 vUV;
uniform sampler2D uFrame;
uniform float uTime;

// --- STANDARD MATRIX UNIFORMS (NON-INVERSE) ---
uniform mat4 uView;              // Native non-inverse View Matrix
uniform mat4 uProj;              // Native non-inverse Projection Matrix
uniform vec3 uCameraWorldPos;    // Actual 3D position of your camera in the world

out vec4 fragColor;

// --- CONFIGURATION ---
const int   RAY_STEPS          = 64;    
const float SINGULARITY_RADIUS = 0.25; 
const float EVENT_HORIZON      = 2.5;   
const vec3  CORE_COLOR         = vec3(0.0, 0.4, 1.0);  
const vec3  DISK_COLOR         = vec3(1.2, 0.5, 0.1);  
const vec3  JET_COLOR          = vec3(0.5, 0.1, 0.9);  

// Pure math 3D Hash function
float hash3D(vec3 p) {
    p = fract(p * vec3(443.8975, 397.2973, 491.1871));
    p += dot(p.xyz, p.yzx + 19.19);
    return fract(p.x * p.y * p.z);
}

// 3D Value Noise for organic space dust tendrils
float noise3D(vec3 p) {
    vec3 i = floor(p); vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(mix(hash3D(i+vec3(0,0,0)), hash3D(i+vec3(1,0,0)), f.x),
                   mix(hash3D(i+vec3(0,1,0)), hash3D(i+vec3(1,1,0)), f.x), f.y),
               mix(mix(hash3D(i+vec3(0,0,1)), hash3D(i+vec3(1,0,1)), f.x),
                   mix(hash3D(i+vec3(0,1,1)), hash3D(i+vec3(1,1,1)), f.x), f.y), f.z);
}

// Analytical Ray-Sphere Intersection
bool IntersectSphere(vec3 ro, vec3 rd, float radius, out float t0, out float t1) {
    float b = dot(ro, rd);
    float c = dot(ro, ro) - (radius * radius);
    float h = b*b - c;
    if (h < 0.0) return false;
    h = sqrt(h);
    t0 = -b - h;
    t1 = -b + h;
    return true;
}

void main() {
    // 1. RECONSTRUCT 3D CAMERA RAY DIRECTION USING STANDARD MATRICES
    // Convert 2D screen UV coordinates to Normalized Device Coordinates [-1, 1]
    vec2 ndc = vUV * 2.0 - 1.0;
    
    // Extract focal terms straight out of your standard Projection Matrix diagonals
    float tanHalfFovX = 1.0 / uProj[0][0];
    float tanHalfFovY = 1.0 / uProj[1][1];
    
    // Generate view space target vector pointing forward down the negative Z-axis (-1.0)
    vec3 viewDir = vec3(ndc.x * tanHalfFovX, ndc.y * tanHalfFovY, -1.0);
    
    // Convert viewDir into a true World Space direction vector
    // Standard orthogonal matrices let us extract world vectors from the transposed rows of uView
    vec3 camRight = vec3(uView[0][0], uView[1][0], uView[2][0]);
    vec3 camUp    = vec3(uView[0][1], uView[1][1], uView[2][1]);
    vec3 camFwd   = vec3(uView[0][2], uView[1][2], uView[2][2]); // Represents view negative Z axis
    
    vec3 rayDir = normalize(viewDir.x * camRight + viewDir.y * camUp + viewDir.z * camFwd);
    vec3 rayOrigin = uCameraWorldPos;

    vec4 accumulatedColor = vec4(0.0);
    vec2 totalWarp = vec2(0.0);

    float tNear = 0.0, tFar = 0.0;
    if (IntersectSphere(rayOrigin, rayDir, EVENT_HORIZON, tNear, tFar)) {
        
        float tStart = max(0.0, tNear);
        float stepSize = (tFar - tStart) / float(RAY_STEPS);
        
        // --- 2. RAY DITHERING BRIDGING ---
        float dither = hash3D(rayOrigin + rayDir * tStart + uTime) * 0.5;
        vec3 p = rayOrigin + rayDir * (tStart + stepSize * dither);

        // 3. True 3D Multi-Volume Raymarching Loop
        for (int i = 0; i < RAY_STEPS; i++) {
            float d3D = length(p);

            // True singularity event horizon
            if (d3D < SINGULARITY_RADIUS) {
                accumulatedColor = vec4(0.0, 0.0, 0.0, 1.0);
                break;
            }

            float gravityPull = smoothstep(EVENT_HORIZON, SINGULARITY_RADIUS, d3D);

            // Compute gravitational rotation warp around the Y-axis
            float twistAngle = gravityPull * 5.0 - (uTime * 1.5);
            float cosA = cos(twistAngle); float sinA = sin(twistAngle);
            vec3 twistedP = p;
            twistedP.xz = mat2(cosA, -sinA, sinA, cosA) * twistedP.xz;

            float volumeDensity = 0.0;
            vec3 sampleColor = vec3(0.0);

            // EFFECT A: 3D Equatorial Accretion Disk Intersection Plane
            float diskRadius = length(p.xz);
            if (diskRadius > 0.4 && diskRadius < 1.8 && abs(p.y) < 0.12) {
                float thicknessProfile = smoothstep(0.12, 0.0, abs(p.y));
                float radialProfile = smoothstep(0.4, 0.7, diskRadius) * smoothstep(1.8, 1.1, diskRadius);
                
                float diskNoise = noise3D(twistedP * 6.0);
                float finalDiskDensity = smoothstep(0.4, 0.7, diskNoise) * thicknessProfile * radialProfile;
                
                volumeDensity += finalDiskDensity * 0.4;
                sampleColor = mix(sampleColor, DISK_COLOR, finalDiskDensity);
            }

            // EFFECT B: Dual-Axis Polar Gravitational Jet Streams
            float jetRadius = length(p.xz);
            if (abs(p.y) > 0.2 && jetRadius < 0.35 * (abs(p.y) + 0.5)) {
                float jetLengthProfile = smoothstep(2.5, 0.2, abs(p.y));
                float jetConeProfile = smoothstep(0.35 * (abs(p.y) + 0.5), 0.0, jetRadius);
                
                vec3 jetCoords = p * vec3(5.0, 2.0, 5.0);
                jetCoords.y -= uTime * 4.0;
                float jetNoise = noise3D(jetCoords);
                
                float finalJetDensity = smoothstep(0.4, 0.75, jetNoise) * jetLengthProfile * jetConeProfile;
                
                volumeDensity += finalJetDensity * 0.3;
                sampleColor = mix(sampleColor, JET_COLOR, finalJetDensity);
            }

            // EFFECT C: Standard Core Ambient Dark Energy Fog
            float coreNoise = noise3D(twistedP * 3.0 + vec3(0.0, uTime * 0.1, 0.0));
            float finalCoreDensity = smoothstep(0.5, 0.8, coreNoise) * gravityPull;
            volumeDensity += finalCoreDensity * 0.15;
            sampleColor = mix(sampleColor, CORE_COLOR, finalCoreDensity);

            // Front-to-back alpha composition step
            if (volumeDensity > 0.001) {
                vec4 energySample = vec4(sampleColor * volumeDensity, volumeDensity * 0.25);
                energySample.rgb *= energySample.a;
                accumulatedColor += energySample * (1.0 - accumulatedColor.a);
            }

            // Accumulate Einstein Ring bending warp vectors
            totalWarp += (normalize(p.xy) * (0.012 / (d3D - SINGULARITY_RADIUS + 0.01))) * gravityPull;

            p += rayDir * stepSize;
        }
    }

    // 4. Background Composite Output
    vec2 lensedUV = vUV + totalWarp;
    vec3 sceneColor = texture(uFrame, clamp(lensedUV, 0.001, 0.999)).rgb;
    vec3 finalColor = mix(sceneColor, accumulatedColor.rgb * 3.0, accumulatedColor.a);

    fragColor = vec4(finalColor, 1.0);
}