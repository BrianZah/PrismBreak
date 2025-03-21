#include "context/Context.hpp"

#include <vector>
#include <numbers>
#include <cmath>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <glm/glm.hpp>

#include "csv/csv.hpp"
#include "context/TDistribution.hpp"

namespace context{
std::vector<float> Context::getParametersStd140Gmm1D() const {
  std::vector<float> parameters;
  int size = mComponentsLevel0.size()*mComponentsLevel0[0].cols()*mGMM.size();
  parameters.reserve(4*size);

  for(const auto& components : mComponentsLevel0) {
    for(const auto& component : components.colwise()) {
      for(const auto& distribution : mGMM) {
        float mean = (component.transpose()*distribution.mean)(0,0);
        float scale = (component.transpose()*distribution.scale*component)(0,0);
        float height = 1.0f / std::sqrt(2.0f*std::numbers::pi*scale);
        parameters.insert(parameters.end(), {1.0f/(scale), height, 0.0f, mean});
      }
    }
  }
  return parameters;
}

std::vector<float> Context::getParametersStd140Gmm2D() const {
  std::vector<float> parameters;
  int size = mComponentsLevel1.size()*mComponentsLevel1[0].cols()*mGMM.size();
  parameters.reserve(8*size);

  for(const auto& components : mComponentsLevel1) {
    for(const auto& component : components.colwise()) {
      Eigen::MatrixX2f viewBox(mViewBox.rows(), 2);
      viewBox.col(0) = mViewBox.col(0);
      viewBox.col(1) = component;
      for(const auto& distribution : mGMM) {
        Eigen::Vector2f mean = viewBox.transpose()*distribution.mean;
        Eigen::Matrix2f scale = viewBox.transpose()*distribution.scale*viewBox;
        Eigen::Matrix2f scaleInv = scale.inverse();
        float height = 1.0f / (2.0f*std::numbers::pi*std::sqrt(scale.determinant()));
        parameters.insert(parameters.end(), {scaleInv(0, 0), scaleInv(0, 1), height, 0.0f,
                                             scaleInv(1, 0), scaleInv(1, 1), mean.x(), mean.y()});
      }
    }
  }
  return parameters;
}

std::vector<float> Context::getParametersStd140Gmm3D() const {
  std::vector<float> parameters;
  int size = mComponentsLevel2.size()*mComponentsLevel2[0].cols()*mGMM.size();
  parameters.reserve(16*size);

  for(const auto& components : mComponentsLevel2) {
    for(const auto& component : components.colwise()) {
      Eigen::MatrixX3f viewBox(mViewBox.rows(), 3);
      viewBox.col(0) = mViewBox.col(0);
      viewBox.col(1) = mViewBox.col(1);
      viewBox.col(2) = component;
      for(const auto& distribution : mGMM) {
        Eigen::Vector3f mean = viewBox.transpose()*distribution.mean;
        Eigen::Matrix3f scale = viewBox.transpose()*distribution.scale*viewBox;
        Eigen::Matrix3f scaleInv = scale.inverse();
        float height = 1.0f / std::sqrt(std::pow(2.0f*std::numbers::pi, 3)*scale.determinant());
        parameters.insert(parameters.end(), {scaleInv(0, 0), scaleInv(0, 1), scaleInv(0, 2), height,
                                             scaleInv(1, 0), scaleInv(1, 1), scaleInv(1, 2), 0.0f,
                                             scaleInv(2, 0), scaleInv(2, 1), scaleInv(2, 2), 0.0f,
                                             mean.x(), mean.y(), mean.z(), 0.0f});
      }
    }
  }
  return parameters;
}

std::vector<float> Context::getParametersStd140GmmDetailView() const {
  std::vector<float> parameters;
  int size = mGMM.size();
  parameters.reserve(16*size);

  for(const auto& distribution : mGMM) {
    Eigen::Vector3f mean = mViewBox.transpose()*distribution.mean;
    Eigen::Matrix3f scale = mViewBox.transpose()*distribution.scale*mViewBox;
    Eigen::Matrix3f scaleInv = scale.inverse();
    float height = 1.0f / std::sqrt(std::pow(2.0f*std::numbers::pi, 3)*scale.determinant());
    parameters.insert(parameters.end(), {scaleInv(0, 0), scaleInv(0, 1), scaleInv(0, 2), height,
                                         scaleInv(1, 0), scaleInv(1, 1), scaleInv(1, 2), 0.0f,
                                         scaleInv(2, 0), scaleInv(2, 1), scaleInv(2, 2), 0.0f,
                                         mean.x(), mean.y(), mean.z(), 0.0f});
  }
  return parameters;
}

std::vector<float> Context::getParametersStd140Gmm1D(const int& iCompSet, const int& iComp) const {
  std::vector<float> parameters;
  parameters.reserve(4);

  Eigen::VectorXf viewBox(mViewBox.rows());
  viewBox.col(0) = mComponentsLevel0[iCompSet].col(iComp);

  for(const auto& distribution : mGMM) {
    float mean = (viewBox.transpose()*distribution.mean)(0,0);
    float scale = (viewBox.transpose()*distribution.scale*viewBox)(0,0);
    float height = 1.0f / std::sqrt(2.0f*std::numbers::pi*scale);
    parameters.insert(parameters.end(), {1.0f/scale, height, 0.0f, mean});
  }
  return parameters;
}

std::vector<float> Context::getParametersStd140Gmm2D(const int& iCompSet, const int& iComp) const {
  std::vector<float> parameters;
  parameters.reserve(8*mGMM.size());

  int showedCanonBasis = mComponentsLevel1.size() == (mGMM.size() + 2);
  Eigen::MatrixX2f viewBox(mViewBox.rows(), 2);
  viewBox.col(0) = mViewBox.col(0);
  viewBox.col(1) = mComponentsLevel1[iCompSet].col(iComp);

  for(const auto& distribution : mGMM) {
    Eigen::Vector2f mean = viewBox.transpose()*distribution.mean;
    Eigen::Matrix2f scale = viewBox.transpose()*distribution.scale*viewBox;
    Eigen::Matrix2f scaleInv = scale.inverse();
    float height = 1.0f / (2.0f*std::numbers::pi*std::sqrt(scale.determinant()));
    parameters.insert(parameters.end(),
                      {scaleInv(0, 0), scaleInv(0, 1), height, 0.0f,
                       scaleInv(1, 0), scaleInv(1, 1), mean.x(), mean.y()});
  }
  return parameters;
}

std::vector<float> Context::getParametersStd140Gmm3D(const int& iCompSet, const int& iComp) const {
  std::vector<float> parameters;
  parameters.reserve(16*mGMM.size());

  int showedCanonBasis = mComponentsLevel2.size() == (mGMM.size() + 2);
  Eigen::MatrixX3f viewBox(mViewBox.rows(), 3);
  viewBox.col(0) = mViewBox.col(0);
  viewBox.col(1) = mViewBox.col(1);
  viewBox.col(2) = mComponentsLevel2[iCompSet].col(iComp);

  for(const auto& distribution : mGMM) {
    Eigen::Vector3f mean = viewBox.transpose()*distribution.mean;
    Eigen::Matrix3f scale = viewBox.transpose()*distribution.scale*viewBox;
    Eigen::Matrix3f scaleInv = scale.inverse();
    float height = 1.0f / std::sqrt(std::pow(2.0f*std::numbers::pi, 3)*scale.determinant());
    parameters.insert(parameters.end(), {scaleInv(0, 0), scaleInv(0, 1), scaleInv(0, 2), height,
                                         scaleInv(1, 0), scaleInv(1, 1), scaleInv(1, 2), 0.0f,
                                         scaleInv(2, 0), scaleInv(2, 1), scaleInv(2, 2), 0.0f,
                                         mean.x(), mean.y(), mean.z(), 0.0f});
  }
  return parameters;
}

std::vector<float> Context::getHightGmm() const {
  std::vector<float> hights(mGMM.size());
  for(int i = 0; i < hights.size(); ++i) hights[i] = mGMM[i].hight();
  return hights;
}

std::vector<float> Context::getScaleInvGmm() const {
  std::vector<float> scaleInvs;
  scaleInvs.reserve(9*mGMM.size());
  for(int i = 0; i < mGMM.size(); ++i) {
    Eigen::Matrix3f result = mViewBox.transpose()*mGMM[i].scaleInv*mViewBox;
    scaleInvs.insert(scaleInvs.end(), {result(0,0), result(0,1), result(0,2),
                                       result(1,0), result(1,1), result(1,2),
                                       result(2,0), result(2,1), result(2,2)});
  }
  return scaleInvs;
}

std::vector<float> Context::getCamPosTMinusMeanT_ScaleInv_camPosMinusMeanGmm(const glm::vec3& camPos) const {
  std::vector<float> result(mGMM.size());
  for(int i = 0; i < result.size(); ++i) {
    Eigen::Vector3f campos = {camPos.x, camPos.y, camPos.z};
    result[i] = ( (mViewBox*campos - mGMM[i].mean).transpose()
                 *mGMM[i].scaleInv
                 *(mViewBox*campos - mGMM[i].mean) )[0];
  }
  return result;
}

std::vector<float> Context::getScaleInv_camPosMinusMeanGmm(const glm::vec3& camPos) const {
  std::vector<float> result(3*mGMM.size());
  for(int i = 0; i < mGMM.size(); ++i) {
    Eigen::Vector3f campos = {camPos.x, camPos.y, camPos.z};
    Eigen::Vector3f tmp = mViewBox.transpose()*mGMM[i].scaleInv
                         *(mViewBox*campos - mGMM[i].mean);
    result[3*i+0] = tmp.x();
    result[3*i+1] = tmp.y();
    result[3*i+2] = tmp.z();
  }
  return result;
}
} // namespace context
