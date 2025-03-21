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
float Context::degreesOfFreedom(const int& index) const {
  return mTMM[index].df;
}

std::vector<float> Context::getParametersStd140Tmm1D() const {
  std::vector<float> parameters;
  int size = mComponentsLevel0.size()*mComponentsLevel0[0].cols()*mTMM.size();
  parameters.reserve(4*size);

  for(const auto& components : mComponentsLevel0) {
    for(const auto& component : components.colwise()) {
      for(const auto& distribution : mTMM) {
        float mean = (component.transpose()*distribution.mean)(0,0);
        float scale = (component.transpose()*distribution.scale*component)(0,0);
        //float height = distribution.weight*std::tgamma(0.5f*(distribution.df+1))
        //              / ( std::tgamma(0.5f*distribution.df)
        //                 *std::sqrt(distribution.df*std::numbers::pi*scale) );
        float height = 1.0f
                      / ( std::betal(0.5f*distribution.df, 0.5f)
                         *std::sqrt(distribution.df*scale) );
        parameters.insert(parameters.end(), {1.0f/(scale), height, distribution.df, mean});
      }
    }
  }
  return parameters;
}

std::vector<float> Context::getParametersStd140Tmm2D() const {
  std::vector<float> parameters;
  int size = mComponentsLevel1.size()*mComponentsLevel1[0].cols()*mTMM.size();
  parameters.reserve(8*size);

  for(const auto& components : mComponentsLevel1) {
    for(const auto& component : components.colwise()) {
      Eigen::MatrixX2f viewBox(mViewBox.rows(), 2);
      viewBox.col(0) = mViewBox.col(0);
      viewBox.col(1) = component;
      for(const auto& distribution : mTMM) {
        Eigen::Vector2f mean = viewBox.transpose()*distribution.mean;
        Eigen::Matrix2f scale = viewBox.transpose()*distribution.scale*viewBox;
        Eigen::Matrix2f scaleInv = scale.inverse();
        //float height = distribution.weight*std::tgamma(0.5f*(distribution.df+2))
        //                     /( std::tgamma(0.5f*distribution.df)
        //                        *distribution.df*std::numbers::pi
        //                        *std::sqrt(scale.determinant()) );
        float height = 1.0f*std::tgamma(1.0f)
                      /( std::betal(0.5f*distribution.df, 1.0f)
                        *std::sqrt( std::pow(distribution.df*std::numbers::pi, 2)
                                   *scale.determinant() ) );
        parameters.insert(parameters.end(), {scaleInv(0, 0), scaleInv(0, 1), height, distribution.df,
                                             scaleInv(1, 0), scaleInv(1, 1), mean.x(), mean.y()});
      }
    }
  }
  return parameters;
}

std::vector<float> Context::getParametersStd140Tmm3D() const {
  std::vector<float> parameters;
  int size = mComponentsLevel2.size()*mComponentsLevel2[0].cols()*mTMM.size();
  parameters.reserve(16*size);

  for(const auto& components : mComponentsLevel2) {
    for(const auto& component : components.colwise()) {
      Eigen::MatrixX3f viewBox(mViewBox.rows(), 3);
      viewBox.col(0) = mViewBox.col(0);
      viewBox.col(1) = mViewBox.col(1);
      viewBox.col(2) = component;
      for(const auto& distribution : mTMM) {
        Eigen::Vector3f mean = viewBox.transpose()*distribution.mean;
        Eigen::Matrix3f scale = viewBox.transpose()*distribution.scale*viewBox;
        Eigen::Matrix3f scaleInv = scale.inverse();
        //float height = distribution.weight*std::tgamma(0.5f*(distribution.df+3))
        //                     /(std::tgamma(0.5f*distribution.df)
        //                       *std::sqrt(std::pow(distribution.df*std::numbers::pi, 3)
        //                                  *scale.determinant()));
        float height = 1.0f*std::tgamma(1.5f)
                      /( std::betal(0.5f*distribution.df, 1.5f)
                        *std::sqrt( std::pow(distribution.df*std::numbers::pi, 3)
                                   *scale.determinant() ) );
        parameters.insert(parameters.end(), {scaleInv(0, 0), scaleInv(0, 1), scaleInv(0, 2), height,
                                             scaleInv(1, 0), scaleInv(1, 1), scaleInv(1, 2), distribution.df,
                                             scaleInv(2, 0), scaleInv(2, 1), scaleInv(2, 2), 0.0f,
                                             mean.x(), mean.y(), mean.z(), 0.0f});
      }
    }
  }
  return parameters;
}

