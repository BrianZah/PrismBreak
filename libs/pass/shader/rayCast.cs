#version 450
#extension GL_NV_shader_atomic_float : require
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0) uniform sampler2D iChannel0;
#include "libs/text.glsl"
makeStr(printSeperator) _ 45 _end
makeStr1i(printDynNum) _dig(i) _end
makeStr1i(printChar) _ i _end
#include "libs/Ray.glsl"

#define M_PI 3.14159265358979
#define not !

layout(rgba16f, binding = 0) uniform image2D gPoints;
layout(rgba16f, binding = 1) uniform image2D gAlbedo;
layout(r32f, binding = 2) uniform image1D gSum;
layout(r16i, binding = 3) uniform iimage1D gSelected;
layout(r8ui, binding = 6) uniform uimage1D gAttributes;

uniform float[numDistributions] weights;
layout(std140, binding = 3) uniform Parameters{
  mat4 parameters[numDistributions];
};
//struct MetaData{
//  int iDis;
//  float distanceToMean;
  //float[numDistributions] probability;
//};
//layout(std140, binding = 4) buffer PointMetaData{
//  MetaData PointMetaData[numPoints];
//};
float df(int index) {return parameters[index][1].w;}
float height(int index) {return weights[index%numDistributions]*parameters[index][0].w;}
vec3 mean(int index) {return vec3(parameters[index][3]);}
mat3 scaleInv(int index) {return mat3(parameters[index]);}

uniform ivec3[3] selectedTiles;
const int screenCam = 0;
uniform ivec2 cursor;
uniform mat4 projection;
uniform mat4 view[maxNumCams];
uniform vec3 camPos[maxNumCams];
const int numCams = maxNumCams;

uniform struct UserInput{
  float threshold;
  bool adoptToStd;
} userInput = {0.005f, true};

uniform vec3 colors[numDistributions];

const int MIP = 0;
const int HULL = 1;
const int DVR = 2;

const int gmm = 1;
const int tmm = 2;

const int projDown = 0;
const int projUp = 1;

#include "libs/distGradient.glsl"
#include "libs/hulls.glsl"
#include "libs/dvr.glsl"
vec4 drawStairs(vec3 color, Ray ray, float targ, float density, int iDis);
float[maxNumCams] argmaxPdfWrtT(int iDis, Ray[maxNumCams] rays);
float[maxNumCams] pdfExceedingThreshold(int iDis, Ray[maxNumCams] rays, float[maxNumCams] t);

uniform int dimensions;
uniform float height_alt[numDistributions];
uniform mat3 scaleInv_alt[numDistributions];
uniform vec3[maxNumCams][numDistributions] scaleInv_camPosMinusMean;
uniform float[maxNumCams][numDistributions] camPosTMinusMeanT_ScaleInv_camPosMinusMean;

#include "libs/distGradient_alt.glsl"
#include "libs/hulls_alt.glsl"
#include "libs/dvr_alt.glsl"
vec4 drawStairs_alt(vec3 color, Ray ray, float targ, float density, int iDis);
float[maxNumCams] argmaxPdfWrtT_alt(int iDis, Ray[maxNumCams] rays);
float[maxNumCams] pdfExceedingThreshold_alt(int iDis, Ray[maxNumCams] rays, float[maxNumCams] t);

vec4 label();
vec4 overlay(vec4 foreground, vec4 background) {
  return (1.0f-foreground.a)*background + foreground.a*vec4(foreground.xyz, 1.0f);
}

vec3 getWorldPosfromScreenPos(vec2 screenPos, int cam);
Ray[maxNumCams] getRays();

const ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
const ivec2 imgSize = imageSize(gAlbedo);

