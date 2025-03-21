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
Context::Context(const std::string& dir)
: mDir(dir), mCreatedRecently(true),
  mPointClusterTmm(classification(dir + "/tmm")),
  mPointClusterGmm(classification(dir + "/gmm")),
  mTMM(readTMixtureModel(dir + "/tmm")),
  mGMM(readGaussianMixtureModel(dir + "/gmm"))
{
  try{
    std::tie(mData, mAttributeNames, mPointNames) = dataTable(dir + "/tmm/scaled.csv");
  } catch(std::string err) {
    std::cout << err << std::endl;
  }
  //std::cout << "[Info](Context::Context) data =\n" << mData.transpose() << std::endl;
  mGlobalVariance = Eigen::MatrixXf((mData*mData.transpose())/mData.cols());
  //std::cout << "[Info](Context::Context) mGlobalVariance =\n" << mGlobalVariance << std::endl;
  try{
    mUnscaledData = std::get<0>(dataTable(dir + "/data.csv"));
  } catch(std::string err) {
    std::cout << err << std::endl;
  }
  //Eigen::JacobiSVD<Eigen::MatrixXf> svd2(mData.transpose(), Eigen::ComputeThinV);
  //std::cout << "[Info](Context::Context) svd2.matrixV() =\n" << svd2.matrixV() << std::endl;
  //std::cout << "[Info](Context::Context) svd2.singularValues() =\n" << svd2.singularValues() << std::endl;
// <- singularvectors vs. eigenvectors ->
  mOrthoSpaceLevel0 = Eigen::MatrixXf::Identity(mTMM[0].dimensions(), mTMM[0].dimensions());
  mComponentsLevel0.reserve(std::max(mTMM.size(), mGMM.size()) + 2);
  mWeightsLevel0.reserve(std::max(mTMM.size(), mGMM.size()) + 2);

  mComponentsLevel1.reserve(std::max(mTMM.size(), mGMM.size()) + 2);
  mWeightsLevel1.reserve(std::max(mTMM.size(), mGMM.size()) + 2);
  mOrthoSpaceLevel2 = Eigen::MatrixXf(mTMM[0].dimensions(), mTMM[0].dimensions()-2);
  mComponentsLevel2.reserve(std::max(mTMM.size(), mGMM.size()) + 2);
  mWeightsLevel2.reserve(std::max(mTMM.size(), mGMM.size()) + 2);
  mViewBox = Eigen::MatrixX3f(mTMM[0].dimensions(), 3);

  mUnsacaledMean = mUnscaledData.rowwise().mean();
  //std::cout << "[Info](Context::Context) mUnsacaledMean = " << mUnsacaledMean << std::endl;
  mUnsacaledStd = (mUnscaledData.array().square().rowwise().mean() - mUnsacaledMean.array().square()).sqrt().matrix();
  //std::cout << "[Info](Context::Context) mUnsacaledStd = " << mUnsacaledStd << std::endl;
}

const std::string& Context::getDir() const {return mDir;}
bool Context::getCreatedRecently() const {return mCreatedRecently;}
void Context::setCreatedRecently(const bool& createdRecently) {mCreatedRecently = createdRecently;}

int Context::showCanonicalBasis(const int& stage) const {
  switch(stage) {
    case 0 : return 1;
    case 1 : return mState[0].set == 0;
    case 2 : return mState[0].set == 0 && mState[1].set == 0;
    default: return 0;
  }
}

int Context::dimensions() const {return mTMM[0].dimensions();}

std::vector<int> Context::getState(const int& currentStage) const {
  std::vector<int> state;
  state.reserve(currentStage*3);
  for(int stage = 0; stage < currentStage; ++stage)
    state.insert(state.end(), {int(mState[stage].model), mState[stage].set, mState[stage].comp});
  return state;
}

void Context::prepare(const Model& model, const int& stage, const int& iCompSet, const int& iComp) {
  extendViewBox(model, stage, iCompSet, iComp);
  if(3 == stage) return;
  createOrthospace(stage, iCompSet, iComp);
  //std::cout << "[Info](Context::prepare) mOrthoSpace(stage) = \n" << mOrthoSpace(stage) << std::endl;
  resizeComponentsAndWeights(model, stage);
  prepareCanonComponents(stage);
  preparePrincipalComponents(stage);
  prepareModelComponents(model, stage);
}

