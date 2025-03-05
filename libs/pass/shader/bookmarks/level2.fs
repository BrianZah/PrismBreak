#version 450 core
#extension GL_NV_shader_atomic_float : require

layout(binding = 0) uniform sampler2D iChannel0;
#include "../libs/text.glsl"
makeStr(printSeperator) _ 45 _end
makeStr1i(printDynNum) _dig(i) _end
makeStr1i(printChar) _ i _end
#include "../libs/Ray.glsl"

#define M_PI 3.1415926535897932384626433832795
#define not !

layout(location = 0) out vec4 FragColor;
layout(location = 1) out ivec2 faceAndWindow;

in VsOut{
  vec3 wPosition;
} frag;

layout(r8i, binding = 0) uniform iimage1D gWindowIndexArray;
layout(r8i, binding = 1) uniform iimage1D gBookmarks;
layout(r32f, binding = 2) uniform image2D gComponents;
layout(r32f, binding = 3) uniform image2D gMetrics;
layout(r32f, binding = 4) uniform image3D gOverlap;

layout(r8ui, binding = 6) uniform uimage1D gAttributes;

//layout(std140, binding = 0) uniform Normals{
//  vec3 normals[numPhysFacets];
//};
//layout(std140, binding = 1) uniform Centers{
//  vec3 centers[numPhysTilesPerFacet];
//};
const int compileErrorAvoider = numComponents == 0 ? 1 : 0;
//layout(std140, binding = 2) uniform Means{
//  vec4 means[numDistributions*numComponents*numFacets+compileErrorAvoider];
//};
uniform float[numDistributions] weights;
layout(std140, binding = 3) uniform Parameters{
  mat4 parameters[numDistributions*numComponents*numFacets+compileErrorAvoider];
};

uniform bool updateMetics;
uniform ivec3[3*numComponents+compileErrorAvoider] selectedTiles;
/*uniform*/ const vec3 camCenter = vec3(0.0f);
uniform vec3 camPos;
uniform float tilePosZ = 15.5f;
uniform float camDegrees;
uniform mat4 view;
uniform mat3 model = mat3(1.0f);
const float sizeOriginalTile = 6.0f;

float tileDepthScalar = 1.5f;
// } user input
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

vec3 mean2(int index) {return vec3(parameters[index][3])/userInput.domainScale;}
mat3 scaleInv2(int index) {return mat3(parameters[index])*userInput.domainScale*userInput.domainScale;}

uniform vec3 colors[numDistributions];

const int MIP = 0;
const int HULL = 1;
const int DVR = 2;

const int gmm = 1;
const int tmm = 2;

#include "../libs/distGradient.glsl"
#include "../libs/hulls.glsl"
#include "../libs/dvr.glsl"
vec4 drawStairs(vec3 color, Ray ray, float targ, float density, int iDis);

const int numMetrics = 3;
const int maxNumMetrics = 10;
void updateMetricsIf(bool condition);

vec4 overlay(vec4 foreground, vec4 background) {
  return (1.0f-foreground.a)*background + foreground.a*vec4(foreground.xyz, 1.0f);
}
vec4 drawBox(Ray ray, float targ, vec3 tmpColor);
vec4 drawBars();
vec4 drawPolarAreaChart();
vec4 bookmarkIf(bool condition);
vec4 label();

Ray getRay();

void main() {
  const Ray ray = getRay();
  //updateMetricsIf(updateMetics);
  faceAndWindow = ivec2(-1, tile.index);
  const int baseIndex = tile.index*numDistributions;
  FragColor = vec4(1.0f);

  float tMax = 1000.0f;
  const float tBox = tMax;

  if(tile.index > -1) {
    if(rendering == MIP) {
      float densities[numDistributions+1];
      int indexDistributionMax = numDistributions;
      densities[numDistributions] = -1.0f;

      //if(densities[0] > threshold(baseIndex)) tMax = tTmp;

      for(int dis = 0; dis < numDistributions; ++dis) {
        if(colors[dis].r*colors[dis].g*colors[dis].b > 0.99f) continue;
        float tTmp = argmaxPdfWrtT(baseIndex+dis, ray);
        densities[dis] = tTmp >= 0.0f ? pdfExceedingThreshold(baseIndex+dis, ray.origin + tTmp*ray.direction) : 0.0f;

        if(densities[dis] > densities[indexDistributionMax]) {
          indexDistributionMax = dis;
          tMax = tTmp;
        }
      }
      if(densities[indexDistributionMax] > 0.0f && tMax < tBox) {
        FragColor = overlay(drawStairs(colors[indexDistributionMax%numDistributions], ray, tMax,
                              densities[indexDistributionMax], baseIndex+indexDistributionMax),
                              FragColor);
      }
    } else if(rendering == HULL) {
      vec3 lightPos = camPos;// + 50*mat3(view[0])*normalize(vec3(1, 1, 0));
      FragColor = overlay(rayCastProcedure(ray, 12, ivec2(gl_FragCoord.xy), tBox, lightPos, ivec2(baseIndex, baseIndex+numDistributions)), FragColor);
    } else if(rendering == DVR) {
      FragColor = overlay(DVRProcedure(ray, tBox, ivec2(baseIndex, baseIndex+numDistributions)), FragColor);
    }
    FragColor = overlay(label(), FragColor);
  }
}

