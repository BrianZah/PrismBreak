#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <vector>
#include <array>
#include <numbers>
#include <cmath>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <glm/glm.hpp>

#include "csv.hpp"
#include "context_TDistribution.hpp"
#include "context_Gaussian.hpp"

namespace context{class Bookmarks;}

class Context{
private:
  friend context::Bookmarks;
public:
  enum Model{nomm = 0, gmm = 1, tmm = 2};
  struct Token{
    Model model;
    int set;
    int comp;
    constexpr Token() : model(nomm), set(-1), comp(-1) {}
    constexpr Token(const Model& model_, const int& set_, const int& comp_) : model(model_), set(set_), comp(comp_) {}
    constexpr Token(const Token& other) : model(other.model), set(other.set), comp(other.comp) {}
    constexpr Token& operator=(const Token& other) {
      model = other.model;
      set = other.set;
      comp = other.comp;
      return *this;
    }
  };
private:
  using TDistribution = context::TDistribution;
  using Gaussian = context::Gaussian;

  std::string mDir;

  Eigen::MatrixX3f mViewBox;
  std::vector<TDistribution> mTMM;
  std::vector<Gaussian> mGMM;
  Eigen::MatrixXf mGlobalVariance;
  Eigen::MatrixXf mOrthoSpaceLevel0;
  Eigen::MatrixXf mOrthoSpaceLevel1;
  Eigen::MatrixXf mOrthoSpaceLevel2;
  std::vector<Eigen::MatrixXf> mComponentsLevel0;
  std::vector<Eigen::VectorXf> mWeightsLevel0;
  std::vector<Eigen::MatrixXf> mComponentsLevel1;
  std::vector<Eigen::VectorXf> mWeightsLevel1;
  std::vector<Eigen::MatrixXf> mComponentsLevel2;
  std::vector<Eigen::VectorXf> mWeightsLevel2;

  Eigen::Matrix<int, 2, 3> mISetsIComps;
  std::array<Token, 3> mState;

  Eigen::MatrixXf mData;
  Eigen::MatrixXf mUnscaledData;
  std::vector<std::string> mAttributeNames;
  std::vector<std::string> mPointNames;
  std::vector<int> mPointClusterTmm;
  std::vector<int> mPointClusterGmm;
  Eigen::VectorXf mUnsacaledMean;
  Eigen::VectorXf mUnsacaledStd;

  std::vector<Eigen::MatrixXf>& mComponents(const int& stage);
  std::vector<Eigen::VectorXf>& mWeights(const int& stage);
  Eigen::MatrixXf& mOrthoSpace(const int& stage);

  const std::vector<Eigen::MatrixXf>& mComponents(const int& stage) const;
  const std::vector<Eigen::VectorXf>& mWeights(const int& stage) const;
  const Eigen::MatrixXf& mOrthoSpace(const int& stage) const;

  Eigen::MatrixXf getEigenVectors(const Eigen::MatrixXf& selfAdjointMatrix) const;
  Eigen::MatrixXf getRightSingularVectors(const Eigen::MatrixXf& matrix) const;
  std::tuple<Eigen::VectorXf, Eigen::MatrixXf> getEigenValuesAndVectors(const Eigen::MatrixXf& selfAdjointMatrix) const;
  std::tuple<Eigen::VectorXf, Eigen::MatrixXf> getRightSingularValuesAndVectors(const Eigen::MatrixXf& matrix) const;

  std::vector<TDistribution> readTMixtureModel(const std::string& dir, const bool& asGaussian = false);
  std::vector<Gaussian> readGaussianMixtureModel(const std::string& dir);
  std::tuple<Eigen::MatrixXf, std::vector<std::string>, std::vector<std::string>>
    dataTable(const std::string& dir, const std::vector<int>& discardRows = {}, const std::vector<int>& discardCols = {});
  std::vector<int> classification(const std::string& dir);

public:
  Context(const std::string& dir);
  const std::string& getDir() const;

  int dimensions() const;
  float degreesOfFreedom(const int& index) const;
  std::vector<int> getIndexSetsAndComps(const int& currentStage) const;
  std::vector<int> getState(const int& currentStage) const;

  void prepare(const Model& model, const int& stage, const int& iDis, const int& iComp);
  void extendViewBox(const Model& model, const int& stage, const int& iCompSet, const int& iComp);
  void createOrthospace(const int& stage, const int& iCompSet, const int& iComp);
  void resizeComponentsAndWeights(const Model& model, const int& stage);
  void prepareCanonComponents(const int& stage);
  void preparePrincipalComponents(const int& stage);
  void prepareModelComponents(const Model& model, const int& stage);

