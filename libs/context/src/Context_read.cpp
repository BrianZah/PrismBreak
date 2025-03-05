#include "Context.hpp"

#include <vector>

#include <Eigen/Core>
#include <Eigen/Dense>

#include "csv.hpp"
#include "context_TDistribution.hpp"
#include "context_Gaussian.hpp"

std::vector<context::TDistribution> Context::readTMixtureModel(const std::string& dir, const bool& asGaussian) {
  auto weight = csv::toArray<std::vector<float>>(dir + "/pig.csv", ",");
  auto df = asGaussian ? std::vector<float>(weight.size(), 1000.0f) :
                         csv::toArray<std::vector<float>>(dir + "/df.csv", ",");

  auto dataArray = csv::toArray<std::vector<float>>(dir + "/mean.csv", ",");
  int dimensions = dataArray.size()/weight.size();
  auto mean = Eigen::Map<Eigen::MatrixXf>(dataArray.data(), dimensions, weight.size());

  auto dataArray2 = csv::toArray<std::vector<float>>(dir + "/sigma.csv", ",");
  auto sigma = Eigen::Map<Eigen::MatrixXf>(dataArray2.data(), dimensions, dimensions*weight.size());


  std::vector<TDistribution> distributions;
  for(int i = 0; i < weight.size(); ++i)
    distributions.push_back(TDistribution(
      df[i], mean.col(i), sigma.block(0, i*dimensions, dimensions, dimensions), weight[i]
    ));
  for(int i = 0; i < weight.size(); ++i) {
    std::cout << "df[" << i << "] = " << df[i]<< std::endl;
    std::cout << "mean[" << i << "] = " << mean.col(i).transpose() << std::endl;
    std::cout << "sigma[" << i << "] =\n" << sigma.block(0, i*dimensions, dimensions, dimensions)<< std::endl;
    std::cout << "sigma[" << i << "].determinant() = " << sigma.block(0, i*dimensions, dimensions, dimensions).determinant() << std::endl;
    std::cout << "weight[" << i << "] = " << weight[i] << std::endl;
  }

  return distributions;
}

std::vector<context::Gaussian> Context::readGaussianMixtureModel(const std::string& dir) {
  auto weight = csv::toArray<std::vector<float>>(dir + "/pig.csv", ",");

  auto dataArray = csv::toArray<std::vector<float>>(dir + "/mean.csv", ",");
  int dimensions = dataArray.size()/weight.size();
  auto mean = Eigen::Map<Eigen::MatrixXf>(dataArray.data(), dimensions, weight.size());

  auto dataArray2 = csv::toArray<std::vector<float>>(dir + "/sigma.csv", ",");
  auto sigma = Eigen::Map<Eigen::MatrixXf>(dataArray2.data(), dimensions, dimensions*weight.size());

  std::vector<Gaussian> distributions;
  for(int i = 0; i < weight.size(); ++i)
    distributions.push_back(Gaussian(
      mean.col(i), sigma.block(0, i*dimensions, dimensions, dimensions), weight[i]
    ));
  for(int i = 0; i < weight.size(); ++i) {
    std::cout << "mean[" << i << "] = " << mean.col(i).transpose() << std::endl;
    std::cout << "sigma[" << i << "] =\n" << sigma.block(0, i*dimensions, dimensions, dimensions)<< std::endl;
    std::cout << "sigma[" << i << "].determinant() = " << sigma.block(0, i*dimensions, dimensions, dimensions).determinant() << std::endl;
    std::cout << "weight[" << i << "] = " << weight[i] << std::endl;
  }

  return distributions;
}

std::vector<int> Context::classification(const std::string& dir) {
  return csv::toArray<std::vector<int>>(dir + "/classification.csv", ",");
}

std::tuple<Eigen::MatrixXf, std::vector<std::string>, std::vector<std::string>> Context::dataTable(const std::string& filename, const std::vector<int>& discardRows, const std::vector<int>& discardCols) {
  std::cout << "[Info](Context::dataTable) test0" << std::endl;
  auto [dataArray, dimensions, colNames, rowNames] = csv::getTable<Eigen::VectorXf>(filename, ",", true, true, discardRows, discardCols);
  std::cout << "[Info](Context::dataTable) test1" << std::endl;
  return {dataArray.reshaped(dimensions, dataArray.size()/dimensions), colNames, rowNames};
}
