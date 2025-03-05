#version 450 core

#define PI 3.1415926535897932384626433832795

in vec4 center;
in float radius;
in float assignedDistribution;
in float variance;
in float[numDistributions] probabilities;

layout(location = 0) out vec4 FragColor;

uniform mat4 projection;
uniform vec2 range;

uniform ivec2 windowSize;
//const vec2 coordsUnitSquare = (2.0f*gl_FragCoord.xy-windowSize - center.xy*windowSize) / (2.0*radius);
const vec2 coordsUnitSquare = (gl_FragCoord.xy - 0.5f*(center.xy+1.0f)*windowSize) / radius;

vec2 polarCoords(vec2 cartesianCoords) {
  return vec2(length(cartesianCoords), atan(cartesianCoords.y, cartesianCoords.x));
}

uniform uint encoding;

void main() {
  vec2 p = polarCoords(-coordsUnitSquare.yx);
  FragColor = vec4(p.x, assignedDistribution, gl_FragDepth, gl_PrimitiveID);
  if(p.x > 1.0f) discard;
  if(1 == encoding) {
    float val = 0.0f;
    for(int i = 0; i < numDistributions; ++i)
      val = max(val, probabilities[i]);
    if(100.0f*val < range.x || 100.0f*val > range.y) discard;

    float sum = 0.0f;
    for(int i = 0; i < numDistributions; ++i) {
      sum+= probabilities[i];
      if((2.0f*sum-1.0f)*PI > p.y) {
        FragColor = vec4(p.x, i, gl_FragDepth, gl_PrimitiveID);
        break;
      }
    }
  } else {
    if(variance < range.x || variance > range.y) discard;
    float varMin = 2.0f;     // variance<=varMin? Then draw normal points
    float varMax = 8.0f;       // variance>=varMin? Then draw contours
    float maxContourLength = 0.50f;
    //float varNorm = smoothstep(varMin, varMax, variance); //varNorm in [0,1]
    float varNorm = min(variance < varMin ? 0.0f : variance/varMax, 1.0f);

    if(p.x > 1.0f-varNorm*maxContourLength) FragColor = vec4(3.0f, assignedDistribution, gl_FragDepth, gl_PrimitiveID);
  }
  return;
}
