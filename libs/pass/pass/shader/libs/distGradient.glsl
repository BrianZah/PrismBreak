#ifndef DISTGRADIENT_GLSL
#define DISTGRADIENT_GLSL

float argmaxPdfWrtT(int iDis, Ray ray) {
  return dot(ray.direction, scaleInv(iDis)*(mean(iDis)-ray.origin))
        /dot(ray.direction, scaleInv(iDis)*ray.direction);
}

float pdf(int iDis, vec3 pos) {
  float innerProduct = dot((pos-mean(iDis)), scaleInv(iDis)*(pos-mean(iDis)));
  if(mixtureModel == gmm) {
    return height(iDis) * exp(-0.5f*innerProduct);
  } else {
    return height(iDis) * pow(1.0f+innerProduct/df(iDis), -0.5f*(df(iDis)+3) );
  }
}

float threshold(int iDis) {
  if(not userInput.adoptToStd) return userInput.threshold;
  float squaredMahalanobis = userInput.threshold*userInput.threshold;
  if(mixtureModel == gmm) {
    return height(iDis) * exp(-0.5f*squaredMahalanobis);
  } else {
    return height(iDis) * pow(1.0f+squaredMahalanobis/df(iDis), -0.5f*(df(iDis)+3) );
  }
}

float pdfExceedingThreshold(int iDis, vec3 pos) {
  float density = pdf(iDis, pos);
  return density >= threshold(iDis) ? density : 0.0f;
}

vec3 distGradient(int iDis, vec3 pos) {
    return normalize(-scaleInv(iDis)*(pos-mean(iDis)));
}

#endif // DISTGRADIENT_GLSL