  std::vector<float> getWeights(const Model& model) const;
  std::vector<float> getParametersStd140(const Model& model, const int& stage) const;
private:
  std::vector<float> getParametersStd140Tmm1D() const;
  std::vector<float> getParametersStd140Tmm2D() const;
  std::vector<float> getParametersStd140Tmm3D() const;
  std::vector<float> getParametersStd140TmmDetailView() const;
  std::vector<float> getParametersStd140Gmm1D() const;
  std::vector<float> getParametersStd140Gmm2D() const;
  std::vector<float> getParametersStd140Gmm3D() const;
  std::vector<float> getParametersStd140GmmDetailView() const;
public:
  std::array<std::vector<float>, 2> getParametersStd140(const int& stage, const int& iCompSet, const int& iComp) const;
private:
  std::vector<float> getParametersStd140Tmm1D(const int& iCompSet, const int& iComp) const;
  std::vector<float> getParametersStd140Gmm1D(const int& iCompSet, const int& iComp) const;
  std::vector<float> getParametersStd140Tmm2D(const int& iCompSet, const int& iComp) const;
  std::vector<float> getParametersStd140Gmm2D(const int& iCompSet, const int& iComp) const;
  std::vector<float> getParametersStd140Tmm3D(const int& iCompSet, const int& iComp) const;
  std::vector<float> getParametersStd140Gmm3D(const int& iCompSet, const int& iComp) const;
public:
  std::vector<float> getHight(const Model& model) const;
  std::vector<float> getScaleInv(const Model& model) const;
  std::vector<float> getCamPosTMinusMeanT_ScaleInv_camPosMinusMean(const Model& model, const glm::vec3& camPos) const;
  std::vector<float> getScaleInv_camPosMinusMean(const Model& model, const glm::vec3& camPos) const;
private:
  std::vector<float> getHightTmm() const;
  std::vector<float> getHightGmm() const;
  std::vector<float> getScaleInvTmm() const;
  std::vector<float> getScaleInvGmm() const;
  std::vector<float> getCamPosTMinusMeanT_ScaleInv_camPosMinusMeanTmm(const glm::vec3& camPos) const;
  std::vector<float> getCamPosTMinusMeanT_ScaleInv_camPosMinusMeanGmm(const glm::vec3& camPos) const;
  std::vector<float> getScaleInv_camPosMinusMeanTmm(const glm::vec3& camPos) const;
  std::vector<float> getScaleInv_camPosMinusMeanGmm(const glm::vec3& camPos) const;
public:
  const std::vector<std::string>& getAttributes() const;
  const std::vector<std::string>& getPointNames() const;
  const std::vector<float> getUnscaledPointValues(const int& index) const;
  const std::vector<float> getPointValues(const int& index) const;
  float getPointStandardDerivation(const Model& model, const int& index) const;
  const std::vector<float> getUnscaledMean(const Model& model, const int& iDis) const;
  const std::vector<float> getMean(const Model& model, const int& iDis) const;

  std::vector<float> getScaledEigenValues(const Model& model, const int& stage) const;
  std::vector<float> getComponents(const int& stage) const;
  const float getComponentValue(const int& stage, const int& set, const int& comp, const int& index) const;
  std::vector<float> getSparsity(const int& stage) const;

  int numDistributions(const Model& model) const;
  int numComponents(const int& stage) const;
  int numComponentsSets(const int& stage) const;
  int showCanonicalBasis(const Model& model, const int& stage) const;
  int showCanonicalBasis(const int& stage) const;

  std::vector<float> dataAs3DPoints(const Model& model, const int& offset = 0) const;
  std::vector<float> getPointProbabilities(const Model& model) const;
  const std::vector<int>& assignedDistribution() const;
  int numPoints() const;
public:
  float prefactor(const int& index) const {
    return mTMM[index].hight();
  }

  float camPosTMinusMeanT_ScaleInv_camPosMinusMean(const glm::vec3& camPos, const int& index) const {
    Eigen::Vector3f campos = {camPos.x, camPos.y, camPos.z};
    return ( (mViewBox*campos - mTMM[index].mean).transpose()
             *mTMM[index].scaleInv
             *(mViewBox*campos - mTMM[index].mean)
           )[0];
  }

  glm::vec3 viewBoxTScaleInv_meanMinusCamPos(const glm::vec3& camPos, const int& index) const {
    Eigen::Vector3f campos = {camPos.x, camPos.y, camPos.z};
    //std::cout << "mViewBox = " << mViewBox << std::endl;
    Eigen::Vector3f result = mViewBox.transpose()*mTMM[index].scaleInv
                            *(mTMM[index].mean - mViewBox*campos);
    return glm::vec3(result.x(), result.y(), result.z());
  }

  glm::mat3 viewBoxTScaleInvViewBox(const int& index) const {
    Eigen::Matrix3f result = mViewBox.transpose()*mTMM[index].scaleInv*mViewBox;
    return glm::mat3(result(0,0), result(0,1), result(0,2),
                     result(1,0), result(1,1), result(1,2),
                     result(2,0), result(2,1), result(2,2));
  }
};
#endif // CONTEXT_HPP
