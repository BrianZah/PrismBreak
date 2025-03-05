#version 450 core

layout(binding = 0) uniform usampler2D iChannel0;
#include "../libs/text.glsl"
makeStr(printSeperator) _ 45 _end
makeStr1i(printDynNum) _dig(i) _end
makeStr1i(printChar) _ i _end

#define M_PI 3.1415926535897932384626433832795
#define not !

layout(location = 0) out vec4 FragColor;
layout(location = 1) out ivec2 facetAndTile;

in VsOut{
  vec3 wPosition;
} frag;

layout(r8ui, binding = 6) uniform uimage1D gAttributes;

//layout(r8i, binding = 0) uniform iimage1D gWindowIndexArray;
//layout(r8i, binding = 1) uniform iimage1D gBookmarks;
//layout(r32f, binding = 2) uniform image2D gComponents;
//layout(r32f, binding = 3) uniform image2D gMetrics;

//layout(std140, binding = 0) uniform Normals{
//  vec3 normals[numPhysFacets];
//};
//layout(std140, binding = 1) uniform Centers{
//  vec3 centers[numPhysTilesPerFacet];
//};
//layout(std140, binding = 2) uniform means{
//  float mean[numDistributions];
//};
uniform struct UserInput{
  float domainScale;
  float codomainScale;
} userInput = {10.0f, 1.0f};

const int compileErrorAvoider = numComponents == 0 ? 1 : 0;
layout(std140, binding = 3) uniform scaleInvs{
  vec4 scaleInvAndHeightAndDf[numDistributions*numComponents*numFacets+compileErrorAvoider];
};
uniform float[numDistributions] weights;
float scaleInv(int index) {return scaleInvAndHeightAndDf[index].x;}
float height(int index) {return weights[index%numDistributions]*scaleInvAndHeightAndDf[index].y;}
float df(int index) {return scaleInvAndHeightAndDf[index].z;}
float mean(int index) {return scaleInvAndHeightAndDf[index].w;}

uniform float camDegrees;
uniform ivec3[numComponents+compileErrorAvoider] selectedTiles;

struct Tile{
  int index;
  vec3 xAxis;
  vec3 center;
  vec2 size;
  vec2 range;
};
Tile getTile();
Tile tile = getTile();

const int numMetrics = 3;
const int maxNumMetrics = 10;
vec4 drawBars();
vec4 drawPolarAreaChart();
vec4 bookmarkIf(bool condition);
vec4 label();

uniform vec3 colors[numDistributions];

float pdf(int distribution, float x);

void main() {
  //setFacetAndTile();
  facetAndTile = ivec2(-1, tile.index);
  const int baseIndex = tile.index*numDistributions;
  FragColor = vec4(0.0f);
  vec3 fragColor = vec3(1.0f);

  if(tile.index > -1) {
    //FragColor += drawBars();
    //FragColor += bookmarkIf(imageLoad(gBookmarks, tile.index).x == 1);
    FragColor += label();
    if(FragColor.a > 0.99f) return;

    float x = dot(frag.wPosition.xz-tile.center.xz, tile.xAxis.xz)/tile.size.x*tile.range.x;
    float y = ((frag.wPosition.y-tile.center.y)/tile.size.y+0.5f)*tile.range.y;

    float densities[numDistributions+1];
    int indexDistributionMax = numDistributions;
    densities[numDistributions] = -1.0f;
    int indexDistributionMin = numDistributions;

    for(int dis = 0; dis < numDistributions; ++dis) {
      if(colors[dis].r*colors[dis].g*colors[dis].b > 0.99f) continue;
      int iDis = baseIndex + dis;
      densities[dis] = pdf(iDis, x);

      if((densities[dis] < densities[indexDistributionMin] || densities[indexDistributionMin] < y) && densities[dis] >= y) {
        indexDistributionMin = dis;
      }

      if(densities[dis] > densities[indexDistributionMax]) {
        indexDistributionMax = dis;
      }
    }
    float border = 0.02f;

    float eps=0.0001;
    float df= (densities[indexDistributionMin] - pdf(baseIndex+indexDistributionMin, x + eps)) / eps;
    float dist=abs(y-densities[indexDistributionMin])/sqrt(1+df*df);

    if (y <= densities[indexDistributionMin]){
      vec3 upperColor = indexDistributionMin == indexDistributionMax ? vec3(1.0f) : colors[indexDistributionMax%numDistributions];
      vec3 lowerColor = colors[indexDistributionMax%numDistributions];
      vec3 minDisColor = 0.8f*colors[indexDistributionMin%numDistributions];
      vec3 color1 = vec3(mix(upperColor, minDisColor, smoothstep(0.,0.25f*border,dist)));
      fragColor = vec3(mix(lowerColor, color1, smoothstep(border,0.75f*border,dist)));
    }
  }
  FragColor = vec4(FragColor.a*FragColor.xyz + (1.0f-FragColor.a)*fragColor, 1.0f);
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

//int getTileId(int virtualFaceId);
// void setFacetAndTile() {
//   facetAndTile.x = getFacetId();
//   facetAndTile.y = getTileId(facetAndTile.x);
//   if(-1 == facetAndTile.y) return;
//   int index = imageLoad(gWindowIndexArray, facetAndTile.x*numComponents + facetAndTile.y).x;
//   facetAndTile.x = index/numComponents;
//   facetAndTile.y = index%numComponents;
// }
// int getTileId(int virtualFaceId) {
//   if(virtualFaceId == 0 && showCanonicalBasis == 0) return -1;
//   if(virtualFaceId == numFacets && showCanonicalBasis == 1) return -1;
//   int windowId = physTile%numPhysTilesPerFacet;
//   return windowId < numComponents ? windowId : -1;
// }
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
  tile.xAxis = vec3(1.0f, 0.0f, 0.0f);
  vec2 offset = 0.5f*tileSize;
  tile.center = vec3(1.0f-offset.x, 1.0f-(2*physTile+1)*offset.y, 0.0f);
  tile.size = vec2(tileSize.x, tileSize.y);
  tile.range = vec2(userInput.domainScale, userInput.codomainScale*1.0f);
  return tile;
}

