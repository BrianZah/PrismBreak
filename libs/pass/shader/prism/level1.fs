#version 450 core
#extension GL_NV_shader_atomic_float : require

layout(binding = 0) uniform usampler2D iChannel0;
#include "../libs/text.glsl"
makeStr(printSeperator) _ 45 _end
makeStr1i(printDynNum) _dig(i) _end
makeStr1i(printChar) _ i _end

#define M_PI 3.1415926535897932384626433832795
#define not !

layout(origin_upper_left) in vec4 gl_FragCoord;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out ivec2 faceAndWindow;

in VsOut{
  vec3 wPosition;
} frag;

layout(r16i, binding = 0) uniform iimage1D gWindowIndexArray;
layout(r8i, binding = 1) uniform iimage1D gBookmarks;
layout(r32f, binding = 2) uniform image2D gComponents;
layout(r32f, binding = 3) uniform image2D gMetrics;
layout(r32f, binding = 4) uniform image3D gOverlap;
layout(r8ui, binding = 6) uniform uimage1D gAttributes;
layout(r8i, binding = 7) uniform iimage1D gUIElement;

layout(std140, binding = 0) uniform Normals{
  vec3 normals[numPhysFacets];
};
layout(std140, binding = 1) uniform Centers{
  vec3 centers[numPhysTilesPerFacet];
};
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

struct Tile{
  int index;
  vec2 size;
  vec3 center;
  vec3 origin;
  vec3 xAxis;
  vec2 range;
};
Tile getTile();
Tile tile = getTile();

mat2 scaleInv(int index) {return mat2(parameters[index]);}
float height(int index) {return weights[index%numDistributions]*parameters[index][0].z;}
float df(int index) {return parameters[index][0].w;}
vec2 mean(int index) {return vec2(parameters[index][1].zw);}
float threshold(int iDis);

const int numMetrics = 3;
const int maxNumMetrics = 10;
void updateMetricsIf(bool condition);

vec4 drawBars();
vec4 drawPolarAreaChart();
vec4 bookmarkIf(bool condition);
vec4 label();

uniform vec3 colors[numDistributions];
uniform ivec2 cursorPos;

float pdf(int distribution, float x, float y);
vec3 drawStairs(vec3 color, float density);

