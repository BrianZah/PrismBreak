#version 450
#extension GL_NV_shader_atomic_float : require
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

#define M_PI 3.14159265358979

layout(rgba16f, binding = 0) uniform image2D gPoints;
layout(rgba16f, binding = 1) uniform image2D gAlbedo;
layout(r32f, binding = 2) uniform image1D gSum;

const int screenCam = 0;
uniform mat4 projection;
uniform mat4 view[maxNumCams];
uniform vec3 camPos[maxNumCams];
const int numCams = maxNumCams;

uniform int dimensions;
uniform float df[numDistributions];
uniform float height[numDistributions];
uniform vec3 mean[numDistributions];
uniform mat3 scaleInv[numDistributions];

const int numColors = 9;
const vec3 colors[numColors] = {
  vec3(71, 168, 186) / 255.0f,
  vec3(255, 255, 179) / 255.0f,
  vec3(182, 140, 185) / 255.0f,
  vec3(190, 186, 218) / 255.0f,
  vec3(251, 128, 114) / 255.0f,
  vec3(77, 73, 146) / 255.0f,
  vec3(179, 222, 105) / 255.0f,
  vec3(253, 180, 98) / 255.0f,
  vec3(217, 217, 217) / 255.0f
};

// user input {
const float threshold = 0.010f;//*prefactor[0];
// } user input

float[maxNumCams] argmaxPdf_wrt_t(int distribution);
float[maxNumCams] pdfExceedingThreshold(int distribution, float[maxNumCams] t);

vec3 getWorldPosfromScreenPos(vec2 screenPos, int cam);
vec3[maxNumCams] rayDirections();

const ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
const ivec2 imgSize = imageSize(gAlbedo);

const vec3 rayDirection[maxNumCams] = rayDirections();

void main() {
  vec4 albedo = vec4(0.0, 0.0, 0.0, 0.0);
  vec4 viewPos = vec4(0.0, 0.0, -10.0, 0.0);
  vec4 normal = vec4(0.0);

  int indexDistributionMax[maxNumCams];
  float tMax[maxNumCams] = argmaxPdf_wrt_t(0);
  float sum[maxNumCams];
  float densities[numDistributions][maxNumCams];
  densities[0] = pdfExceedingThreshold(0, tMax);

  for(int i = 0; i < numCams; ++i) {
    indexDistributionMax[i] = 0;
    sum[i] = (densities[0][i] >= threshold)
              ? 0.5f*numDistributions*(numDistributions-1)*densities[0][i]*densities[0][i]
              : 0.0f;
  }
  for(int distribution = 1; distribution < numDistributions; ++distribution) {
    float tTmp[maxNumCams] = argmaxPdf_wrt_t(distribution);
    densities[distribution] = pdfExceedingThreshold(distribution, tTmp);

    for(int i = 0; i < numCams; ++i) {
      for(int j = 0; j < distribution; ++j)
        if(min(densities[distribution][i], densities[j][i]) > threshold)
          sum[i]-= numDistributions*densities[distribution][i]*densities[j][i];
      if(densities[distribution][i] > threshold)
        sum[i]+= 0.5f*numDistributions*(numDistributions-1)*densities[distribution][i]*densities[distribution][i];
      if(densities[distribution][i] > densities[indexDistributionMax[i]][i]) {
        if(i == screenCam) indexDistributionMax[i] = distribution;
        tMax[i] = tTmp[i];
      }
    }
  }

  for(int i = 0; i < numCams; ++i) imageAtomicAdd(gSum, i, sum[i]);

  vec4 points = imageLoad(gPoints, pixel);
  if(points.r > 0.0f) {
    albedo = vec4((1.0f-0.33f*points.r)*colors[int(points.g+0.5f)%numColors], 1.0f)*vec4(1.0f);
  } else if(densities[indexDistributionMax[screenCam]][screenCam] >= threshold) {
    albedo = vec4(colors[indexDistributionMax[screenCam]%numColors], 1.0f);
  }

  imageStore(gAlbedo, pixel, albedo);
}

vec3 getWorldPosfromScreenPos(vec2 screenPos, int cam) {
  vec4 worldPos = inverse(projection*view[cam])*vec4(2.0f*screenPos-1.0f, 0.0f, 1.0f);
  return worldPos.xyz/worldPos.w;
}

vec3[maxNumCams] rayDirections() {
  vec3 rayDirection[maxNumCams];
  for(int i = 0; i < numCams; ++i)
    rayDirection[i] = getWorldPosfromScreenPos((1.0f*pixel)/imgSize, i)-camPos[i];
  return rayDirection;
}

float argmaxPdf_wrt_t(int distribution, int cam) {
  return dot(rayDirection[cam], scaleInv[distribution]*(mean[distribution]-camPos[cam]))
        /dot(rayDirection[cam], scaleInv[distribution]*rayDirection[cam]);
}
float[maxNumCams] argmaxPdf_wrt_t(int distribution) {
  float argmax[maxNumCams];
  for(int i = 0; i < numCams; ++i) argmax[i] = argmaxPdf_wrt_t(distribution, i);
  return argmax;
}

float pdf(int distribution, int cam, float t) {
  int index = distribution*maxNumCams + cam;
  vec3 pMinusMean = camPos[cam] + t*rayDirection[cam] - mean[distribution];
  return
    height[distribution]
    * pow( 1.0f + 1.0f/df[distribution]*dot(pMinusMean, scaleInv[distribution]*pMinusMean),
           -0.5f*(df[distribution]+3) );
}
float[maxNumCams] pdfExceedingThreshold(int distribution, float[maxNumCams] t) {
  float density[maxNumCams];
  for(int i = 0; i < numCams; ++i) {
    density[i] = t[i] >= 0 ? pdf(distribution, i, t[i]) : 0.0f;
    density[i] = density[i] >= threshold ? density[i] : 0.0f;
  }
  return density;
}