void Context::extendViewBox(const Model& model, const int& stage, const int& iCompSet, const int& iComp) {
  if(0 == stage) return;
  int showedCanonBasis = showCanonicalBasis(stage-1);
  mState[stage-1] = Token(iCompSet <= showedCanonBasis ? Context::nomm : model, iCompSet, iComp);
  mViewBox.col(stage-1) = mComponents(stage-1)[iCompSet].col(iComp);
}

void Context::createOrthospace(const int& stage, const int& iCompSet, const int& iComp) {
  if(0 == stage) {
    mOrthoSpace(stage) = Eigen::MatrixXf::Identity(mTMM[0].dimensions(), mTMM[0].dimensions());
    return;
  }
  mOrthoSpace(stage) = Eigen::MatrixXf(mTMM[0].dimensions(), mTMM[0].dimensions()-stage);
  for(int i = 0, j = 0; i < mComponents(stage-1)[iCompSet].cols(); ++i) {
    if(i != iComp) {
      mOrthoSpace(stage).col(j) = mComponents(stage-1)[iCompSet].col(i);
      ++j;
    }
  }
}

void Context::resizeComponentsAndWeights(const Model& model, const int& stage) {
  int showCanonBasis = showCanonicalBasis(stage);
  mComponents(stage).resize(showCanonBasis + 1 + (tmm == model ? mTMM.size() : mGMM.size()));
  mWeights(stage).resize(1 + (tmm == model ? mTMM.size() : mGMM.size()));
}

void Context::prepareCanonComponents(const int& stage) {
  if(1 == showCanonicalBasis(stage)) mComponents(stage)[0] = mOrthoSpace(stage);
}

void Context::preparePrincipalComponents(const int& stage) {
  auto [values, vectors] = getEigenValuesAndVectors(mOrthoSpace(stage).transpose()*mGlobalVariance*mOrthoSpace(stage));
  mComponents(stage)[showCanonicalBasis(stage)] = mOrthoSpace(stage)*vectors;
  mWeights(stage)[0] = values;
}

void Context::prepareModelComponents(const Model& model, const int& stage) {
  int i = showCanonicalBasis(stage) + 1;
  int j = 1;
  if(tmm == model) {
    for(const auto& distribution : mTMM) {
      auto [values, vectors] = getEigenValuesAndVectors(mOrthoSpace(stage).transpose()*distribution.scale*mOrthoSpace(stage));
      mComponents(stage)[i] = mOrthoSpace(stage)*vectors;
      mWeights(stage)[j] = values;
      ++i; ++j;
    }
  } else {
    for(const auto& distribution : mGMM) {
      auto [values, vectors] = getEigenValuesAndVectors(mOrthoSpace(stage).transpose()*distribution.scale*mOrthoSpace(stage));
      mComponents(stage)[i] = mOrthoSpace(stage)*vectors;
      mWeights(stage)[j] = values;
      ++i; ++j;
    }
  }
}

std::array<std::vector<float>, 2> Context::getParametersStd140(const int& stage, const int& iCompSet, const int& iComp) const {
  switch(stage) {
    case 0: return {getParametersStd140Tmm1D(iCompSet, iComp), getParametersStd140Gmm1D(iCompSet, iComp)};
    case 1: return {getParametersStd140Tmm2D(iCompSet, iComp), getParametersStd140Gmm2D(iCompSet, iComp)};
    case 2: return {getParametersStd140Tmm3D(iCompSet, iComp), getParametersStd140Gmm3D(iCompSet, iComp)};
  }
  std::cerr << "[Error](Context::getParametersStd140) Invalid stage" << std::endl;
}

std::vector<float> Context::getWeights(const Model& model) const {
  std::vector<float> weights;
  if(tmm == model) {
    weights.resize(mTMM.size());
    for(int i = 0; i < mTMM.size(); ++i) weights[i] = mTMM[i].weight;
  } else {
    weights.resize(mGMM.size());
    for(int i = 0; i < mGMM.size(); ++i) weights[i] = mGMM[i].weight;
  }
  return weights;
}