vec4 drawAxis(Ray ray) {
  float radius = 0.001f*distance(camPos[0], vec3(0.0f));// 0.01f;
  vec4 col = vec4(0.0f);
  float a = dot(ray.direction.yz, ray.direction.yz);
  float b = 2.0f*dot(ray.origin.yz, ray.direction.yz);
  float c = dot(ray.origin.yz, ray.origin.yz)-radius*radius;
  float determinant = b*b-4*a*c;
  if(determinant >= 0) {
    float t0 = (-b + sqrt(determinant))/(2.0f*a);
    float t1 = (-b - sqrt(determinant))/(2.0f*a);
    float t = max(t0, t1);
    if(t > 1.0f && (abs(ray.origin.x+t0*ray.direction.x) < 10.0f ||  abs(ray.origin.x+t1*ray.direction.x) < 10.0f)) {
      float d = sin(acos(dot(vec3(1.0f, 0.0f, 0.0f),ray.direction)))*abs(t0-t1);
      col = vec4(vec3(0.0f), smoothstep(0.0f, 2.0f*radius, d));
    }
    //if(t > 0.0f && (abs(ray.origin.x+t0*ray.direction.x) < 12.0f ||  abs(ray.origin.x+t1*ray.direction.x) < 12.0f)) {
    //  float fontsize = 16.0f;
    //  float x = fontsize*((pixel.x/float(imgSize.y)-(float(imgSize.x)/imgSize.y-0.5f))*2.0f)-(fontsize);
    //  float y = fontsize*(pixel.y/float(imgSize.y)-0.5f)*2.0f-(fontsize-1);
    //  float alpha = smoothstep(0.0f, 1.0f, printChar(vec2(x+5, y), 120));
    //  return vec4(vec3(0.0f), alpha);
    //}
  }

  a = dot(ray.direction.xz, ray.direction.xz);
  b = 2.0f*dot(ray.origin.xz, ray.direction.xz);
  c = dot(ray.origin.xz, ray.origin.xz)-radius*radius;
  determinant = b*b-4*a*c;
  if(determinant >= 0) {
    float t0 = (-b + sqrt(determinant))/(2.0f*a);
    float t1 = (-b - sqrt(determinant))/(2.0f*a);
    float t = max(t0, t1);
    if(t > 1.0f && (abs(ray.origin.y+t0*ray.direction.y) < 10.0f ||  abs(ray.origin.y+t1*ray.direction.y) < 10.0f)) {
      float d = sin(acos(dot(vec3(0.0f, 1.0f, 0.0f),ray.direction)))*abs(t0-t1);
      float alpha = smoothstep(0.0f, 2.0f*radius, d);
      col = vec4(vec3(0.0f), alpha + col.a - alpha*col.a);
      //col = vec4(vec3(0.0f), max(alpha , col.a));
    }
  }

  a = dot(ray.direction.xy, ray.direction.xy);
  b = 2.0f*dot(ray.origin.xy, ray.direction.xy);
  c = dot(ray.origin.xy, ray.origin.xy)-radius*radius;
  determinant = b*b-4*a*c;
  if(determinant >= 0) {
    float t0 = (-b + sqrt(determinant))/(2.0f*a);
    float t1 = (-b - sqrt(determinant))/(2.0f*a);
    float t = max(t0, t1);
    if(t > 1.0f && (abs(ray.origin.z+t0*ray.direction.z) < 10.0f ||  abs(ray.origin.z+t1*ray.direction.z) < 10.0f)) {
      float d = sin(acos(dot(vec3(0.0f, 0.0f, 1.0f),ray.direction)))*abs(t0-t1);
      float alpha = smoothstep(0.0f, 2.0f*radius, d);
      col = vec4(vec3(0.0f), alpha + col.a - alpha*col.a);
      //col = vec4(vec3(0.0f), max(alpha , col.a));
    }
  }

  return col;
}

