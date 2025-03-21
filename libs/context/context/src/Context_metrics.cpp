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
std::vector<float> Context::getScaledEigenValues(const Model& model, const int& stage) const {
  std::vector<float> nsv;
  nsv.reserve(mComponents(stage).size()*mWeights(stage)[0].rows());
  if(1 == showCanonicalBasis(stage)) {
    Eigen::VectorXf scaledWeights = mGlobalVariance.diagonal();
    float max = scaledWeights.maxCoeff();
    scaledWeights = scaledWeights/max;

    std::array<int, 2> iToAvoid = {-1, -1};
    if(stage > 0) iToAvoid[0] = mState[0].comp;
    if(stage > 1) iToAvoid[1] = mState[1].comp >= mState[0].comp ? mState[1].comp+1 : mState[1].comp;
    //if(stage > 0) iToAvoid[0] = mISetsIComps(1,0);
    //if(stage > 1) iToAvoid[1] = mISetsIComps(1,1) >= mISetsIComps(1,0) ? mISetsIComps(1,1)+1 : mISetsIComps(1,1);
    for(int i = 0; i < scaledWeights.rows(); ++i)
      if(iToAvoid[0] != i && iToAvoid[1] != i) {
        nsv.push_back(scaledWeights(i));
      }
  }
  for(const auto& weights : mWeights(stage)) {
    Eigen::VectorXf scaledWeights = weights/weights[0];
    nsv.insert(nsv.end(), scaledWeights.begin(), scaledWeights.end());
  }
  return nsv;
}

std::vector<float> Context::getSparsity(const int& stage) const {
  std::vector<float> sparsity;
  sparsity.reserve(mComponents(stage).size() * mComponents(stage)[0].size());
  for(const auto& comp : mComponents(stage)) {
    for(const auto& col : comp.colwise()) {
      sparsity.push_back((col.array()*col.array()*col.array()*col.array()).sum());
    }
  }
  //std::cout << "[Info](Context::getSparsity) Array =\n";
  //int i = 0;
  //for(const auto& value : sparsity) std::cout << value << (++i%mComponents(stage)[0].cols() == 0 ? "\n" : ", ");
  return sparsity;
}

std::vector<float> Context::getComponents(const int& stage) const {
  std::vector<float> components;
  components.reserve(mComponents(stage).size() * mComponents(stage)[0].size());
  for(const auto& comp : mComponents(stage)) {
    for(const auto& col : comp.colwise()) {
      components.insert(components.end(), col.begin(), col.end());
    }
  }
  //std::cout << "[Info](Context::getComponents) Array =\n";
  //int i = 0;
  //for(const auto& comp : components) std::cout << comp << (++i%9 == 0 ? "\n" : ", ");
  return components;
}
} // namespace context
