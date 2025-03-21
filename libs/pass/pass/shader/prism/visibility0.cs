#version 450
#extension GL_NV_shader_atomic_float : require
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

#define M_PI 3.1415926535897932384626433832795
#define not !

const ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
const int tileIndex = int(gl_GlobalInvocationID.z);
const ivec2 imgSize = ivec2(512, 512);

layout(r32f, binding = 4) uniform image2D gOverlap;

uniform struct UserInput{
  float domainScale;
  float codomainScale;
} userInput = {10.0f, 1.0f};

uniform float[numDistributions] weights;
layout(std140, binding = 3) buffer Parameters{
  vec4 parameters[numDistributions*numComponents*numFacets];
};
float scaleInv(int index) {return parameters[index].x;}
float height(int index) {return weights[index%numDistributions]*parameters[index].y;}
float df(int index) {return parameters[index].z;}
float mean(int index) {return parameters[index].w;}

float pdf(int distribution, float x);

void updateMetricsIf(bool condition);

void main() {
  updateMetricsIf(true);
}

float gaussianPdf(int distribution, float x) {
  float xMinusMean = x - mean(distribution);
  return height(distribution)*exp(-0.5*xMinusMean*scaleInv(distribution)*xMinusMean);
}

float tPdf(int distribution, float x) {
  float xMinusMean = x - mean(distribution);
  return
    height(distribution)
    * pow( 1.0f + 1.0f/df(distribution)*xMinusMean*scaleInv(distribution)*xMinusMean,
           -0.5f*(df(distribution)+1) );
}

const int gmm = 1;
const int tmm = 2;
float pdf(int distribution, float x) {
  if(mixtureModel == gmm) return gaussianPdf(distribution, x);
  else return tPdf(distribution, x);
}

float[numDistributions][numDistributions] initSum(float value) {
  float[numDistributions][numDistributions] sum;
  for(int dis = 0; dis < numDistributions; ++dis)
    for(int j = 0; j < dis; ++j)
      sum[dis][j] = value;
  return sum;
}

void updateMetricsIf(bool condition) {
  if(not condition) return;
  int baseIndex = tileIndex*numDistributions;
  int indexDistributionMax = 0;

  float x = (float(pixel.x)/float(imgSize.x) - 0.5f)*userInput.domainScale;

  float[numDistributions] densities;
  float[numDistributions][numDistributions] sum = initSum(0.0f);

  for(int dis = 0; dis < numDistributions; ++dis) {
    int iDis = baseIndex + dis;
    densities[dis] = pdf(iDis, x);
    if(densities[dis] > densities[indexDistributionMax]) indexDistributionMax = dis;
    for(int j = 0; j < dis; ++j) sum[dis][j]+= densities[dis]*densities[j];
  }

  imageAtomicAdd(gOverlap, ivec2(indexDistributionMax*numDistributions + indexDistributionMax, tileIndex), densities[indexDistributionMax]*densities[indexDistributionMax]);
  for(int dis = 0; dis < numDistributions; ++dis) {
    for(int j = 0; j < dis; ++j) {
      imageAtomicAdd(gOverlap, ivec2(dis*numDistributions + j, tileIndex), 1.0f*sum[dis][j]);
      imageAtomicAdd(gOverlap, ivec2(j*numDistributions + dis, tileIndex), 1.0f*sum[dis][j]);
    }
  }
}