void main() {
  const Ray[maxNumCams] rays = getRays();
    vec4 albedo = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 viewPos = vec4(0.0, 0.0, -10.0, 0.0);
    vec4 normal = vec4(0.0);
  if(rendering == MIP) {
    float tMax[maxNumCams];
    float sum[maxNumCams];
    int iDisMax[maxNumCams];
    float densities[numDistributions+1][maxNumCams];
    for(int i = 0; i < numCams; ++i) {
      tMax[i] = 0.0f;
      sum[i] = 0.0f;
      iDisMax[i] = numDistributions;
      densities[numDistributions][i] = -1.0f;
    }
    if(projDown == projMethod) {
      //tMax = argmaxPdfWrtT(0, rays);
      //densities[0] = pdfExceedingThreshold(0, rays, tMax);


      for(int iDis = 0; iDis < numDistributions; ++iDis) {
        if(colors[iDis].r*colors[iDis].g*colors[iDis].b > 0.99f) continue;
        float tTmp[maxNumCams] = argmaxPdfWrtT(iDis, rays);
        densities[iDis] = pdfExceedingThreshold(iDis, rays, tTmp);

        for(int i = 0; i < numCams; ++i) {
          for(int j = 0; j < iDis; ++j) sum[i]-= numDistributions*densities[iDis][i]*densities[j][i];
          sum[i]+= 0.5f*numDistributions*(numDistributions-1)*densities[iDis][i]*densities[iDis][i];
          if(densities[iDis][i] > densities[iDisMax[i]][i]) {
            if(i == screenCam) iDisMax[i] = iDis;
            tMax[i] = tTmp[i];
          }
        }
      }
    } else if(projUp == projMethod) {
      tMax = argmaxPdfWrtT_alt(0, rays);
      densities[0] = pdfExceedingThreshold_alt(0, rays, tMax);

      for(int i = 0; i < numCams; ++i) {
        iDisMax[i] = 0;
        sum[i] = //(densities[0][i] >= userInput.threshold)
                  //?
                  0.5f*numDistributions*(numDistributions-1)*densities[0][i]*densities[0][i];
                  //: 0.0f;
      }
      //float maxD=0;

      for(int iDis = 1; iDis < numDistributions; ++iDis) {
        float tTmp[maxNumCams] = argmaxPdfWrtT_alt(iDis, rays);
        densities[iDis] = pdfExceedingThreshold_alt(iDis, rays, tTmp);

    	//if( maxD < densities[iDis][0])
    	//		maxD=densities[iDis][0];

        for(int i = 0; i < numCams; ++i) {
          for(int j = 0; j < iDis; ++j)
            //if(min(densities[iDis][i], densities[j][i]) > userInput.threshold)
              sum[i]-= numDistributions*densities[iDis][i]*densities[j][i];
          //if(densities[iDis][i] > userInput.threshold)
            sum[i]+= 0.5f*numDistributions*(numDistributions-1)*densities[iDis][i]*densities[iDis][i];
          if(densities[iDis][i] > densities[iDisMax[i]][i]) {
            if(i == screenCam) iDisMax[i] = iDis;
            tMax[i] = tTmp[i];
          }
        }
      }
    }

    //for(int i = 0; i < numCams; ++i) imageAtomicAdd(gSum, i, sum[i]);

    vec4 points = imageLoad(gPoints, pixel);
    if(points.r > 0.0f) {
      albedo = vec4((1.0f-0.33f*points.r)*colors[int(points.g+0.5f)%numDistributions], 1.0f)*vec4(1.0f);
      if(pixel.x == cursor.x && pixel.y == cursor.y) {
        imageStore(gSelected, 0, ivec4(points.y+0.5f));
        imageStore(gSelected, 1, ivec4(points.a+0.5f));
      }
    } else
    if(densities[iDisMax[screenCam]][screenCam] > 0.0f) {
      if(projDown == projMethod) {
        albedo = drawStairs(colors[iDisMax[0]%numDistributions], rays[0], tMax[0], densities[iDisMax[0]][0], iDisMax[0]);
        if(pixel.x == cursor.x && pixel.y == cursor.y) imageStore(gSelected, 0, ivec4(iDisMax[0]));
      } else if(projUp == projMethod) {
        albedo = drawStairs_alt(colors[iDisMax[0]%numDistributions], rays[0], tMax[0], densities[iDisMax[0]][0], iDisMax[0]);
      }
    }
  }
  if(rendering == HULL) {
    vec4 points = imageLoad(gPoints, pixel);
    if(points.r > 0.0f) {
      albedo = vec4((1.0f-0.33f*points.r)*colors[int(points.g+0.5f)%numDistributions], 1.0f)*vec4(1.0f);
      if(pixel.x == cursor.x && pixel.y == cursor.y) {
        imageStore(gSelected, 0, ivec4(points.y+0.5f));
        imageStore(gSelected, 1, ivec4(points.a+0.5f));
      }
    } else {
      vec3 lightPos = camPos[0];// + 50*mat3(view[0])*normalize(vec3(1, 1, 0));
      if(projDown == projMethod) {
        albedo = overlay(rayCastProcedure(rays[0],12,ivec2(gl_GlobalInvocationID.xy),lightPos), albedo);
      } else if(projUp == projMethod) {
        albedo = overlay(rayCastProcedure_alt(rays[0],12,ivec2(gl_GlobalInvocationID.xy),lightPos), albedo);
      }
    }
  }
  if(rendering == DVR) {
    vec4 points = imageLoad(gPoints, pixel);
    if(points.r > 0.0f) {
      albedo = vec4((1.0f-0.33f*points.r)*colors[int(points.g+0.5f)%numDistributions], 1.0f)*vec4(1.0f);
      if(pixel.x == cursor.x && pixel.y == cursor.y) {
        imageStore(gSelected, 0, ivec4(points.y+0.5f));
        imageStore(gSelected, 1, ivec4(points.a+0.5f));
      }
    } else {
      if(projDown == projMethod) {
        albedo = overlay(DVRProcedure(rays[0]), albedo);
      } else if(projUp == projMethod) {
        albedo = overlay(DVRProcedure_alt(rays[0]), albedo);
      }
    }
  }

  albedo = overlay(label(), albedo);
  //albedo = overlay(drawAxis(rays[0]), albedo);

  imageStore(gAlbedo, pixel, albedo);
}

