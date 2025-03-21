#ifndef HULLS_ALT_GLSL
#define HULLS_ALT_GLSL

#include "Ray.glsl"
#include "Hull.glsl"
#include "distGradient_alt.glsl"

const int numHulls_alt = 2;
float hulls_alt[numHulls_alt] = float[numHulls_alt](5.0f*userInput.threshold, userInput.threshold);

vec3 rgb2hsv_alt(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

// All components are in the range [0,1], including hue.
vec3 hsv2rgb_alt(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

int modi_alt(int ai, int bi)
{
    float a = float(ai);
    float b = float(bi);
    float m = a - floor((a + 0.5) / b) * b;
    return int(floor(m + 0.5));
}

float getIsoValue_alt(int iDis, int iHull) {
  if(mixtureModel == tmm)
    return 1.0f*df(iDis)*(pow(hulls_alt[iHull]/height_alt[iDis], -2.0f/(df(iDis) + dimensions)) - 1.0f);
  else if(mixtureModel == gmm)
    return -2.0f*log(hulls_alt[iHull]/height_alt[iDis]);
}

vec2 getDeltasIntersectionPoints_alt(int iDis, int iHull, Ray ray, float t) {
  float iso_gauss = getIsoValue_alt(iDis, iHull);
  float a = dot(ray.direction, scaleInv_alt[iDis]*ray.direction);
  float b = dot(ray.direction, 2*scaleInv_camPosMinusMean[0][iDis]) + 2*t*dot(ray.direction, scaleInv_alt[iDis]*ray.direction);
  float c = camPosTMinusMeanT_ScaleInv_camPosMinusMean[0][iDis] + t*t*dot(ray.direction, scaleInv_alt[iDis]*ray.direction) + t*dot(ray.direction, 2*scaleInv_camPosMinusMean[0][iDis]) - iso_gauss;
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

Hull getNextHull_alt(Hull hullLast, Ray ray, ivec2 iDisRange) {
  bool firstTest = true;
  Hull hull = {-1, -1, 0.0f};

  for(int iDis = iDisRange.x; iDis < iDisRange.y; iDis++) {
    for(int iHull = 0; iHull < numHulls_alt; iHull++) {
      vec2 deltas = getDeltasIntersectionPoints_alt(iDis, iHull, ray, hullLast.t);
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

vec4 rayCastProcedure_alt(Ray ray, int depth, ivec2 q, float tBorder, vec3 lightPos, ivec2 iDisRange) {
  //vec3 lightPos = ray.origin;// - 0. * ray + 5. * normalize(cross(ray,vec3(2,10,-1)));// (camera.up - camera.right);
  float dashedLineThickness = .05;

  Hull hullLast = {-1, -1, 0.0f};
  Hull hull;
  float innerHull_tLast = 0.0f;

  vec4 result = vec4(0.);
  bool hitCore = false;
  bool outlineOnly = false;
  bool hitOutline = false;

  for (int i = 0; i < depth; i++) {
    hull = getNextHull_alt(hullLast, ray, iDisRange);
    if(hull.t <= hullLast.t || tBorder <= hull.t) break;
    vec3 intersect = ray.origin + hull.t*ray.direction;
    vec3 gradient = normalize(distGradient_alt(hull.iDis, ray.direction, hull.t));

    vec3 colorOuterHull = rgb2hsv_alt(colors[hull.iDis%numDistributions]);
    colorOuterHull.z = 0.0;
    colorOuterHull = hsv2rgb_alt(colorOuterHull);
    vec3 colorInnerHull = rgb2hsv_alt(colors[hull.iDis%numDistributions]);
    colorInnerHull.z *= 0.5;
    colorInnerHull = hsv2rgb_alt(colorInnerHull);
    vec4 curCol = vec4(mix(colorOuterHull, colorInnerHull, float(numHulls_alt - hull.index) / float(numHulls_alt)), 1.0f);

    vec3 lightDir = normalize(lightPos-intersect);
    vec3 viewDir = normalize(ray.origin-intersect);
    vec3 halfwayDir = normalize(lightDir+viewDir);
    float diffuse = 0.5f*max(dot(-gradient, lightDir), 0.0f);
    float specular = 1.0f*pow(max(dot(-gradient, halfwayDir), 0.0f), 128.0f);
    float fresenel = 0.9f*float(depth-i)/float(depth) * pow(1.0f - max(dot(-gradient, viewDir), 0.0f), 4.0f);

    // color the hulls_alt
    if (!outlineOnly) {
      result = result + (float(depth-i)/float(depth))*(fresenel+specular+diffuse)*curCol;
      //result = result - (result) * (1-curCol) + (0.2*fresenel + 0.8*specular + 0.2*diffuse) * (curCol) * (result);
    }
    // color the outline
    if(abs(dot(gradient, -normalize(intersect - ray.origin))) < 0.15f) {
      // we did not reach the core yet, so full color
      if(!hitCore) {
        result = colorInnerHull.rgbr * 2.0f;
      } else {
        //outline behind the color is less colorful
        result = colorInnerHull.rgbr;
        result.rgb = rgb2hsv_alt(colorInnerHull.rgb);
        result.y *= 0.75f;
        result.rgb = hsv2rgb_alt(result.rgb);
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
          if(modi_alt(p.y, 2) == 0) {
            result = result - (float(depth - i) / float(depth)) * (fresenel + specular + diffuse) * curCol;
            result.rgb = rgb2hsv_alt(result.rgb);
            result.z += 0.5f;
            result.rgb = hsv2rgb_alt(result.rgb);
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
  return vec4(result.xyz, length(result.rgb) > 0.0f ? 1.0f : 0.0f);
}


vec4 rayCastProcedure_alt(Ray ray, int depth, ivec2 q, vec3 lightPos) {
  float tBorder = 1000.0f;
  return rayCastProcedure_alt(ray, depth, q, tBorder, lightPos, ivec2(0, numDistributions));
}

#endif // HULLS_ALT_GLSL
