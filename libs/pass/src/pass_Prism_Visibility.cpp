#include <vector>

#include "gl_Shader.hpp"
#include "pass_Prism.hpp"

namespace pass{
  std::vector<float> Prism::Visibility::get() const {

    //uploadData(shader);
    //Eigen::VectorXf evalData = getEvalData(shader);
    //std::vector<float> sumSoftMax = getSumSoftMax(evalData);
    std::vector<float> sumSoftMax;
    return sumSoftMax;
  }
} // namespace pass
