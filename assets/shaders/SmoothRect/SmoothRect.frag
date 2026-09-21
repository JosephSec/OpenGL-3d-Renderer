#version 330 core

uniform vec2 windowSize;
uniform vec4 rect;
uniform float radius;
uniform vec4 color;

out vec4 fragColor;

float sdRoundedBox(vec2 p, vec2 b, float r) {
  vec2 q = abs(p) - b + vec2(r);
  return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

void main() {
  vec2 pixelPos = vec2(gl_FragCoord.x, windowSize.y - gl_FragCoord.y);
  vec2 rectCenter = rect.xy + rect.zw * 0.5;
  vec2 halfSize = rect.zw * 0.5;
  vec2 p = pixelPos - rectCenter;
  
  float dist = sdRoundedBox(p, halfSize, radius);
  float edgeSmooth = fwidth(dist);
  float alpha = 1.0 - smoothstep(-edgeSmooth, 0.0, dist);
  
  fragColor = vec4(color.rgb, color.a * alpha);
}
