#version 450
#extension GL_NV_shader_atomic_float : require
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

#include "../libs/Ray.glsl"

#define M_PI 3.1415926535897932384626433832795
#define not !

const ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
const int tileIndex = int(gl_GlobalInvocationID.z);
const ivec2 imgSize = ivec2(512, 512);

layout(r32f, binding = 4) uniform image2D gOverlap;

uniform float[numDistributions] weights;
layout(std140, binding = 3) buffer Parameters{
  mat4 parameters[numDistributions*numComponents*numFacets];
};

/*uniform*/ const vec3 camCenter = vec3(0.0f);
uniform vec3 camPos;
uniform float tilePosZ = 15.5f;
uniform float camDegrees;
uniform mat4 view;
uniform mat3 model;
const float sizeOriginalTile = 6.0f;

float tileDepthScalar = 1.5f;

uniform struct UserInput{
  float domainScale;
  float threshold;
  float distanceminimizer;
  bool adoptToStd;
} userInput = {4.0f/3.0f, 0.005f, 3.5f, true};

struct Tile{
  int index;
  mat3 rotation;
  vec3 shift;
  vec3 center;
  vec3 size;
  vec3 offset;
};
Tile getTile();
Tile tile = getTile();

vec3 mean(int index) {return tile.rotation*(model*vec3(parameters[index][3])/userInput.domainScale+tile.shift);}
mat3 scaleInv(int index) {return tile.rotation*((model*mat3(parameters[index])*transpose(model))*userInput.domainScale*userInput.domainScale)*transpose(tile.rotation);}
float height(int index) {return weights[index%numDistributions]*parameters[index][0].w;}
float df(int index) {return parameters[index][1].w;}

vec3 mean2(int index) {return model*vec3(parameters[index][3])/userInput.domainScale+vec3(0.0f, 0.0f, tile.shift.z);}
mat3 scaleInv2(int index) {return (model*mat3(parameters[index])*transpose(model))*userInput.domainScale*userInput.domainScale;}

const int gmm = 1;
const int tmm = 2;

float threshold(int iDis) {
  if(not userInput.adoptToStd)
    return userInput.threshold;
  float squaredMahalanobis = userInput.threshold*userInput.threshold;
  if(mixtureModel == gmm) {
    return height(iDis) * exp(-0.5f*squaredMahalanobis);
  } else {
    return height(iDis) * pow(1.0f+squaredMahalanobis/df(iDis), -0.5f*(df(iDis)+3) );
  }
}

void updateMetricsIf(bool condition);

void main() {
  updateMetricsIf(true);
  //imageAtomicAdd(gOverlap, ivec2(0, tile.index), 1.0f/512.0f);
}

Tile getTile() {
  Tile tile;
  tile.index = tileIndex;
  vec3 zAxis = vec3(0.0f, 0.0f, 1.0f);
  vec3 yAxis = vec3(0.0f, 1.0f, 0.0f);
  vec3 xAxis = vec3(1.0f, 0.0f, 0.0f);
  tile.rotation = mat3(xAxis, yAxis, zAxis);
  tile.shift = vec3(0.0f, 0.0f, tilePosZ) - vec3(0.0f, 0.0f, userInput.distanceminimizer);
  float offset = 0.5f*tileSize;
  tile.center = vec3(1.0f-offset, 1.0f-offset, -tilePosZ);
  tile.size = vec3(tileSize, tileSize, tileDepthScalar*tileSize);
  return tile;
}

float argmaxPdfWrtTMetric(int iDis, Ray mRay) {
  return dot(mRay.direction, scaleInv2(iDis)*(mean2(iDis)-mRay.origin))
        /dot(mRay.direction, scaleInv2(iDis)*mRay.direction);
}

float pdfMetric(int iDis, vec3 pos) {
  float innerProduct = dot((pos-mean2(iDis)), scaleInv2(iDis)*(pos-mean2(iDis)));
  if(mixtureModel == gmm) {
    return height(iDis) * exp(-0.5f*innerProduct);
  } else {
    return height(iDis) * pow(1.0f+innerProduct/df(iDis), -0.5f*(df(iDis)+3));
  }
}

float pdfExceedingThresholdMetric(int iDis, vec3 pos) {
  float density = pdfMetric(iDis, pos);
  return density >= threshold(iDis) ? density : 0.0f;
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
  Ray mRay;
  vec3 mCamPos = vec3(0.0f, 0.0f, distance(camPos, camCenter));
  vec3 fragPos = vec3(tileSize*(1.0f*pixel/imgSize-0.5f), tilePosZ);
  mRay.direction = normalize(fragPos-mCamPos);
  mRay.origin = fragPos;

  //for(int face = 0; face < numFacets; ++face) {
  //  for(int comp = 0; comp < numComponents; ++comp) {
      int baseIndex = tile.index*numDistributions;
      int indexDistributionMax = 0;
      float tMax = 0.0f;
      float[numDistributions] densities;
      float[numDistributions][numDistributions] sum = initSum(0.0f);

      for(int dis = 0; dis < numDistributions; ++dis) {
        float tTmp = argmaxPdfWrtTMetric(baseIndex + dis, mRay);
        densities[dis] = tTmp >= 0.0f ? pdfExceedingThresholdMetric(baseIndex + dis, mRay.origin + tTmp*mRay.direction) : 0.0f;
        //densities[dis] = pdfExceedingThresholdMetric(baseIndex + dis, mRay.origin + tTmp*mRay.direction);

        for(int j = 0; j < dis; ++j) sum[dis][j]+= densities[dis]*densities[j];

        if(densities[dis] > densities[indexDistributionMax]) {
          indexDistributionMax = dis;
          tMax = tTmp;
        }
      }

      imageAtomicAdd(gOverlap, ivec2(indexDistributionMax*numDistributions + indexDistributionMax, tile.index), densities[indexDistributionMax]*densities[indexDistributionMax]);
      for(int dis = 0; dis < numDistributions; ++dis) {
        for(int j = 0; j < dis; ++j) {
          imageAtomicAdd(gOverlap, ivec2(dis*numDistributions + j, tile.index), 1.0f*sum[dis][j]);
          imageAtomicAdd(gOverlap, ivec2(j*numDistributions + dis, tile.index), 1.0f*sum[dis][j]);
        }
      }
  //  }
  //}
}
