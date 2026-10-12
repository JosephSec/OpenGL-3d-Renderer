#version 430 core

in vec2 vUV;
uniform sampler2D uFrame; // Your base scene
uniform float uTime;

out vec4 fragColor;

// --- HIGH-DETAIL CONFIGURATION ---
const float CHARACTER_GRID_X = 256.0; // Pushed way up for sharp, high-density text detail
const float AS_RATIO         = 1.777; // 16:9 standard format aspect correction ratio
const vec3 MATRIX_NEON_GREEN = vec3(0.0, 1.0, 0.3);
const vec3 MATRIX_DEEP_GLOW  = vec3(0.0, 0.25, 0.05);

// High-fidelity procedural font generator 
// Decodes a precise 4x7 glyph matrix completely using GPU integer bitmask logic
float getDetailedGlyph(int glyphIndex, vec2 localUV) {
    if(localUV.x < 0.0 || localUV.x > 1.0 || localUV.y < 0.0 || localUV.y > 1.0) return 0.0;
    
    int x = int(localUV.x * 4.0);
    int y = int(localUV.y * 7.0);
    int bitIndex = x + (y * 4);
    
    uint mask = 0u;
    
    // Detailed 4x7 dot-matrix font selections packed into single integers
    switch(glyphIndex) {
        case 0: mask = 0u;          break; // Blank Space
        case 1: mask = 269488144u;  break; // Vertical tracking dot trail [ . ]
        case 2: mask = 277909616u;  break; // Alternate matrix terminal symbol [ x ]
        case 3: mask = 1431655765u; break; // Japanese-style digital glyph variant [ ｦ ]
        case 4: mask = 2005436535u; break; // Heavy tracking terminal node [ 0 ]
        case 5: mask = 4294967295u; break; // Pure solid core block [ # ]
    }
    
    return float((mask >> bitIndex) & 1u);
}

// Pseudo-random line speed tracker
float hash11(float p) {
    return fract(sin(p * 127.1) * 43758.5453123);
}

void main() {
    // 1. Setup high-density square glyph boundary boxes
    vec2 gridRes = vec2(CHARACTER_GRID_X, CHARACTER_GRID_X / AS_RATIO);
    vec2 blockUV = floor(vUV * gridRes) / gridRes;
    vec2 localUV = fract(vUV * gridRes);

    // 2. Extract linear light data and luminance profiles
    vec3 sceneColor = texture(uFrame, blockUV).rgb;
    float luma = dot(sceneColor, vec3(0.299, 0.587, 0.114));
    
    // Compress and shape the brightness profile to keep dark areas crisp
    luma = clamp(pow(luma, 1.2) * 1.3, 0.0, 1.0);

    // 3. Procedural "Code Rain" Vertical Scroller calculation
    float columnIndex = floor(vUV.x * CHARACTER_GRID_X);
    float fallSpeed   = 0.5 + hash11(columnIndex) * 1.5; // Dynamic per-column drop speed
    float verticalPos = uTime * fallSpeed + hash11(columnIndex * 33.1) * 10.0;
    
    // Calculate the looping vertical code stream trails
    float rainTrail = fract(vUV.y - verticalPos);
    rainTrail = pow(rainTrail, 8.0); // Sharpen into a trailing digital droplet whip

    // 4. Select character complexity based on scene brightness combined with rain state
    // Brighter lights/objects morph characters into denser, more complex digital glyphs
    float combinedEnergy = mix(luma, rainTrail, 0.4);
    int characterIndex   = int(combinedEnergy * 5.99);

    // Render the micro-font profile
    float fontPixel = getDetailedGlyph(characterIndex, localUV);

    // 5. High-Contrast Composite Color Ramp
    // Core luminance creates a bright white focal center, fading out into rich neon green glows
    vec3 baseCoreColor = mix(MATRIX_DEEP_GLOW, MATRIX_NEON_GREEN, luma);
    
    // If a rolling digital rain column hits a bright scene accent, overload it to pure blinding white
    vec3 hyperHighlight = mix(baseCoreColor, vec3(1.2, 1.5, 1.3), rainTrail * luma);
    
    // Cut out character topology out of the color profile maps
    vec3 finalColor = hyperHighlight * fontPixel;

    // Add a dark digital backdrop silhouette bleed so characters float above empty geometry fields
    finalColor += MATRIX_DEEP_GLOW * (1.0 - fontPixel) * 0.15 * luma;

    fragColor = vec4(finalColor, 1.0);
}