Ray getRay() {
  Ray ray;
  ray.origin = vec3(sizeOriginalTile*(frag.wPosition.xy-tile.center.xy)/tile.size.xy, tilePosZ);
  ray.direction = normalize(ray.origin - vec3(0.0f, 0.0f, distance(camPos, camCenter)));
  return ray;
}

vec4 drawStairs(vec3 color, Ray ray, float targ, float density, int iDis) {
  /*uniform*/ float steps = 2.5f;
  /*uniform*/ float step_width = 25.0f;

    float growing = 0.75f;
    float dist_max = -2*log(density); // <----NEW
    dist_max = dist_max / (growing * growing);
    dist_max = dist_max - steps * floor(dist_max / steps);

    //float targ = argmaxPdf_wrt_t(indexDistribution, 0);    // <----NEW
    //vec3 dist_gr = -normalize(vec3(frag.mean[iDis]-(ray.origin+targ*ray.direction)));   // <----NEW
    vec3 dist_gr = distGradient(iDis, ray.origin + targ*ray.direction);
    ////vec3 dist_gr = normalize(-1.0f/df[indexDistribution]*(2.0f*scaleInv[indexDistribution]*ray.direction - 2.0f*scaleInv[indexDistribution]*(mean[indexDistribution]-ray.origin)));

    float t = -dist_max * dist_max * step_width + dot(dist_gr.xy, dist_gr.xy) / (growing * growing);
    t = 1.0f- smoothstep(-1.0f, 0.0f, t);

    float gd = dist_gr.x + dist_gr.y;
    float t_f = (t + 1.0f) / 2.0f;

    t = -dist_max * dist_max * 2. * step_width + dot(dist_gr.xy, dist_gr.xy) / (growing * growing);
    t = 1.0f- smoothstep(-1.0f, -0.0f, t);

    vec3 returnCol = t_f*color.rgb + (1.0f-t)*(color.rgb*0.75f + vec3(0.5f)*gd);

    return vec4(returnCol, 1.0f);
}

float argmaxPdfWrtTMetric(Ray mRay, int index) {
  return dot(mRay.direction, scaleInv2(index)*(mean2(index)-mRay.origin))
        /dot(mRay.direction, scaleInv2(index)*mRay.direction);
}

float pdfMetric(Ray mRay, int index, float t) {
  vec3 pMinusMean = mRay.origin + t*mRay.direction - mean2(index);
  return
    height(index)
    * pow( 1.0f + 1.0f/df(index)*dot(pMinusMean, scaleInv2(index)*pMinusMean),
           -0.5f*(df(index)+3) );
}

float pdfExceedingThresholdMetric(Ray mRay, int index, float t) {
  float density = t >= 0 ? pdfMetric(mRay, index, t) : 0.0f;
  return density >= threshold(index) ? density : 0.0f;
}

void updateMetricsIf(bool condition) {
  if(not condition) return;
  Ray mRay;
  vec3 mCamPos = vec3(0.0f, 0.0f, distance(camPos, camCenter));
  mRay.direction = (mat3(view))*frag.wPosition - mCamPos;
  mRay.origin = (mat3(view))*frag.wPosition;

  for(int face = 0; face < numFacets; ++face) {
    for(int comp = 0; comp < numComponents; ++comp) {
      int baseIndex = face*numComponents*numDistributions + comp*numDistributions;
      int indexDistributionMax = 0;
      float tMax = argmaxPdfWrtTMetric(mRay, baseIndex + indexDistributionMax);
      float densities[numDistributions];
      float sum[numDistributions][numDistributions];

      densities[0] = pdfExceedingThresholdMetric(mRay, baseIndex + indexDistributionMax, tMax);
      sum[0][0] = 0.0f;

      for(int dis = 0; dis < numDistributions; ++dis) {
        float tTmp = argmaxPdfWrtTMetric(mRay, baseIndex + dis);
        densities[dis] = pdfExceedingThresholdMetric(mRay, baseIndex + dis, tTmp);

        for(int j = 0; j < dis; ++j) {
          sum[dis][j] = densities[dis]*densities[j];
        }
        if(densities[dis] > densities[indexDistributionMax]) {
          indexDistributionMax = dis;
          tMax = tTmp;
        }
      }

      imageAtomicAdd(gOverlap, ivec3(indexDistributionMax*numDistributions + indexDistributionMax, comp, face), 1.0f*densities[indexDistributionMax]*densities[indexDistributionMax]);
      for(int dis = 0; dis < numDistributions; ++dis) {
        for(int j = 0; j < dis; ++j) {
          imageAtomicAdd(gOverlap, ivec3(dis*numDistributions + j, comp, face), 30.0f*sum[dis][j]);
          imageAtomicAdd(gOverlap, ivec3(j*numDistributions + dis, comp, face), 30.0f*sum[dis][j]);
        }
      }
    }
  }
}

