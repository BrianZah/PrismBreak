#version 450
#extension GL_NV_shader_atomic_float : require
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

#define M_PI 3.1415926535897932384626433832795
#define not !

const ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
const int tileIndex = int(gl_GlobalInvocationID.z);
const ivec2 imgSize = ivec2(512, 512);

layout(r32f, binding = 4) uniform image2D gOverlap;

uniform float[numDistributions] weights;
layout(std140, binding = 3) buffer Parameters{
  mat2x4 parameters[numDistributions*numComponents*numFacets];
};

uniform bool updateMetics;
uniform ivec3 selectedTiles;
uniform float camDegrees;

uniform struct UserInput{
  float domainScale;
  float threshold;
  bool adoptToStd;
} userInput = {10.0f, 0.005f, true};

mat2 scaleInv(int index) {return mat2(parameters[index]);}
float height(int index) {return weights[index%numDistributions]*parameters[index][0].z;}
float df(int index) {return parameters[index][0].w;}
vec2 mean(int index) {return vec2(parameters[index][1].zw);}
float threshold(int iDis);

float pdf(int distribution, float x, float y);

void updateMetricsIf(bool condition);

void main() {
  updateMetricsIf(true);
  //imageAtomicAdd(gOverlap, ivec2(0, tile.index), 1.0f/512.0f);
}


float gaussianPdf(int distribution, float x, float y) {
  vec2 pMinusMean = vec2(x, y) - mean(distribution);
  return height(distribution)*exp(-0.5*dot(pMinusMean, scaleInv(distribution)*pMinusMean));
}

float tPdf(int distribution, float x, float y) {
  vec2 pMinusMean = vec2(x, y) - mean(distribution);
  return
    height(distribution)
    * pow( 1.0f + 1.0f/df(distribution)*dot(pMinusMean, scaleInv(distribution)*pMinusMean),
           -0.5f*(df(distribution)+2) );
}

const int gmm = 1;
const int tmm = 2;
float pdf(int distribution, float x, float y) {
  if(mixtureModel == gmm) return gaussianPdf(distribution, x, y);
  else return tPdf(distribution, x, y);
}

float threshold(int iDis) {
  if(not userInput.adoptToStd) return userInput.threshold;
  float squaredMahalanobis = userInput.threshold*userInput.threshold;
  if(mixtureModel == gmm) {
    return height(iDis) * exp(-0.5f*squaredMahalanobis);
  } else {
    return height(iDis) * pow(1.0f+squaredMahalanobis/df(iDis), -0.5f*(df(iDis)+2) );
  }
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
  //for(int face = 0; face < numFacets; ++face) {
  //  for(int comp = 0; comp < numComponents; ++comp) {
      int baseIndex = tileIndex*numDistributions;
      int indexDistributionMax = 0;

      float x = (float(pixel.x)/float(imgSize.x) - 0.5f)*userInput.domainScale;
      float y = (float(pixel.y)/float(imgSize.y) - 0.5f)*userInput.domainScale;

      float[numDistributions] densities;
      float[numDistributions][numDistributions] sum = initSum(0.0f);

      for(int dis = 0; dis < numDistributions; ++dis) {
        int iDis = baseIndex + dis;
        densities[dis] = pdf(iDis, x, y);
        densities[dis] = densities[dis] >= threshold(iDis) ? densities[dis] : 0.0f;

        if(densities[dis] > densities[indexDistributionMax]) {
          indexDistributionMax = dis;
        }

        for(int j = 0; j < dis; ++j) sum[dis][j]+= densities[dis]*densities[j];
      }

      //imageAtomicAdd(gOverlap, ivec2(indexDistributionMax*numDistributions + indexDistributionMax, tileIndex), 1.0f);
      imageAtomicAdd(gOverlap, ivec2(indexDistributionMax*numDistributions + indexDistributionMax, tileIndex), densities[indexDistributionMax]*densities[indexDistributionMax]);
      for(int dis = 0; dis < numDistributions; ++dis) {
        for(int j = 0; j < dis; ++j) {
          imageAtomicAdd(gOverlap, ivec2(dis*numDistributions + j, tileIndex), 1.0f*sum[dis][j]);
          imageAtomicAdd(gOverlap, ivec2(j*numDistributions + dis, tileIndex), 1.0f*sum[dis][j]);
        }
      }
  //  }
  //}
}