void main() {
  updateMetricsIf(updateMetics);
  faceAndWindow = ivec2(tile.index/numComponents, tile.index%numComponents);
  const int baseIndex = tile.index*numDistributions;
  FragColor = vec4(0.0f);
  vec3 fragColor = vec3(1.0f);

  if(tile.index > -1) {
    FragColor += drawBars();
    FragColor += bookmarkIf(imageLoad(gBookmarks, tile.index).x == 1);
    FragColor += drawPolarAreaChart();
    FragColor += label();
    if(FragColor.a > 0.99f) return;

    float x = dot(frag.wPosition.xz - tile.center.xz, tile.xAxis.xz)/(tile.size.x)*tile.range.x;
    float y = ((frag.wPosition.y-tile.center.y)/tile.size.y)*tile.range.y;

    float densities[numDistributions+1];
    int indexDistributionMax = numDistributions;
    densities[numDistributions] = -1.0f;//pdf(baseIndex, x, y);
    //densities[0] = densities[0] >= threshold(baseIndex) ? densities[0] : 0.0f;

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

float[numDistributions][numDistributions] initSum(float value) {
  float[numDistributions][numDistributions] sum;
  for(int dis = 0; dis < numDistributions; ++dis)
    for(int j = 0; j < dis; ++j)
      sum[dis][j] = value;
  return sum;
}

void updateMetricsIf(bool condition) {
  if(not condition) return;

  for(int face = 0; face < numFacets; ++face) {
    for(int comp = 0; comp < numComponents; ++comp) {
      int baseIndex = face*numComponents*numDistributions + comp*numDistributions;
      int indexDistributionMax = 0;

      float x = dot(frag.wPosition.xz - tile.center.xz, tile.xAxis.xz)/(tile.size.x)*tile.range.x;
      float y = ((frag.wPosition.y-tile.center.y)/tile.size.y)*tile.range.y;

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

      imageAtomicAdd(gOverlap, ivec3(indexDistributionMax*numDistributions + indexDistributionMax, comp, face), densities[indexDistributionMax]*densities[indexDistributionMax]);
      for(int dis = 0; dis < numDistributions; ++dis) {
        for(int j = 0; j < dis; ++j) {
          imageAtomicAdd(gOverlap, ivec3(dis*numDistributions + j, comp, face), 1.0f*sum[dis][j]);
          imageAtomicAdd(gOverlap, ivec3(j*numDistributions + dis, comp, face), 1.0f*sum[dis][j]);
        }
      }
    }
  }
}

int getFacet(int physTile) {
  int faceId = physTile/numPhysTilesPerFacet;
  int turns = int(round(camDegrees/360.0f - float(faceId)/numPhysFacets));
  int facet = int(mod(faceId+numPhysFacets*turns, numFacets+2-showCanonicalBasis));
  return facet-1+showCanonicalBasis;
}

int getTileIndex() {
  int physTile = gl_PrimitiveID/2;
  int facet = getFacet(physTile);
  int tile = int(physTile)%numPhysTilesPerFacet;
  bool tileIsNotUsed = facet == -1 || facet == numFacets || tile >= numComponents;
  return tileIsNotUsed ? -1 : imageLoad(gWindowIndexArray, facet*numComponents+tile).x;
}

Tile getTile() {
  Tile tile;
  int physTile = gl_PrimitiveID/2;
  tile.index = getTileIndex();
  vec3 front = normals[physTile/numPhysTilesPerFacet];;
  vec3 up = vec3(0.0f, 1.0f, 0.0f);
  tile.xAxis = normalize(cross(up, front));
  mat3 rotation = mat3(tile.xAxis, up, front);
  tile.center = rotation*centers[physTile%numPhysTilesPerFacet];
  tile.size = vec2(tileSize);
  tile.range = vec2(userInput.domainScale);
  return tile;
}

vec4 bookmarkIf(bool condition){
  if(not condition) return vec4(0.0f);
  float x = dot(frag.wPosition.xz - tile.center.xz, tile.xAxis.xz)/(0.5f*tile.size.x);
  float y = (frag.wPosition.y - tile.center.y)/(0.5f*tile.size.y);
  float xcenter = -0.8f;
  float width = 0.2f;
  float height = 0.4f;
  float a = height/width;
  float b0 = 1.0f - a*(xcenter+0.5f*width);
  float xlim0 = xcenter-0.5f*width;
  float b1 = 1.0f + a*(xcenter-0.5f*width);
  float xlim1 = xcenter+0.5f*width;

  if((a*x+b0 < y && x > xlim0) || (-a*x+b1 < y && x < xlim1)) return vec4(1.0f, 0.0f, 0.0f, 1.0f);
  return vec4(0.0f);
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
  int facet = selectedTiles.y-2;
  vec3 color = facet < 0 ? vec3(0.0f) : 0.8*colors[facet%numDistributions];
  int selectedCanonBasis = int(0 == selectedTiles.y);
  int selectedPcaBasis = int(1 == selectedTiles.y);
  float scalar = 1.2f;

  alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+4.5/scalar, y), 120));
  alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+4.0/scalar, y), 45));
  if(1 == selectedCanonBasis) {
    int letter = int(imageLoad(gAttributes, (selectedTiles.z)*tokenLength+0).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+3.5/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles.z)*tokenLength+1).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+3/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles.z)*tokenLength+2).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+2.5/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles.z)*tokenLength+3).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+2/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles.z)*tokenLength+4).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+1.5/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles.z)*tokenLength+5).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+1/scalar, y), letter));
  } else {
    if(1 == selectedPcaBasis) {
      alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+3.5/scalar, y), 80));
      alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+3.0/scalar, y), 67));
      alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+2.5/scalar, y), 65));
    } else {
      alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+3.5/scalar, y), tmm == selectedTiles.x ? 84 : 71));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(scalar*vec2(x+3/scalar, y), (selectedTiles.y-1)/10));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(scalar*vec2(x+2.5/scalar, y), (selectedTiles.y-1)%10));
    }
    alpha += smoothstep(0.0f, 0.5f, printSeperator(scalar*vec2(x+2/scalar, y)));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(scalar*vec2(x+1.5/scalar, y), (selectedTiles.z+1)/10));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(scalar*vec2(x+1/scalar, y), (selectedTiles.z+1)%10));
  }
  fragColor = overlay(vec4(color, alpha), fragColor);
  alpha = 0.0f;
  y++;
  int selectedCanonBasisOld = selectedCanonBasis;
  facet = tile.index/numComponents-1-selectedCanonBasisOld;
  color = facet < 0 ? vec3(0.0f) : 0.8*colors[facet%numDistributions];
  selectedCanonBasis*= int(0 == tile.index/numComponents);
  int skip = int(tile.index%numComponents >= selectedTiles.z);
  alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+4.5, y), 121));
  alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+4.0, y), 45));
  if(1 == selectedCanonBasis) {
    int letter = int(imageLoad(gAttributes, (tile.index%numComponents+skip)*tokenLength+0).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+3.5, y), letter));
    letter = int(imageLoad(gAttributes, (tile.index%numComponents+skip)*tokenLength+1).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+3, y), letter));
    letter = int(imageLoad(gAttributes, (tile.index%numComponents+skip)*tokenLength+2).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+2.5, y), letter));
    letter = int(imageLoad(gAttributes, (tile.index%numComponents+skip)*tokenLength+3).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+2, y), letter));
    letter = int(imageLoad(gAttributes, (tile.index%numComponents+skip)*tokenLength+4).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+1.5, y), letter));
    letter = int(imageLoad(gAttributes, (tile.index%numComponents+skip)*tokenLength+5).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+1, y), letter));
  } else {
    if(0 == tile.index/numComponents-selectedCanonBasisOld) {
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.5, y), 80));
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.0, y), 67));
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+2.5, y), 65));
    } else {
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.5, y), tmm == mixtureModel ? 84 : 71));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+3, y), (tile.index/numComponents-selectedCanonBasisOld)/10));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+2.5, y), (tile.index/numComponents-selectedCanonBasisOld)%10));
    }
    alpha += smoothstep(0.0f, 0.5f, printSeperator(vec2(x+2, y)));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+1.5, y), (tile.index%numComponents+1)/10));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+1, y), (tile.index%numComponents+1)%10));
  }

  //fragColor = alpha > 0.001 ? vec4(color, 1.0f) : fragColor;
  fragColor = overlay(vec4(color, alpha), fragColor);
  alpha = 0.0f;
  return fragColor;
}