Ray[maxNumCams] getRays() {
  Ray rays[maxNumCams];
  for(int i = 0; i < numCams; ++i)
    rays[i] = getRay(getWorldPosfromScreenPos((1.0f*pixel)/imgSize, i), camPos[i], true);
  return rays;
}
vec3 getWorldPosfromScreenPos(vec2 screenPos, int cam) {
  vec4 worldPos = inverse(projection*view[cam])*vec4((2.0f*screenPos-1.0f), -1.0f, 1.0f);
  return worldPos.xyz/worldPos.w;
}

vec4 drawStairs(vec3 color, Ray ray, float targ, float density, int iDis) {
  /*uniform*/ float steps = 2.5f;
  /*uniform*/ float step_width = 25.0f;

    float growing = 0.75f;
    float dist_max = -2*log(density); // <----NEW
    dist_max = dist_max / (growing * growing);
    dist_max = dist_max - steps * floor(dist_max / steps);

    //float targ = argmaxPdf_wrt_t(iDis, 0);    // <----NEW
    //vec3 dist_gr = -normalize(vec3(frag.mean[iDis]-(ray.origin+targ*ray.direction)));   // <----NEW
    vec3 dist_gr = distGradient(iDis, ray.origin + targ*ray.direction);
    ////vec3 dist_gr = normalize(-1.0f/df[iDis]*(2.0f*scaleInv[iDis]*ray.direction - 2.0f*scaleInv[iDis]*(mean[iDis]-ray.origin)));

    float t = -dist_max * dist_max * step_width + dot(dist_gr.xy, dist_gr.xy) / (growing * growing);
    t = 1.0f- smoothstep(-1.0f, 0.0f, t);

    float gd = dist_gr.x + dist_gr.y;
    float t_f = (t + 1.0f) / 2.0f;

    t = -dist_max * dist_max * 2. * step_width + dot(dist_gr.xy, dist_gr.xy) / (growing * growing);
    t = 1.0f- smoothstep(-1.0f, -0.0f, t);

    vec3 returnCol = t_f*color.rgb + (1.0f-t)*(color.rgb*0.75f + vec3(0.5f)*gd);

    return vec4(returnCol, 1.0f);
}

float[maxNumCams] argmaxPdfWrtT(int iDis, Ray[maxNumCams] rays) {
  float argmax[maxNumCams];
  for(int i = 0; i < numCams; ++i) argmax[i] = argmaxPdfWrtT(iDis, rays[i]);
  return argmax;
}
float[maxNumCams] pdfExceedingThreshold(int iDis, Ray[maxNumCams] rays, float[maxNumCams] t) {
  float density[maxNumCams];
  for(int i = 0; i < numCams; ++i) {
    vec3 pos = rays[i].origin + t[i]*rays[i].direction;
    density[i] = t[i] >= 0 ? pdf(iDis, pos) : 0.0f;
    density[i] = density[i] >= threshold(iDis) ? density[i] : 0.0f;
  }
  return density;
}

