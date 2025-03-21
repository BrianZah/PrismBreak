#ifndef HULLS_GLSL
#define HULLS_GLSL

#include "Ray.glsl"
#include "Hull.glsl"
#include "distGradient.glsl"

const int numHulls = 2;
float hulls[numHulls] = float[numHulls](5.0f*userInput.threshold, userInput.threshold);

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

// All components are in the range [0,1], including hue.
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

int modi(int ai, int bi)
{
    float a = float(ai);
    float b = float(bi);
    float m = a - floor((a + 0.5) / b) * b;
    return int(floor(m + 0.5));
}

float getIsoValue(int iDis, int iHull) {
  if(mixtureModel == tmm)
    return 1.0f*df(iDis)*(pow(hulls[iHull]/height(iDis), -2.0f/(df(iDis) + 3.0f)) - 1.0f);
  else if(mixtureModel == gmm)
    return -2.0f*log(hulls[iHull]/height(iDis));
}

vec2 getDeltasIntersectionPoints(int iDis, int iHull, Ray ray, float t) {
  float iso_gauss = getIsoValue(iDis, iHull);
  vec3 pos = ray.origin + t*ray.direction;
  float a = dot(ray.direction, scaleInv(iDis)*ray.direction);
  float b = 2. * dot(ray.direction, scaleInv(iDis)*pos) - 2*dot(ray.direction, scaleInv(iDis)*mean(iDis));
  float c = dot(mean(iDis), scaleInv(iDis)*mean(iDis)) - 2*dot(pos, scaleInv(iDis)*mean(iDis)) + dot(pos, scaleInv(iDis)*pos) - iso_gauss;
  float determinant = b * b - 4. * a * c;

  vec2 deltas = vec2(-1.0f);
  if(determinant >= 0.) {
    float delta0 = (-b + sqrt(determinant)) / (2. * a);
    float delta1 = (-b - sqrt(determinant)) / (2. * a);
    deltas.x = min(delta0, delta1);
    deltas.y = max(delta0, delta1);
    if(deltas.x <= 0.0) deltas.x = deltas.y;
    if(deltas.y <= 0.0) deltas.y = deltas.x;
  }
  return deltas;
}

Hull getNextHull(Hull hullLast, Ray ray, ivec2 iDisRange) {
  bool firstTest = true;
  Hull hull = {-1, -1, 0.0f};

  for(int iDis = iDisRange.x; iDis < iDisRange.y; iDis++) {
    for(int iHull = 0; iHull < numHulls; iHull++) {
      vec2 deltas = getDeltasIntersectionPoints(iDis, iHull, ray, hullLast.t);
      if(deltas.x <= 0.0f) continue;
      float delta = (iHull == hullLast.index && iDis == hullLast.iDis) ? deltas.y : deltas.x;
      if(hullLast.t+delta < hull.t || firstTest) {
        firstTest = false;
        hull.t = hullLast.t+delta;
        hull.iDis = iDis;
        hull.index = iHull;
      }
    }
  }
  return hull;
}

