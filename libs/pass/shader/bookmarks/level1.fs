#version 450 core

layout(binding = 0) uniform usampler2D iChannel0;
#include "../libs/text.glsl"
makeStr(printSeperator) _ 45 _end
makeStr1i(printDynNum) _dig(i) _end
makeStr1i(printChar) _ i _end

#define M_PI 3.1415926535897932384626433832795
#define not !

layout(location = 0) out vec4 FragColor;
layout(location = 1) out ivec2 faceAndWindow;

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
const int compileErrorAvoider = numComponents == 0 ? 1 : 0;
//layout(std140, binding = 2) uniform Means{
//  vec4 means[numDistributions*numComponents*numFacets+compileErrorAvoider];
//};
uniform float[numDistributions] weights;
layout(std140, binding = 3) uniform scaleInvs{
  mat2x4 scaleInvAndHeightAndDf[numDistributions*numComponents*numFacets+compileErrorAvoider];
};
mat2 scaleInv(int index) {return mat2(scaleInvAndHeightAndDf[index]);}
float height(int index) {return weights[index%numDistributions]*scaleInvAndHeightAndDf[index][0].z;}
float df(int index) {return scaleInvAndHeightAndDf[index][0].w;}
vec2 mean(int index) {return vec2(scaleInvAndHeightAndDf[index][1].zw);}
float threshold(int iDis);

uniform float camDegrees;
uniform ivec3[2*numComponents+compileErrorAvoider] selectedTiles;

uniform struct UserInput{
  float domainScale;
  float threshold;
  bool adoptToStd;
} userInput = {10.0f, 0.005f, true};

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

float pdf(int distribution, float x, float y);
vec3 drawStairs(vec3 color, float density);

void main() {
  faceAndWindow = ivec2(-1, tile.index);
  const int baseIndex = tile.index*numDistributions;
  FragColor = vec4(0.0f);
  vec3 fragColor = vec3(1.0f);

  if(tile.index > -1) {
    //FragColor += drawBars();
    //FragColor += bookmarkIf(imageLoad(gBookmarks, tile.index).x == 1);
    FragColor += label();
    if(FragColor.a > 0.99f) return;

    float x = dot(frag.wPosition.xz - tile.center.xz, tile.xAxis.xz)/(tile.size.x)*tile.range.x;
    float y = ((frag.wPosition.y-tile.center.y)/tile.size.y)*tile.range.y;

    float densities[numDistributions+1];
    int indexDistributionMax = numDistributions;
    densities[numDistributions] = -1.0f;

    for(int dis = 0; dis < numDistributions; ++dis) {
      if(colors[dis].r*colors[dis].g*colors[dis].b > 0.99f) continue;
      int iDis = baseIndex + dis;
      densities[dis] = pdf(iDis, x, y);
      densities[dis] = densities[dis] >= threshold(iDis) ? densities[dis] : 0.0f;

      if(densities[dis] > densities[indexDistributionMax]) {
        indexDistributionMax = dis;
      }
    }

    if(densities[indexDistributionMax] > 0.0f)
      fragColor = drawStairs(colors[indexDistributionMax%numDistributions], densities[indexDistributionMax]);
  }
  FragColor = vec4(FragColor.a*FragColor.xyz + (1.0f-FragColor.a)*fragColor, 1.0f);
}

vec3 drawStairs(vec3 color, float density) {
  /*uniform*/ float steps = 2.5f;
  /*uniform*/ float step_width = 25.0f;

    float growing = 0.75f;
    float dist_max = -2*log(density); // <----NEW
    dist_max = dist_max / (growing * growing);
    dist_max = dist_max - steps * floor(dist_max / steps);


    //float targ = argmaxPdf_wrt_t(indexDistribution, 0);    // <----NEW
    //vec3 dist_gr = -normalize(window.origin-window.front);   // <----NEW
    vec3 dist_gr = -normalize(vec3(1.0, 1.0, 1.0));   // <----NEW
    ////vec3 dist_gr = normalize(-1.0f/df[indexDistribution]*(2.0f*scaleInv[indexDistribution]*ray.direction - 2.0f*scaleInv[indexDistribution]*(mean[indexDistribution]-ray.origin)));

    float t = -dist_max * dist_max * step_width + dot(dist_gr.xy, dist_gr.xy) / (growing * growing);
    t = 1.0f- smoothstep(-1.0f, 0.0f, t);

    float gd = dist_gr.x + dist_gr.y;
    float t_f = (t + 1.0f) / 2.0f;

    t = -dist_max * dist_max * 2. * step_width + dot(dist_gr.xy, dist_gr.xy) / (growing * growing);
    t = 1.0f- smoothstep(-1.0f, -0.0f, t);

    vec3 returnCol = t_f*color.rgb + (1.0f-t)*(color.rgb*0.75f + vec3(0.5f)*gd);

    return returnCol;
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
  tile.range = vec2(userInput.domainScale);
  return tile;
}