vec4 drawStairs_alt(vec3 color, Ray ray, float targ, float density, int iDis) {
  float steps = 5.5;
  float step_width = 2.;

    float growing = 0.75f;
    float dist_max = -2*log(density);
	  dist_max = dist_max / (growing * growing);
    dist_max = dist_max - steps * floor(dist_max / steps);

	//float targ=argmaxPdfWrtT_alt(iDis, 0, ray.direction);
	//vec3 dist_gr=-normalize(mean[iDis]-(ray.origin+targ*ray.direction));
    vec3 dist_gr = distGradient_alt(iDis, ray.direction, targ);
	//vec3 dist_gr = normalize(-1.0f/df[iDis]*(2.0f*scaleInv[iDis]*ray.direction - 2.0f*scaleInv[iDis]*(mean[iDis]-ray.origin)));


    float t = -dist_max * dist_max * step_width + dot(dist_gr.xy, dist_gr.xy) / (growing * growing);
    t = 1.- smoothstep(-1., 0., t);

    float gd = dist_gr.x + dist_gr.y;
    float t_f = (t + 1.) / 2.;

    t = -dist_max * dist_max * 2. * step_width + dot(dist_gr.xy, dist_gr.xy) / (growing * growing);
    t = 1.- smoothstep(-1., -0., t);

    vec3 returnCol;
    returnCol=(t_f) * color.rgb + (1. - t) * (color.rgb * 0.75 + vec3(0.5) * gd);

    return vec4(returnCol, 1.0f);
}
float[maxNumCams] argmaxPdfWrtT_alt(int iDis, Ray[maxNumCams] rays) {
  float argmax[maxNumCams];
  for(int i = 0; i < numCams; ++i) argmax[i] = argmaxPdfWrtT_alt(iDis, i, rays[i].direction);
  return argmax;
}

float[maxNumCams] pdfExceedingThreshold_alt(int iDis, Ray[maxNumCams] rays, float[maxNumCams] t) {
  float density[maxNumCams];
  for(int i = 0; i < numCams; ++i) {
    density[i] = t[i] >= 0 ? pdf_alt(iDis, i, rays[i].direction, t[i]) : 0.0f;
    density[i] = density[i] >= threshold_alt(iDis) ? density[i] : 0.0f;
  }
  return density;
}