std::vector<float> Context::getParametersStd140TmmDetailView() const {
  std::vector<float> parameters;
  int size = mTMM.size();
  parameters.reserve(16*size);

  for(const auto& distribution : mTMM) {
    Eigen::Vector3f mean = mViewBox.transpose()*distribution.mean;
    Eigen::Matrix3f scale = mViewBox.transpose()*distribution.scale*mViewBox;
    Eigen::Matrix3f scaleInv = scale.inverse();
    //float height = distribution.weight*std::tgamma(0.5f*(distribution.df+3))
    //                     /(std::tgamma(0.5f*distribution.df)
    //                       *std::sqrt(std::pow(distribution.df*std::numbers::pi, 3)
    //                                  *scale.determinant()));
    float height = std::tgamma(1.5f)
                  /( std::betal(0.5f*distribution.df, 1.5f)
                    *std::sqrt( std::pow(distribution.df*std::numbers::pi, 3)
                               *scale.determinant() ) );
    parameters.insert(parameters.end(), {scaleInv(0, 0), scaleInv(0, 1), scaleInv(0, 2), height,
                                         scaleInv(1, 0), scaleInv(1, 1), scaleInv(1, 2), distribution.df,
                                         scaleInv(2, 0), scaleInv(2, 1), scaleInv(2, 2), 0.0f,
                                         mean.x(), mean.y(), mean.z(), 0.0f});
  }
  return parameters;
}

std::vector<float> Context::getParametersStd140Tmm1D(const int& iCompSet, const int& iComp) const {
  std::vector<float> parameters;
  parameters.reserve(4);

  Eigen::VectorXf viewBox(mViewBox.rows());
  viewBox.col(0) = mComponentsLevel0[iCompSet].col(iComp);

  for(const auto& distribution : mTMM) {
    float mean = (viewBox.transpose()*distribution.mean)(0,0);
    float scale = (viewBox.transpose()*distribution.scale*viewBox)(0,0);
    //float height = distribution.weight*std::tgamma(0.5f*(distribution.df+1))
    //              / ( std::tgamma(0.5f*distribution.df)
    //                 *std::sqrt(distribution.df*std::numbers::pi*scale) );
    float height = 1.0f
                  / ( std::betal(0.5f*distribution.df, 0.5f)
                     *std::sqrt(distribution.df*scale) );
    parameters.insert(parameters.end(), {1.0f/scale, height, distribution.df, mean});
  }
  return parameters;
}

std::vector<float> Context::getParametersStd140Tmm2D(const int& iCompSet, const int& iComp) const {
  std::vector<float> parameters;
  parameters.reserve(8*mTMM.size());

  int showedCanonBasis = mComponentsLevel1.size() == (mTMM.size() + 2);
  Eigen::MatrixX2f viewBox(mViewBox.rows(), 2);
  viewBox.col(0) = mViewBox.col(0);
  viewBox.col(1) = mComponentsLevel1[iCompSet].col(iComp);

  for(const auto& distribution : mTMM) {
    Eigen::Vector2f mean = viewBox.transpose()*distribution.mean;
    Eigen::Matrix2f scale = viewBox.transpose()*distribution.scale*viewBox;
    Eigen::Matrix2f scaleInv = scale.inverse();
    //float height = distribution.weight*std::tgamma(0.5f*(distribution.df+2))
    //                     /(std::tgamma(0.5f*distribution.df)
    //                       *std::sqrt(std::pow(distribution.df*std::numbers::pi, 2)
    //                                  *scale.determinant()));
    float height = std::tgamma(1.0f)
                         /(std::betal(0.5f*distribution.df, 1.0f)
                           *std::sqrt(std::pow(distribution.df*std::numbers::pi, 2)
                                      *scale.determinant()));
    parameters.insert(parameters.end(),
                      {scaleInv(0, 0), scaleInv(0, 1), height, distribution.df,
                       scaleInv(1, 0), scaleInv(1, 1), mean.x(), mean.y()});
  }
  return parameters;
}