vec4 drawBars() {
  const float maxHeight = 0.5f;
  float x = dot(frag.wPosition.xz - tile.center.xz, tile.xAxis.xz)/(0.5f*tile.size.x)*maxNumMetrics;
  float y = (frag.wPosition.y - tile.center.y)/(0.5f*tile.size.y);
  if(y > -1.0f+maxHeight || x < maxNumMetrics-numMetrics) return vec4(0.0f);

  float yBar = (y+1.0f)/maxHeight;

  float transition = 0.01f;
  vec4 fragColor = vec4(vec3(1.0f), 0.75f);
  if(maxNumMetrics-3 < x && x < maxNumMetrics-2) {
    float value = imageLoad(gMetrics, ivec2(tile.index, 0)).x;
    fragColor = mix(vec4(0.9*vec3(179,205,227)/255.0f, 1.0f), fragColor, smoothstep(value-transition, value, yBar));
  } else if(maxNumMetrics-2 < x && x < maxNumMetrics-1) {
    float value = imageLoad(gMetrics, ivec2(tile.index, 1)).x;
    fragColor = mix(vec4(0.9*vec3(251,180,174)/255.0f, 1.0f), fragColor, smoothstep(value-transition, value, yBar));
  } else if(maxNumMetrics-1 < x && x < maxNumMetrics-0) {
    float value = imageLoad(gMetrics, ivec2(tile.index, 2)).x;
    fragColor = mix(vec4(0.9*vec3(204,235,197)/255.0f, 1.0f), fragColor, smoothstep(value-transition, value, yBar));
  }

  float num = 0.25f;
  float dist = fract(yBar/num)*num;
  float border = 0.01f;
  if(fragColor.a < 0.99) {
    vec4 fragColor2 = mix(fragColor, vec4(vec3(0.4), 1.0), smoothstep(num-(border+2.0f*transition),num-(border+transition),dist));
    fragColor = mix(fragColor2, fragColor, smoothstep(num-transition, num, dist));
  } else {
    vec4 fragColor2 = mix(fragColor, vec4(vec3(0.6), 1.0), smoothstep(num-(border+2.0f*transition),num-(border+transition),dist));
    fragColor = mix(fragColor2, fragColor, smoothstep(num-transition, num, dist));
  }
  //border = 0.05f;
  //if(maxNumMetrics-1 < x && x < maxNumMetrics-0) {
  //  float value = imageLoad(gMetrics, ivec2(tile.index, 3)).x;
  //  vec4 fragColor2= mix(fragColor, vec4(0.6*vec3(204,235,197)/255.0f, 1.0f), smoothstep(value-(border+2.0f*transition),value-(border+transition),yBar));
  //  fragColor = mix(fragColor2, fragColor, smoothstep(value-transition, value, yBar));
  //}

  return vec4(mix(fragColor, vec4(0.0f), smoothstep(1.0f-transition, 1.0f, yBar)));
}