vec4 rayCastProcedure(Ray ray, int depth, ivec2 q, float tBorder, ivec2 iDisRange) {
  vec3 lightPos = ray.origin;// - 0. * ray + 5. * normalize(cross(ray,vec3(2,10,-1)));// (camera.up - camera.right);
  float dashedLineThickness = .05;

  Hull hullLast = {-1, -1, 0.0f};
  Hull hull;
  float innerHull_tLast = 0.0f;

  //vec4 result = vec4(0.0f);
  vec4 result = vec4(1.0f);
  bool hitCore = false;
  bool outlineOnly = false;
  bool hitOutline = false;

  for (int i = 0; i < depth; i++) {
    hull = getNextHull(hullLast, ray, iDisRange);
    if(hull.t <= hullLast.t || tBorder <= hull.t) break;
    vec3 intersect = ray.origin + hull.t*ray.direction;
    vec3 gradient = normalize(distGradient(hull.iDis, intersect));

    vec3 colorOuterHull = rgb2hsv(colors[modi(hull.iDis, numDistributions)]);
    colorOuterHull.z = 0.0;
    colorOuterHull = hsv2rgb(colorOuterHull);
    vec3 colorInnerHull = rgb2hsv(colors[modi(hull.iDis, numDistributions)]);
    //colorInnerHull.z *= 0.5;
    colorInnerHull.z *= 0.8;
    colorInnerHull = hsv2rgb(colorInnerHull);
    //vec4 curCol = vec4(mix(colorOuterHull, colorInnerHull, float(numHulls - hull.index) / float(numHulls)), 1.0f);
    vec4 curCol = vec4(colorInnerHull, 1.0f);

    vec3 lightDir = normalize(lightPos-intersect);
    vec3 viewDir = normalize(ray.origin-intersect);
    vec3 halfwayDir = normalize(lightDir+viewDir);
    //float diffuse = 0.3f*max(dot(-gradient, lightDir), 0.0f);
    float diffuse = max(dot(-gradient, lightDir), 0.0f);
    float specular = 1.0f*pow(max(dot(-gradient, halfwayDir), 0.0f), 64.0f);
    //float fresenel = 0.7f*float(depth-i)/float(depth) * pow(1.0f - max(dot(-gradient, viewDir), 0.0f), 4.0f);
    float fresenel = pow(1.0f - max(dot(-gradient, viewDir), 0.0f), 4.0f);

    // color the hulls
    if (!outlineOnly) {
      //result = result + (float(depth-i)/float(depth))*(fresenel+specular+diffuse)*curCol;
      result = result - (result) * (1-curCol) + (0.2*fresenel + 0.8*specular + 0.2*diffuse) * (curCol) * (result);
    }
    // color the outline
    if(abs(dot(gradient, -normalize(intersect - ray.origin))) < 0.15) {
      // we did not reach the core yet, so full color
      if(!hitCore) {
        //result = colorInnerHull.rgbr * 2.;
        result = colorInnerHull.rgbr;
      } else {
        //outline behind the color is less colorful
        result = colorInnerHull.rgbr;
        result.rgb = rgb2hsv(colorInnerHull.rgb);
        result.y *= 0.75;
        result.rgb = hsv2rgb(result.rgb);
      }
      break;
    }
    // we reached the core
    if(hull.index == 0 && !outlineOnly) {
      float front_face = dot(-gradient, -normalize(intersect - ray.origin));
      // front of the core and it is the first core hit
      if(front_face >= 0.0f && !hitCore) {
        result = (fresenel + specular + diffuse) * colorInnerHull.rgbr;
      }
      // we already hit the core (core inside the core)
      if(front_face >= 0.0f && hitCore) {
        ivec2 p = q / 10;
        if(hull.t-innerHull_tLast < dashedLineThickness) {
          if(modi(p.y, 2) == 0) {
            //result = result - (float(depth - i) / float(depth)) * (fresenel + specular + diffuse) * curCol;
            //result.rgb = rgb2hsv(result.rgb);
            //result.z += .5;
            //result.rgb = hsv2rgb(result.rgb);
            result = colorInnerHull.rgbr;//esult - 0.2*(fres_sum + spec_sum + diff_sum) * (col_sum) * (result)+ (result) * (1-col_sum);//col_sum *result;
            break;
          }
        }
        outlineOnly = true;
      }
      if(!hitCore) hitCore = true;
      innerHull_tLast = hull.t;
    }
    hullLast = hull;
    hullLast.t += 1e-3;
  }
  return length(result) > 0.0f ? vec4(result.xyz, 1.0f) : vec4(1.0f);
}


vec4 rayCastProcedure(Ray ray, int depth, ivec2 q) {
  float tBorder = 1000.0f;
  return rayCastProcedure(ray, depth, q, tBorder, ivec2(0, numDistributions));
}

#endif // HULLS_GLSL