vec4 label() {
  float fontsize = 16.0f;
  int stage = 2;
  float x = fontsize*((pixel.x/float(imgSize.y)-(float(imgSize.x)/imgSize.y-0.5f))*2.0f)-(fontsize);
  float y = fontsize*(pixel.y/float(imgSize.y)-0.5f)*2.0f-(fontsize-1);
  if(y < -1/fontsize-stage || x < -4.5) return vec4(0.0);

  vec4 fragColor = vec4(1.0f, 1.0f, 1.0f, 0.75f);
  float alpha = 0.0f;
  int facet = selectedTiles[0].y-2;
  vec3 color = facet < 0 ? vec3(0.0f) : 0.8*colors[facet];
  int selectedCanonBasis = int(0 == selectedTiles[0].y);
  int selectedPcaBasis = int(1 == selectedTiles[0].y);
  float scalar = 1.0f;

  alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+4.5/scalar, y), 120));
  alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+4.0/scalar, y), 45));
  if(1 == selectedCanonBasis) {
    int letter = int(imageLoad(gAttributes, (selectedTiles[0].z)*tokenLength+0).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+3.5/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[0].z)*tokenLength+1).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+3/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[0].z)*tokenLength+2).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+2.5/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[0].z)*tokenLength+3).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+2/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[0].z)*tokenLength+4).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+1.5/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[0].z)*tokenLength+5).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+1/scalar, y), letter));
  } else {
    if(1 == selectedPcaBasis) {
      alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+3.5/scalar, y), 80));
      alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+3.0/scalar, y), 67));
      alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+2.5/scalar, y), 65));
    } else {
      alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+3.5/scalar, y), tmm == selectedTiles[0].x ? 84 : 71));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(scalar*vec2(x+3/scalar, y), (selectedTiles[0].y-1)/10));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(scalar*vec2(x+2.5/scalar, y), (selectedTiles[0].y-1)%10));
    }
    alpha += smoothstep(0.0f, 0.5f, printSeperator(scalar*vec2(x+2/scalar, y)));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(scalar*vec2(x+1.5/scalar, y), (selectedTiles[0].z+1)/10));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(scalar*vec2(x+1/scalar, y), (selectedTiles[0].z+1)%10));
  }
  fragColor = overlay(vec4(color, alpha), fragColor);
  alpha = 0.0f;
  y++;
  int selectedCanonBasisOld = selectedCanonBasis;
  facet = selectedTiles[1].y-1-selectedCanonBasisOld;
  color = facet < 0 ? vec3(0.0f) : 0.8*colors[facet];
  selectedCanonBasis*= int(0 == selectedTiles[1].y);
  selectedPcaBasis = int(0 == selectedTiles[1].y-selectedCanonBasisOld);
  int skip = int(selectedTiles[1].z >= selectedTiles[0].z);
  alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+4.5/scalar, y), 121));
  alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+4.0/scalar, y), 45));
  if(1 == selectedCanonBasis) {
    int letter = int(imageLoad(gAttributes, (selectedTiles[1].z+skip)*tokenLength+0).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+3.5/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[1].z+skip)*tokenLength+1).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+3/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[1].z+skip)*tokenLength+2).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+2.5/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[1].z+skip)*tokenLength+3).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+2/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[1].z+skip)*tokenLength+4).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+1.5/scalar, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[1].z+skip)*tokenLength+5).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(scalar*vec2(x+1/scalar, y), letter));
  } else {
    if(1 == selectedPcaBasis) {
      alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+3.5/scalar, y), 80));
      alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+3.0/scalar, y), 67));
      alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+2.5/scalar, y), 65));
    } else {
      alpha += smoothstep(0.0f, 1.0f, printChar(scalar*vec2(x+3.5/scalar, y), tmm == selectedTiles[1].x ? 84 : 71));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(scalar*vec2(x+3/scalar, y), (selectedTiles[1].y-selectedCanonBasisOld)/10));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(scalar*vec2(x+2.5/scalar, y), (selectedTiles[1].y-selectedCanonBasisOld)%10));
    }
    alpha += smoothstep(0.0f, 0.5f, printSeperator(scalar*vec2(x+2/scalar, y)));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(scalar*vec2(x+1.5/scalar, y), (selectedTiles[1].z+1)/10));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(scalar*vec2(x+1/scalar, y), (selectedTiles[1].z+1)%10));
  }
  fragColor = overlay(vec4(color, alpha), fragColor);
  alpha = 0.0f;
  y++;
  selectedCanonBasisOld = selectedCanonBasis;
  facet = selectedTiles[0+2].y - 1 - selectedCanonBasisOld;
  color = facet < 0 ? vec3(0.0f) : 0.8*colors[facet];
  selectedCanonBasis*= int(0 == selectedTiles[0+2].y);
  selectedPcaBasis = int(0 == selectedTiles[0+2].y-selectedCanonBasisOld);
  int prevskip = int(selectedTiles[0+2].z >= selectedTiles[0+1].z);
  skip = int(selectedTiles[0+2].z+prevskip >= selectedTiles[0].z) + prevskip;
  alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+4.5, y), 122));
  alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+4.0, y), 45));
  if(1 == selectedCanonBasis) {
    int letter = int(imageLoad(gAttributes, (selectedTiles[0+2].z+skip)*tokenLength+0).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+3.5, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[0+2].z+skip)*tokenLength+1).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+3, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[0+2].z+skip)*tokenLength+2).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+2.5, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[0+2].z+skip)*tokenLength+3).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+2, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[0+2].z+skip)*tokenLength+4).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+1.5, y), letter));
    letter = int(imageLoad(gAttributes, (selectedTiles[0+2].z+skip)*tokenLength+5).x);
    alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+1, y), letter));
  } else {
    if(1 == selectedPcaBasis) {
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.5, y), 80));
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+3.0, y), 67));
      alpha += smoothstep(0.0f, 1.0f, printChar(vec2(x+2.5, y), 65));
    } else {
      alpha += smoothstep(0.0f, 0.5f, printChar(vec2(x+3.5, y), tmm == selectedTiles[2].x ? 84 : 71));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+3, y), (selectedTiles[0+2].y-selectedCanonBasisOld)/10));
      alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+2.5, y), (selectedTiles[0+2].y-selectedCanonBasisOld)%10));
    }
    alpha += smoothstep(0.0f, 0.5f, printSeperator(vec2(x+2, y)));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+1.5, y), (selectedTiles[0+2].z+1)/10));
    alpha += smoothstep(0.0f, 0.5f, printDynNum(vec2(x+1, y), (selectedTiles[0+2].z+1)%10));
  }
  fragColor = overlay(vec4(color, alpha), fragColor);
  alpha = 0.0f;
  return fragColor;
}