vec4 overlay(vec4 foreground, vec4 background) {
  return (1.0f-foreground.a)*background + foreground.a*vec4(foreground.xyz, 1.0f);
}
vec4 label() {
  float fontsize = 8.0f;
  int stage = 0;
  float x = fontsize*dot(frag.wPosition.xz - tile.center.xz, tile.xAxis.xz)/(0.5f*tile.size.x)-(fontsize);
  float y = fontsize*((frag.wPosition.y - tile.center.y)/(0.5f*tile.size.y))-(fontsize-1);
  if(y < -1/fontsize-stage || x < -4.5) return vec4(0.0);

  vec4 fragColor = vec4(1.0f, 1.0f, 1.0f, 0.75f);
  float alpha = 0.0f;
  int facet = selectedTiles[tile.index].y-2;
  vec3 color = facet < 0 ? vec3(0.0f) : 0.8*colors[facet%numDistributions];
  int selectedCanonBasis = int(0 == selectedTiles[tile.index].y);
  int selectedPcaBasis = int(1 == selectedTiles[tile.index].y);
  float scalar = 1.0f;

  alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+4.5/scalar, y), 120));
  alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+4.0/scalar, y), 45));
  if(1 == selectedCanonBasis) {
    int letter = int(imageLoad(gAttributes, (selectedTiles[tile.index].z)*tokenLength+0).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+3.5/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[tile.index].z)*tokenLength+1).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+3/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[tile.index].z)*tokenLength+2).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+2.5/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[tile.index].z)*tokenLength+3).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+2/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[tile.index].z)*tokenLength+4).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+1.5/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[tile.index].z)*tokenLength+5).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+1/scalar, y), letter));
  } else {
    if(1 == selectedPcaBasis) {
      alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+3.5/scalar, y), 80));
      alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+3.0/scalar, y), 67));
      alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+2.5/scalar, y), 65));
    } else {
      alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+3.5/scalar, y), tmm == selectedTiles[tile.index].x ? 84 : 71));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(scalar*vec2(x+3/scalar, y), (selectedTiles[tile.index].y-1)/10));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(scalar*vec2(x+2.5/scalar, y), (selectedTiles[tile.index].y-1)%10));
    }
    alpha += smoothstep(0.0f, 0.5f, printSeperator(scalar*vec2(x+2/scalar, y)));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(scalar*vec2(x+1.5/scalar, y), (selectedTiles[tile.index].z+1)/10));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(scalar*vec2(x+1/scalar, y), (selectedTiles[tile.index].z+1)%10));
  }
  fragColor = overlay(vec4(color, alpha), fragColor);
  alpha = 0.0f;
  return fragColor;
}