std::vector<float> Context::getParametersStd140(const Model& model, const int& stage) const {
  switch(stage) {
    case 0: return tmm == model ? getParametersStd140Tmm1D() : getParametersStd140Gmm1D();
    case 1: return tmm == model ? getParametersStd140Tmm2D() : getParametersStd140Gmm2D();
    case 2: return tmm == model ? getParametersStd140Tmm3D() : getParametersStd140Gmm3D();
    case 3: return tmm == model ? getParametersStd140TmmDetailView() : getParametersStd140GmmDetailView();
  }
  std::cerr << "[Error](Context::getParametersStd140) Invalid stage" << std::endl;
}

std::vector<float> Context::getHight(const Model& model) const {
  return tmm == model ? getHightTmm() : getHightGmm();
}
std::vector<float> Context::getScaleInv(const Model& model) const {
  return tmm == model ? getScaleInvTmm() : getScaleInvGmm();
}
std::vector<float> Context::getCamPosTMinusMeanT_ScaleInv_camPosMinusMean(const Model& model, const glm::vec3& camPos) const {
  return tmm == model ? getCamPosTMinusMeanT_ScaleInv_camPosMinusMeanTmm(camPos) : getCamPosTMinusMeanT_ScaleInv_camPosMinusMeanGmm(camPos);
}
std::vector<float> Context::getScaleInv_camPosMinusMean(const Model& model, const glm::vec3& camPos) const {
  return tmm == model ? getScaleInv_camPosMinusMeanTmm(camPos) : getScaleInv_camPosMinusMeanGmm(camPos);
}

const std::vector<std::string>& Context::getAttributes() const {return mAttributeNames;}
const std::vector<std::string>& Context::getPointNames() const {return mPointNames;}
const std::vector<float> Context::getUnscaledPointValues(const int& index) const {
  std::vector<float> values(mUnscaledData.rows());
  for(int i = 0; i < values.size(); ++i) values[i] = mUnscaledData(i, index);
  return values;
}
const std::vector<float> Context::getPointValues(const int& index) const {
  std::vector<float> values(mData.rows());
  for(int i = 0; i < values.size(); ++i) values[i] = mData(i, index);
  return values;
}
float Context::getPointStandardDerivation(const Model& model, const int& index) const {
  if(model == tmm) {
    auto& dis = mTMM[mPointClusterTmm[index]-1];
    float distance = dis.mahalanobisSquared(mData.col(index));
    return std::sqrt(distance);
  } else {
    auto& dis = mGMM[mPointClusterGmm[index]-1];
    float distance = dis.mahalanobisSquared(mData.col(index));
    return std::sqrt(distance);
  }
}
const std::vector<float> Context::getUnscaledMean(const Model& model, const int& iDis) const {
  if(tmm == model) {
    std::vector<float> mean(mTMM[iDis].dimensions());
    for(int i = 0; i < mTMM[iDis].dimensions(); ++i)
      mean[i] = mTMM[iDis].mean[i]*mUnsacaledStd[i] + mUnsacaledMean[i];
    return mean;
  }
  std::vector<float> mean(mGMM[iDis].dimensions());
  for(int i = 0; i < mGMM[iDis].dimensions(); ++i)
    mean[i] = mGMM[iDis].mean[i]*mUnsacaledStd[i] + mUnsacaledMean[i];
  return mean;
}
const std::vector<float> Context::getMean(const Model& model, const int& iDis) const {
  if(tmm == model) {
    std::vector<float> mean(mTMM[iDis].dimensions());
    for(int i = 0; i < mTMM[iDis].dimensions(); ++i) mean[i] = mTMM[iDis].mean[i];
    return mean;
  }
  std::vector<float> mean(mGMM[iDis].dimensions());
  for(int i = 0; i < mGMM[iDis].dimensions(); ++i) mean[i] = mGMM[iDis].mean[i];
  return mean;
}

int Context::numDistributions(const Model& model) const {return tmm == model ? mTMM.size() : mGMM.size();}
int Context::numComponents(const int& stage) const {return dimensions()-stage;}
int Context::numComponentsSets(const int& stage) const {return mComponents(stage).size();}
const float Context::getComponentValue(const int& stage, const int& set, const int& comp, const int& index) const {
  return mComponents(stage)[set](index, comp);
}
//int Context::numDistributionsOverAll(const int& stage) const {return mComponents(stage).size()*(dimensions()-stage)*mTMM.size();}
int Context::numPoints() const {return mData.cols();}