std::vector<float> Context::getParametersStd140Tmm3D(const int& iCompSet, const int& iComp) const {
  std::vector<float> parameters;
  parameters.reserve(16*mTMM.size());

  int showedCanonBasis = mComponentsLevel2.size() == (mTMM.size() + 2);
  Eigen::MatrixX3f viewBox(mViewBox.rows(), 3);
  viewBox.col(0) = mViewBox.col(0);
  viewBox.col(1) = mViewBox.col(1);
  viewBox.col(2) = mComponentsLevel2[iCompSet].col(iComp);

  for(const auto& distribution : mTMM) {
    Eigen::Vector3f mean = viewBox.transpose()*distribution.mean;
    Eigen::Matrix3f scale = viewBox.transpose()*distribution.scale*viewBox;
    Eigen::Matrix3f scaleInv = scale.inverse();
    //float height = distribution.weight*std::tgamma(0.5f*(distribution.df+3))
    //                     /(std::tgamma(0.5f*distribution.df)
    //                       *std::sqrt(std::pow(distribution.df*std::numbers::pi, 3)
    //                                  *scale.determinant()));
    float height = std::tgamma(1.5f)
                         /(std::betal(0.5f*distribution.df, 1.5f)
                           *std::sqrt(std::pow(distribution.df*std::numbers::pi, 3)
                                      *scale.determinant()));
    parameters.insert(parameters.end(), {scaleInv(0, 0), scaleInv(0, 1), scaleInv(0, 2), height,
                                         scaleInv(1, 0), scaleInv(1, 1), scaleInv(1, 2), distribution.df,
                                         scaleInv(2, 0), scaleInv(2, 1), scaleInv(2, 2), 0.0f,
                                         mean.x(), mean.y(), mean.z(), 0.0f});
  }
  return parameters;
}

std::vector<float> Context::getHightTmm() const {
  std::vector<float> hights(mTMM.size());
  for(int i = 0; i < hights.size(); ++i) hights[i] = mTMM[i].hight();
  return hights;
}

std::vector<float> Context::getScaleInvTmm() const {
  std::vector<float> scaleInvs;
  scaleInvs.reserve(9*mTMM.size());
  for(int i = 0; i < mTMM.size(); ++i) {
    Eigen::Matrix3f result = mViewBox.transpose()*mTMM[i].scaleInv*mViewBox;
    scaleInvs.insert(scaleInvs.end(), {result(0,0), result(0,1), result(0,2),
                                       result(1,0), result(1,1), result(1,2),
                                       result(2,0), result(2,1), result(2,2)});
  }
  return scaleInvs;
}

std::vector<float> Context::getCamPosTMinusMeanT_ScaleInv_camPosMinusMeanTmm(const glm::vec3& camPos) const {
  std::vector<float> result(mTMM.size());
  for(int i = 0; i < result.size(); ++i) {
    Eigen::Vector3f campos = {camPos.x, camPos.y, camPos.z};
    result[i] = ( (mViewBox*campos - mTMM[i].mean).transpose()
                 *mTMM[i].scaleInv
                 *(mViewBox*campos - mTMM[i].mean) )[0];
  }
  return result;
}

std::vector<float> Context::getScaleInv_camPosMinusMeanTmm(const glm::vec3& camPos) const {
  std::vector<float> result(3*mTMM.size());
  for(int i = 0; i < mTMM.size(); ++i) {
    Eigen::Vector3f campos = {camPos.x, camPos.y, camPos.z};
    Eigen::Vector3f tmp = mViewBox.transpose()*mTMM[i].scaleInv
                         *(mViewBox*campos - mTMM[i].mean);
    result[3*i+0] = tmp.x();
    result[3*i+1] = tmp.y();
    result[3*i+2] = tmp.z();
  }
  return result;
}
} // namespace context
