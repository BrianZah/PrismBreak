#ifndef CONTEXT_BOOKMARKS_HPP
#define CONTEXT_BOOKMARKS_HPP

#include <vector>
#include <array>
#include <tuple>
#include <memory>

#include <Eigen/Dense>
#include <glm/glm.hpp>

#include "context/Context.hpp"

namespace context{
class Bookmarks{
private:
  std::shared_ptr<Context> mContext;

  std::vector<Context::Token> mTokensStage0;
  mutable std::vector<float> mParametersTmm1D;
  mutable std::vector<float> mParametersGmm1D;

  //std::vector<Eigen::Matrix<int, 2, 2>> mISetsICompsStage1;
  std::vector<Context::Token> mTokensStage1;
  mutable std::vector<float> mParametersTmm2D;
  mutable std::vector<float> mParametersGmm2D;

  std::vector<Eigen::Matrix<int, 2, 3>> mISetsICompsStage2;
  std::vector<Context::Token> mTokensStage2;
  mutable std::vector<float> mParametersTmm3D;
  mutable std::vector<float> mParametersGmm3D;

  std::vector<int> mIndexArray;
public:
  Bookmarks();
  void init(const std::shared_ptr<Context>& context, const int& reservedMemory = 5);
  std::vector<float> getParametersStd140(const Context::Model& model, const int& stage) const;
  std::vector<int> getIndexSetsAndComps(const int& stage) const;
  std::vector<int> getTokens(const int& stage) const;
  void prepareContext(const Context::Model& model, const int& stage, const int& index) const;
  std::array<int, 3> getIndexSetAndCompAndOffset(const int& stage, const int& index) const;
  void add(const int& stage, const Context::Model& model, const int& iCompSet, const int& iComp);
  std::vector<int> getIndices(const int& stage, const Context::Model& model);
  void remove(const int& stage, const Context::Model& model, const int& iCompSet, const int& iComp);
  void remove(const int& stage, const int& index);
  int size(const int& stage) const;
};
} // namespace context

#endif // CONTEXT_BOOKMARKS_HPP