std::vector<float> Context::dataAs3DPoints(const Model& model, const int& offset) const {
  std::vector<float> array(mData.cols() * (3+2+offset));
  //std::vector<Eigen::Vector3f> means;
  //means.reserve(mTMM.size());
  //std::vector<Eigen::Matrix3f> scaleInvs;
  //scaleInvs.reserve(mTMM.size());
  //for(const auto& distribution : mTMM) {
  //  means.emplace_back(mViewBox.transpose()*distribution.mean);
  //  Eigen::Matrix3f scale = mViewBox.transpose()*distribution.scale*mViewBox;
  //  scaleInvs.emplace_back(scale.inverse());
  //}
  for(int i = 0; i < mData.cols(); ++i) {
    Eigen::Vector3f point3D = mViewBox.transpose()*mData.col(i);
    array[i*(3+2+offset) + 0] = point3D.x();
    array[i*(3+2+offset) + 1] = point3D.y();
    array[i*(3+2+offset) + 2] = point3D.z();
    array[i*(3+2+offset) + 3] = model == tmm ? mPointClusterTmm[i]-1 : mPointClusterGmm[i]-1;
    //array[i*(3+1+2+offset) + 4] = i;
    if(model == tmm) {
      auto& dis = mTMM[mPointClusterTmm[i]-1];
      float distance = dis.mahalanobisSquared(mData.col(i));
      array[i*(3+2+offset) + 4] = std::sqrt(distance);
    } else {
      auto& dis = mGMM[mPointClusterGmm[i]-1];
      float distance = ((dis.mean-mData.col(i)).transpose() * dis.scaleInv * (dis.mean-mData.col(i)))(0, 0);
      array[i*(3+2+offset) + 4] = std::sqrt(distance);
    }
    //Eigen::Vector3f& mean = means[mTPointClass[i]-1];
    //Eigen::Matrix3f& scaleInv = scaleInvs[mTPointClass[i]-1];
    //distance = ((mean-point3D).transpose() * scaleInv * (mean-point3D))(0, 0);
    //array[i*(3+1+2+offset) + 5] = std::sqrt(distance);
  }
  //std::cout << "[Info] (Context::dataAs3DPoints) 3D Points =" << std::endl;
  //for(int i = 0; i < array.size(); ++i) std::cout << array[i] << ((i+1)%6 != 0 ? ", " : "\n");
  return array;
}

std::vector<float> Context::getPointProbabilities(const Model& model) const {
  std::vector<float> probabilities;
  if(model == tmm) {
    probabilities.resize(mData.cols()*mTMM.size());
    for(int i = 0; i < mData.cols(); ++i) {
      float sum = 0.0f;
      for(int j = 0; j < mTMM.size(); ++j) {
        probabilities[i*mTMM.size() + j] = mTMM[j].pdf(mData.col(i));
        sum+= probabilities[i*mTMM.size() + j];
      }
      //std::cout << "[Info] (Context::getPointProbabilities) probabilities[" << i << "] =";
      for(int j = 0; j < mTMM.size(); ++j) {
        probabilities[i*mTMM.size() + j]/= sum;
        //std::cout << " " << probabilities[i*mTMM.size() + j];
      }
      //std::cout << std::endl;
    }
  } else {
    probabilities.resize(mData.cols()*mGMM.size());
    for(int i = 0; i < mData.cols(); ++i) {
      float sum = 0.0f;
      for(int j = 0; j < mGMM.size(); ++j) {
        probabilities[i*mGMM.size() + j] = mGMM[j].pdf(mData.col(i));
        sum+= probabilities[i*mGMM.size() + j];
      }
      //std::cout << "[Info] (Context::getPointProbabilities) probabilities[" << i << "] =";
      for(int j = 0; j < mGMM.size(); ++j) {
        probabilities[i*mGMM.size() + j]/= sum;
        //std::cout << " " << probabilities[i*mGMM.size() + j];
      }
      //std::cout << std::endl;
    }
  }
  return probabilities;
}

//const std::vector<int>& Context::assignedDistribution() const {
//  return mTPointClass;
//}
} // namespace context
