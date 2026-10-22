#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;

out vec4 vColor;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;

uniform float t;


float wave(float x, float count, float height, float speed) {
  return height * cos(count * x - speed * t);
}
float height() {
  return
    wave(aPos.x, 0.05, 2.2, 0.8) +
    wave(aPos.z, 0.04, 1.8, 0.6) +
    
    // 2. Medium choppy surface waves (Medium frequency, medium amplitude, medium speed)
    wave(aPos.x, 0.25, 0.6, 1.8) +
    wave(aPos.z, 0.20, 0.5, 1.5) +
    
    // 3. Small capillary surface ripples (High frequency, tiny amplitude, very fast)
    wave(aPos.x, 0.85, 0.15, 3.2) +
    wave(aPos.z, 0.70, 0.10, 2.8)
  ;
}

void main() {
  float h = height();

  vec4 lowColor = vec4(0,0,.1,1);
  vec4 highColor = vec4(0,0,1,1);

  float normalizedHeight = (h + 5.35) / 10.7;

  vColor = lowColor + (highColor - lowColor) * pow(normalizedHeight, 3);
  gl_Position = uProj * uView * uModel * vec4(aPos.x, h, aPos.z, 1);
}