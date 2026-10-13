#version 430 core
in vec3 vDir;

uniform samplerCube uSky;
out vec4 fragColor;

void main() {
  fragColor = texture(uSky, vDir);
}