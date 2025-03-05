#version 450 core
layout(location = 0) in vec3 wPosition;

out VsOut{
  vec3 wPosition;
} vsOut;

uniform mat4 projection;
uniform mat4 view;

void main() {
  vsOut.wPosition = vec3(wPosition);
  gl_Position = projection*view*vec4(wPosition, 1.0f);
}