int getFacetId(int physTile) {
  int faceId = physTile/numPhysTilesPerFacet;
  int turns = int(round(camDegrees/360.0f - float(faceId)/numPhysFacets));
  return int(mod(faceId+numPhysFacets*turns, numFacets+1));
}

int getTileIndex() {
  int physTile = gl_PrimitiveID/2;
  int facet = getFacetId(physTile);
  int tile = int(physTile)%numPhysTilesPerFacet;
  bool tileIsNotUsed = tile >= numComponents;
  //return tileIsNotUsed ? -1 : imageLoad(gWindowIndexArray, facet*numComponents+tile).x;
  return tileIsNotUsed ? -1 : tile;
}

Tile getTile() {
  Tile tile;
  int physTile = gl_PrimitiveID/2;
  tile.index = getTileIndex();
  vec3 zAxis = vec3(0.0f, 0.0f, 1.0f);
  vec3 yAxis = vec3(0.0f, 1.0f, 0.0f);
  vec3 xAxis = vec3(1.0f, 0.0f, 0.0f);
  tile.rotation = mat3(xAxis, yAxis, zAxis);
  tile.shift = vec3(0.0f, 0.0f, tilePosZ) - vec3(0.0f, 0.0f, userInput.distanceminimizer);
  vec2 offset = 0.5f*tileSize;
  tile.center = vec3(1.0f-offset.x, 1.0f-(2*physTile+1)*offset.y, -tilePosZ);
  tile.size = vec3(tileSize.x, tileSize.y, tileDepthScalar*tileSize);
  //vec3 tmp = (centers[physTile%numPhysTilesPerFacet]+vec3(0.5f*tile.size.xy, 0.001f));
  //tile.offset = tile.rotation*mod(tmp, tile.size);
  return tile;
}

