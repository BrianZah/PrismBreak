#ifndef CONTEXT_GAUSSIAN_HPP
#define CONTEXT_GAUSSIAN_HPP

#include <numbers>
#include <cmath>

#include <Eigen/Core>
#include <Eigen/Dense>

namespace context{
class Gaussian{
public:
  Eigen::VectorXf mean;
  Eigen::MatrixXf scale;
  float scaleDet;
  Eigen::MatrixXf scaleInv;
  float weight = 1.0f;

  inline Gaussian(const Eigen::VectorXf& mean_, const Eigen::MatrixXf& scale_, const float& weight_ = 1.0f)
  : mean(mean_), scale(scale_), scaleDet(scale_.determinant()), scaleInv(scale_.inverse()), weight(weight_) {}

  inline int dimensions() const {return mean.size();}

  inline float hight() const {
    return weight / std::sqrt(std::pow(2.0f*std::numbers::pi, dimensions())*scaleDet);
  }

  inline float mahalanobisSquared(const Eigen::VectorXf& x) const {
    return ((x-mean).transpose()*scaleInv*(x-mean))(0, 0);
  }

  inline float pdf(const Eigen::VectorXf& x) const {
    return hight()*std::exp(-0.5f*mahalanobisSquared(x));
  }
};
}

#endif // CONTEXT_GAUSSIAN_HPP
