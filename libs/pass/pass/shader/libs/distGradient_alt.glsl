#ifndef DISTGRADIENT_ALT_GLSL
#define DISTGRADIENT_ALT_GLSL

float argmaxPdfWrtT_alt(int iDis, int cam, vec3 rayDirection) {
  return dot(rayDirection, -scaleInv_camPosMinusMean[cam][iDis])
        /dot(rayDirection, scaleInv_alt[iDis]*rayDirection);
}

float pdf_alt(int iDis, int cam, vec3 rayDirection, float t) {
  if(mixtureModel == gmm) {
    return height_alt[iDis]*exp(
      -0.5f*( camPosTMinusMeanT_ScaleInv_camPosMinusMean[cam][iDis]
             +t*dot(rayDirection, 2.0f*scaleInv_camPosMinusMean[cam][iDis])
             +t*t*dot(rayDirection, scaleInv_alt[iDis]*rayDirection))
    );
  } else {
    return height_alt[iDis]*pow(
      1.0f + 1.0f/df(iDis)*( camPosTMinusMeanT_ScaleInv_camPosMinusMean[cam][iDis]
                             +t*2.0f*dot(rayDirection, scaleInv_camPosMinusMean[cam][iDis])
                             +t*t*dot(rayDirection, scaleInv_alt[iDis]*rayDirection)),
      -0.5f*(df(iDis)+dimensions)
    );
  }
}

vec3 distGradient_alt(int iDis, vec3 rayDirection, float t) {
  //if(mixtureModel == gmm) {
    return normalize(-(scaleInv_alt[iDis]*t*rayDirection + scaleInv_camPosMinusMean[0][iDis]));
  //} else {
    //float d = -0.5*(df(iDis) + dimensions);
    //return normalize(d*pow(1.0f+1.0f/df(iDis) * ( camPosTMinusMeanT_ScaleInv_camPosMinusMean[0][iDis]
    //                                             +t*2.0f*dot(rayDirection, scaleInv_camPosMinusMean[0][iDis])
    //                                             +t*t*dot(rayDirection, scaleInv_alt[iDis]*rayDirection)), 2)
    //                 *2.0f/df(iDis)*(t*scaleInv_alt[iDis]*rayDirection + scaleInv_camPosMinusMean[0][iDis]) );
    //return normalize(-(scaleInv_alt[iDis]*t*rayDirection + scaleInv_camPosMinusMean[0][iDis]));
  //}
}

float threshold_alt(int iDis) {
  if(not userInput.adoptToStd) return userInput.threshold;
  float squaredMahalanobis = userInput.threshold*userInput.threshold;
  if(mixtureModel == gmm) {
    return height_alt[iDis] * exp(-0.5f*squaredMahalanobis);
  } else {
    return height_alt[iDis] * pow(1.0f+squaredMahalanobis/df(iDis), -0.5f*(df(iDis)+3) );
  }
}

#endif // DISTGRADIENT_ALT_GLSL