float flowerGlyph( vec2 p,  vec2 m, int num)
{

    float h=length(m)/(1.+sqrt(3.)*tan(M_PI/float(num)/2.));
    float ra=2./sqrt(3.)*(length(m)-h)*0.95;
    m=m/length(m)*(length(m)-ra);

    vec2  q = vec2(dot(p,m), dot(p,vec2(m.y,-m.x)))/length(m);

    float x=q.x/length(m);
    float y=abs(q.y);

    float b=1.75;
    float a=1./3.;
    float res=0.005*(1.-x); //ensure starting at origin
    float f=ra/(1.+(1./a-1.)*pow(1./x-1.,b))+res; //https://arxiv.org/pdf/2308.03644.pdf


    if( x >= 1.0 )
        return sqrt(dot(p-m,p-m)) - ra;
    else if(x >=0.)
        return y-f;
    else
        return 1.;
}

float polarCoordinates(vec2 p, float len, float num)
{

    float d=length(p);
    d=(cos(2.*d*M_PI*num)-len)/(1.-len);
    return 1.-d;
}

vec4 GlyphStart(vec2 p)
{
    vec4 col = vec4(vec3(1.), 0.75);

    //float lenFlower[dimensions] = float[](0.5, 1., 0.8, 0.3, 0.6, 0.4,0.7, 0.2, 0.9 );
    float dist = 1.0;

    for(int i=0; i<dimensions; i++)
    {
        float alpha=2.*M_PI*float(i)/float(dimensions);
        float value = imageLoad(gComponents, ivec2(i, tile.index)).x;
        vec2 midPoint=vec2(cos(alpha),sin(alpha))*abs(value);
        dist=min(dist,flowerGlyph( p, midPoint,dimensions ));
    }

    col = (mix(vec4(vec3(1.),0.75),vec4(vec3(0.), 1.0),smoothstep(0.,0.01,-dist))); //black contour
    col = (mix(col,vec4(0.9*vec3(251,180,174)/255.*(1.-dist*2.), 1.0),smoothstep(0.,0.03,-dist))); //blueish color

    float val = 20*cos(dimensions*atan(p.y,p.x))+20.5;
    if(int(gl_FragCoord.x) == cursorPos.x && int(gl_FragCoord.y) == cursorPos.y) {
      float val2 = mod(dimensions*(0.5f*atan(-p.y,-p.x)/M_PI+0.5f)+0.5f,dimensions);
      imageStore(gUIElement, 0, ivec4(tile.index/numComponents));
      imageStore(gUIElement, 1, ivec4(tile.index%numComponents));
      imageStore(gUIElement, 2, ivec4(val2));
    }
    if(val < 0.95f)
      col = mix(vec4(vec3(val), 1),vec4(col), smoothstep(-0.01,-0.005,-dist));
    //if(length(p)<=1.05)
    float pc = polarCoordinates( p, 0.8,5.);
    if(pc < 0.95) {
      col = mix(vec4(vec3(pc), 1),vec4(col), smoothstep(-0.01,-0.005,-dist));
    }

    return col;
}

vec4 drawPolarAreaChart() {
  vec2 pos = vec2( dot(frag.wPosition.xz - tile.center.xz, tile.xAxis.xz)/(0.5f*tile.size.x),
                   (frag.wPosition.y - tile.center.y)/(0.5f*tile.size.y));
  vec2 center = vec2(-0.75f, -0.75f);
  float x = 4.0f*(pos.x-center.x);
  float y = 4.0f*(pos.y-center.y);
  if(x*x + y*y > 1.0f) return vec4(0.);
  vec4 col = GlyphStart(vec2(y, x)*1.02);
  return col;
  //if(col.b > 0.9999) return vec4(vec3(1.0f), 0.75);
  //return vec4(col, 1.0);
}
