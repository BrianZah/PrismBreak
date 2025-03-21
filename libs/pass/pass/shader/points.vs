#version 450 core
layout(location = 0) in vec3 wPosition;
layout(location = 1) in float assignedDistributionIn;
layout(location = 2) in float distanceHD;

layout(std430, binding = 1) buffer Probabilities{
  float[] probabilitiesIn;
};

out vec4 center;
out float radius;
out float assignedDistribution;
out float variance;
out float[numDistributions] probabilities;

uniform mat4 view;
uniform mat4 projection;

void main() {
  assignedDistribution = assignedDistributionIn;
  variance = distanceHD;

  for(int i = 0; i < numDistributions; ++i) {
    probabilities[i] = probabilitiesIn[gl_VertexID*numDistributions+i];
  }

  gl_Position = projection*view*vec4(wPosition, 1.0f);
  center = gl_Position / gl_Position.w;
  radius = 100.0f;
  vec4 border = projection*(view*vec4(wPosition, 1.0f)+vec4(radius, 0.0f, 0.0f, 0.0f));
  radius = distance(gl_Position/gl_Position.w, border/border.w);
  gl_PointSize = 2*radius;
}
