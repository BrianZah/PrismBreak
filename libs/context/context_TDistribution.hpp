#ifndef TDISTRIBUTION_HPP
#define TDISTRIBUTION_HPP

#include <numbers>
#include <cmath>

#include <Eigen/Core>
#include <Eigen/Dense>

namespace context{
class TDistribution{
public:
  float df;
  Eigen::VectorXf mean;
  Eigen::MatrixXf scale;
  float scaleDet;
  Eigen::MatrixXf scaleInv;
  float weight = 1.0f;

  inline TDistribution(const float& df_, const Eigen::VectorXf& mean_, const Eigen::MatrixXf& scale_, const float& weight_ = 1.0f)
  : df(df_), mean(mean_), scale(scale_), scaleDet(scale_.determinant()), scaleInv(scale_.inverse()), weight(weight_) {}

  inline int dimensions() const {return mean.size();}

  inline float hight() const {
    return weight*std::tgamma(0.5f*dimensions())
           / ( std::betal(0.5f*df, 0.5f*dimensions())
               *std::sqrt(std::pow(df*std::numbers::pi, dimensions())*scaleDet)
             );
  }

  inline float mahalanobisSquared(const Eigen::VectorXf& x) const {
    return ((x-mean).transpose()*scaleInv*(x-mean))(0, 0);
  }

  inline float pdf(const Eigen::VectorXf& x) const {
    return hight()*std::pow(1.0f + mahalanobisSquared(x)/df, -0.5f*(df+dimensions()));
  }
};
}

#endif // TDISTRIBUTION_HPP