vec4 label() {
  float fontsize = 8.0f;
  int stage = 2;
  float x = fontsize*dot(frag.wPosition.xz - tile.center.xz, tile.rotation[0].xz)/(0.5f*tile.size.x)-(fontsize);
  float y = fontsize*((frag.wPosition.y - tile.center.y)/(0.5f*tile.size.y))-(fontsize-1);
  if(y < -1/fontsize-stage || x < -4.5) return vec4(0.0);

  vec4 fragColor = vec4(1.0f, 1.0f, 1.0f, 0.75f);
  float alpha = 0.0f;
  int facet = selectedTiles[3*tile.index].y-2;
  vec3 color = facet < 0 ? vec3(0.0f) : 0.8*colors[facet%numDistributions];
  int selectedCanonBasis = int(0 == selectedTiles[3*tile.index].y);
  int selectedPcaBasis = int(1 == selectedTiles[3*tile.index].y);

  alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+4.5, y), 120));
  alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+4.0, y), 45));
  if(1 == selectedCanonBasis) {
    int letter = int(imageLoad(gAttributes, selectedTiles[3*tile.index].z*tokenLength+0).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+3.5, y), letter));
    letter = int(imageLoad(gAttributes, selectedTiles[3*tile.index].z*tokenLength+1).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+3.0, y), letter));
    letter = int(imageLoad(gAttributes, selectedTiles[3*tile.index].z*tokenLength+2).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+2.5, y), letter));
    letter = int(imageLoad(gAttributes, selectedTiles[3*tile.index].z*tokenLength+3).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+2.0, y), letter));
    letter = int(imageLoad(gAttributes, selectedTiles[3*tile.index].z*tokenLength+4).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+1.5, y), letter));
    letter = int(imageLoad(gAttributes, selectedTiles[3*tile.index].z*tokenLength+5).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+1.0, y), letter));
  } else {
    if(1 == selectedPcaBasis) {
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.5, y), 80));
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.0, y), 67));
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+2.5, y), 65));
    } else {
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.5, y), tmm == selectedTiles[3*tile.index].x ? 84 : 71));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+3, y), (selectedTiles[3*tile.index].y-1)/10));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+2.5, y), (selectedTiles[3*tile.index].y-1)%10));
    }
    alpha += smoothstep(0.0f, 0.5f, printSeperator(vec2(x+2, y)));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+1.5, y), (selectedTiles[3*tile.index].z+1)/10));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+1, y), (selectedTiles[3*tile.index].z+1)%10));
  }
  fragColor = overlay(vec4(color, alpha), fragColor);
  alpha = 0.0f;
  y++;
  int selectedCanonBasisOld = selectedCanonBasis;
  facet = selectedTiles[3*tile.index+1].y-1-selectedCanonBasisOld;
  color = facet < 0 ? vec3(0.0f) : 0.8*colors[facet%numDistributions];
  selectedCanonBasis*= int(0 == selectedTiles[3*tile.index+1].y);
  selectedPcaBasis = int(selectedCanonBasisOld == selectedTiles[3*tile.index+1].y);
  int skip = int(selectedTiles[2*tile.index+1].z >= selectedTiles[3*tile.index].z);
  alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+4.5, y), 121));
  alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+4.0, y), 45));
  if(1 == selectedCanonBasis) {
    int letter = int(imageLoad(gAttributes, (selectedTiles[3*tile.index+1].z+skip)*tokenLength+0).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+3.5, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[3*tile.index+1].z+skip)*tokenLength+1).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+3, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[3*tile.index+1].z+skip)*tokenLength+2).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+2.5, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[3*tile.index+1].z+skip)*tokenLength+3).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+2, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[3*tile.index+1].z+skip)*tokenLength+4).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+1.5, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[3*tile.index+1].z+skip)*tokenLength+5).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+1, y), letter));
  } else {
    if(1 == selectedPcaBasis) {
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.5, y), 80));
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.0, y), 67));
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+2.5, y), 65));
    } else {
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.5, y), tmm == selectedTiles[3*tile.index+1].x ? 84 : 71));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+3, y), (selectedTiles[3*tile.index+1].y-selectedCanonBasisOld)/10));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+2.5, y), (selectedTiles[3*tile.index+1].y-selectedCanonBasisOld)%10));
    }
    alpha += smoothstep(0.0f, 0.5f, printSeperator(vec2(x+2, y)));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+1.5, y), (selectedTiles[3*tile.index+1].z+1)/10));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+1, y), (selectedTiles[3*tile.index+1].z+1)%10));
  }
  fragColor = overlay(vec4(color, alpha), fragColor);
  alpha = 0.0f;
  y++;
  selectedCanonBasisOld = selectedCanonBasis;
  facet = selectedTiles[3*tile.index+2].y - 1 - selectedCanonBasisOld;
  color = facet < 0 ? vec3(0.0f) : 0.8*colors[facet%numDistributions];
  selectedCanonBasis*= int(0 == selectedTiles[3*tile.index+2].y);
  selectedPcaBasis = int(selectedCanonBasisOld == selectedTiles[3*tile.index+2].y);
  int prevskip = int(selectedTiles[3*tile.index+2].z >= selectedTiles[3*tile.index+1].z);
  skip = int(selectedTiles[3*tile.index+2].z+prevskip >= selectedTiles[3*tile.index].z) + prevskip;
  alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+4.5, y), 122));
  alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+4.0, y), 45));
  if(1 == selectedCanonBasis) {
    int letter = int(imageLoad(gAttributes, (selectedTiles[3*tile.index+2].z+skip)*tokenLength+0).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+3.5, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[3*tile.index+2].z+skip)*tokenLength+1).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+3, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[3*tile.index+2].z+skip)*tokenLength+2).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+2.5, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[3*tile.index+2].z+skip)*tokenLength+3).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+2, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[3*tile.index+2].z+skip)*tokenLength+4).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+1.5, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[3*tile.index+2].z+skip)*tokenLength+5).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+1, y), letter));
  } else {
    if(1 == selectedPcaBasis) {
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.5, y), 80));
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.0, y), 67));
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+2.5, y), 65));
    } else {
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.5, y), tmm == selectedTiles[3*tile.index+2].x ? 84 : 71));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+3, y), (selectedTiles[3*tile.index+2].y-selectedCanonBasisOld)/10));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+2.5, y), (selectedTiles[3*tile.index+2].y-selectedCanonBasisOld)%10));
    }
    alpha += smoothstep(0.0f, 0.5f, printSeperator(vec2(x+2, y)));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+1.5, y), (selectedTiles[3*tile.index+2].z+1)/10));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+1, y), (selectedTiles[3*tile.index+2].z+1)%10));
  }
  fragColor = overlay(vec4(color, alpha), fragColor);
  alpha = 0.0f;
  return fragColor;
  //int facet = selectedTiles[3*tile.index+1].x-2;
  //vec3 color = facet < 0 || alpha < 0.01 ? vec3(0.0f) : colors[facet];
  //return vec4(0.8*color, alpha);
}
