#include "Context.hpp"

#include <vector>
#include <numbers>
#include <cmath>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <glm/glm.hpp>

#include "csv.hpp"
#include "context_TDistribution.hpp"

std::vector<Eigen::MatrixXf>& Context::mComponents(const int& stage) {
  switch (stage) {
    case 0: {return mComponentsLevel0;}
    case 1: {return mComponentsLevel1;}
    case 2: {return mComponentsLevel2;}
    default: {return mComponentsLevel0;}
  }
}
std::vector<Eigen::VectorXf>& Context::mWeights(const int& stage) {
  switch (stage) {
    case 0: {return mWeightsLevel0;}
    case 1: {return mWeightsLevel1;}
    case 2: {return mWeightsLevel2;}
    default: {return mWeightsLevel0;}
  }
}
Eigen::MatrixXf& Context::mOrthoSpace(const int& stage) {
  switch (stage) {
    case 0: {return mOrthoSpaceLevel0;}
    case 1: {return mOrthoSpaceLevel1;}
    case 2: {return mOrthoSpaceLevel2;}
    default: {return mOrthoSpaceLevel1;}
  }
}

const std::vector<Eigen::MatrixXf>& Context::mComponents(const int& stage) const {
  switch (stage) {
    case 0: {return mComponentsLevel0;}
    case 1: {return mComponentsLevel1;}
    case 2: {return mComponentsLevel2;}
    default: {return mComponentsLevel0;}
  }
}
const std::vector<Eigen::VectorXf>& Context::mWeights(const int& stage) const {
  switch (stage) {
    case 0: {return mWeightsLevel0;}
    case 1: {return mWeightsLevel1;}
    case 2: {return mWeightsLevel2;}
    default: {return mWeightsLevel0;}
  }
}
const Eigen::MatrixXf& Context::mOrthoSpace(const int& stage) const {
  switch (stage) {
    case 0: {return mOrthoSpaceLevel0;}
    case 1: {return mOrthoSpaceLevel1;}
    case 2: {return mOrthoSpaceLevel2;}
    default: {return mOrthoSpaceLevel1;}
  }
}

Eigen::MatrixXf Context::getEigenVectors(const Eigen::MatrixXf& selfAdjointMatrix) const {
  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> es(selfAdjointMatrix);
  std::cout << "[Info](Context::getComponents) eigenvalues =\n" << es.eigenvalues().reverse() << std::endl;
  return es.eigenvectors().rowwise().reverse();
}
Eigen::MatrixXf Context::getRightSingularVectors(const Eigen::MatrixXf& matrix) const {
  Eigen::JacobiSVD<Eigen::MatrixXf> svd(matrix, Eigen::ComputeThinV);
  std::cout << "[Info](Context::getComponents) singularValues =\n" << svd.singularValues() << std::endl;
  return svd.matrixV();
}

std::tuple<Eigen::VectorXf, Eigen::MatrixXf> Context::getEigenValuesAndVectors(const Eigen::MatrixXf& selfAdjointMatrix) const {
  std::cout << "[Info](Context::getComponents) selfAdjointMatrix =\n" << selfAdjointMatrix << std::endl;
  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> es(selfAdjointMatrix);
  std::cout << "[Info](Context::getComponents) eigenvalues =\n" << es.eigenvalues().reverse() << std::endl;
  return {es.eigenvalues().reverse(), es.eigenvectors().rowwise().reverse()};
}
std::tuple<Eigen::VectorXf, Eigen::MatrixXf> Context::getRightSingularValuesAndVectors(const Eigen::MatrixXf& matrix) const {
  Eigen::JacobiSVD<Eigen::MatrixXf> svd(matrix, Eigen::ComputeThinV);
  std::cout << "[Info](Context::getComponents) singularValues =\n" << svd.singularValues() << std::endl;
  return {svd.singularValues(), svd.matrixV()};
}