vec4 overlay(vec4 foreground, vec4 background) {
  return (1.0f-foreground.a)*background + foreground.a*vec4(foreground.xyz, 1.0f);
}
vec4 label() {
  float fontsize = 8.0f;
  int stage = 1;
  float x = fontsize*dot(frag.wPosition.xz - tile.center.xz, tile.xAxis.xz)/(0.5f*tile.size.x)-(fontsize);
  float y = fontsize*((frag.wPosition.y - tile.center.y)/(0.5f*tile.size.y))-(fontsize-1);
  if(y < -1/fontsize-stage || x < -4.5) return vec4(0.0);

  vec4 fragColor = vec4(1.0f, 1.0f, 1.0f, 0.75f);
  float alpha = 0.0f;
  int facet = selectedTiles[2*tile.index].y-2;
  vec3 color = facet < 0 ? vec3(0.0f) : 0.8*colors[facet%numDistributions];
  int selectedCanonBasis = int(0 == selectedTiles[2*tile.index].y);
  int selectedPcaBasis = int(1 == selectedTiles[2*tile.index].y);

  alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+4.5, y), 120));
  alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+4.0, y), 45));
  if(1 == selectedCanonBasis) {
    int letter = int(imageLoad(gAttributes, selectedTiles[2*tile.index].z*tokenLength+0).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+3.5, y), letter));
    letter = int(imageLoad(gAttributes, selectedTiles[2*tile.index].z*tokenLength+1).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+3.0, y), letter));
    letter = int(imageLoad(gAttributes, selectedTiles[2*tile.index].z*tokenLength+2).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+2.5, y), letter));
    letter = int(imageLoad(gAttributes, selectedTiles[2*tile.index].z*tokenLength+3).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+2.0, y), letter));
    letter = int(imageLoad(gAttributes, selectedTiles[2*tile.index].z*tokenLength+4).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+1.5, y), letter));
    letter = int(imageLoad(gAttributes, selectedTiles[2*tile.index].z*tokenLength+5).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+1.0, y), letter));
  } else {
    if(1 == selectedPcaBasis) {
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.5, y), 80));
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.0, y), 67));
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+2.5, y), 65));
    } else {
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.5, y), tmm == selectedTiles[2*tile.index].x ? 84 : 71));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+3, y), (selectedTiles[2*tile.index].y-1)/10));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+2.5, y), (selectedTiles[2*tile.index].y-1)%10));
    }
    alpha += smoothstep(0.0f, 0.5f, printSeperator(vec2(x+2, y)));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+1.5, y), (selectedTiles[2*tile.index].z+1)/10));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+1, y), (selectedTiles[2*tile.index].z+1)%10));
  }
  fragColor = overlay(vec4(color, alpha), fragColor);
  alpha = 0.0f;
  y++;
  int selectedCanonBasisOld = selectedCanonBasis;
  facet = selectedTiles[2*tile.index+1].y-1-selectedCanonBasisOld;
  color = facet < 0 ? vec3(0.0f) : 0.8*colors[facet%numDistributions];
  selectedCanonBasis*= int(0 == selectedTiles[2*tile.index+1].y);
  selectedPcaBasis = int(selectedCanonBasisOld == selectedTiles[2*tile.index+1].y);
  int skip = int(selectedTiles[2*tile.index+1].z >= selectedTiles[2*tile.index].z);
  alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+4.5, y), 121));
  alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+4.0, y), 45));
  if(1 == selectedCanonBasis) {
    int letter = int(imageLoad(gAttributes, (selectedTiles[2*tile.index+1].z+skip)*tokenLength+0).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+3.5, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[2*tile.index+1].z+skip)*tokenLength+1).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+3, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[2*tile.index+1].z+skip)*tokenLength+2).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+2.5, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[2*tile.index+1].z+skip)*tokenLength+3).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+2, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[2*tile.index+1].z+skip)*tokenLength+4).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+1.5, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[2*tile.index+1].z+skip)*tokenLength+5).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+1, y), letter));
  } else {
    if(1 == selectedPcaBasis) {
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.5, y), 80));
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.0, y), 67));
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+2.5, y), 65));
    } else {
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.5, y), tmm == selectedTiles[2*tile.index+1].x ? 84 : 71));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+3, y), (selectedTiles[2*tile.index+1].y-selectedCanonBasisOld)/10));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+2.5, y), (selectedTiles[2*tile.index+1].y-selectedCanonBasisOld)%10));
    }
    alpha += smoothstep(0.0f, 0.5f, printSeperator(vec2(x+2, y)));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+1.5, y), (selectedTiles[2*tile.index+1].z+1)/10));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+1, y), (selectedTiles[2*tile.index+1].z+1)%10));
  }
  fragColor = overlay(vec4(color, alpha), fragColor);
  alpha = 0.0f;
  return fragColor;
}
