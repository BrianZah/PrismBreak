#include "context_Bookmarks.hpp"

#include <vector>
#include <array>
#include <tuple>
#include <iostream>

#include <glm/glm.hpp>

#include "context_TDistribution.hpp"
#include "Context.hpp"

namespace context{
  Bookmarks::Bookmarks()
  : mContext(nullptr),
    mParametersTmm1D(), mParametersGmm1D(),
    mParametersTmm2D(), mParametersGmm2D(),
    mParametersTmm3D(), mParametersGmm3D()
  {}

  void Bookmarks::init(const std::shared_ptr<Context>& context, const int& reservedMemory) {
    mContext = context;

    mTokensStage0.reserve(reservedMemory);
    mParametersTmm1D.reserve(reservedMemory*mContext->numDistributions(Context::tmm)*4);
    mParametersGmm1D.reserve(reservedMemory*mContext->numDistributions(Context::gmm)*4);

    mTokensStage0.reserve(reservedMemory*2);
    mParametersTmm2D.reserve(reservedMemory*mContext->numDistributions(Context::tmm)*8);
    mParametersGmm2D.reserve(reservedMemory*mContext->numDistributions(Context::gmm)*8);

    mTokensStage2.reserve(reservedMemory*3);
    mParametersTmm3D.reserve(reservedMemory*mContext->numDistributions(Context::tmm)*16);
    mParametersGmm3D.reserve(reservedMemory*mContext->numDistributions(Context::gmm)*16);
  }

  std::vector<float> Bookmarks::getParametersStd140(const Context::Model& model, const int& stage) const {
    switch(stage) {
      case 0: return Context::tmm == model ? mParametersTmm1D : mParametersGmm1D;
      case 1: return Context::tmm == model ? mParametersTmm2D : mParametersGmm2D;
      case 2: return Context::tmm == model ? mParametersTmm3D : mParametersGmm3D;
      default: return Context::tmm == model ? mParametersTmm1D : mParametersGmm1D;
    }
  }

  std::vector<int> Bookmarks::getTokens(const int& stage) const {
    std::vector<int> tokens;
    switch(stage) {
      case 0: {
        tokens.reserve(mTokensStage0.size()*3);
        for(const auto& token : mTokensStage0)
          tokens.insert(tokens.end(), {int(token.model), token.set, token.comp});
        return tokens;
      }
      case 1: {
        tokens.reserve(mTokensStage1.size()*3);
        for(const auto& token : mTokensStage1)
          tokens.insert(tokens.end(), {int(token.model), token.set, token.comp});
        return tokens;
      }
      case 2: {
        tokens.reserve(mTokensStage2.size()*3);
        for(const auto& token : mTokensStage2)
          tokens.insert(tokens.end(), {int(token.model), token.set, token.comp});
        return tokens;
      }
      default: {return tokens;}
    }
  }

  void Bookmarks::prepareContext(const Context::Model& model, const int& stage, const int& index) const {
    switch (stage) {
      case 0: {break;}
      case 1: {
        if(mContext->mState[0].set != mTokensStage1[2*index].set || mContext->mState[0].comp != mTokensStage1[2*index].comp) {
          mContext->prepare(model, 1, mTokensStage1[2*index].set, mTokensStage1[2*index].comp);
        }
        break;
      }
      case 2: {
        if(mContext->mState[0].set != mTokensStage2[3*index].set || mContext->mState[0].comp != mTokensStage2[3*index].comp) {
          mContext->prepare(model, 1, mTokensStage2[3*index].set, mTokensStage2[3*index].comp);
          mContext->prepare(model, 2, mTokensStage2[3*index+1].set, mTokensStage2[3*index+1].comp);
        } else if(mContext->mState[1].set != mTokensStage2[3*index+1].set || mContext->mState[1].comp != mTokensStage2[3*index+1].comp) {
          mContext->prepare(model, 2, mTokensStage2[3*index+1].set, mTokensStage2[3*index+1].comp);
        }
        break;
      }
    }
  }

  std::array<int, 3> Bookmarks::getIndexSetAndCompAndOffset(const int& stage, const int& index) const {
    std::array<int, 3> iSetAndCompAndOffset = {0, 0, 0};
    switch (stage) {
      case 0: {
        iSetAndCompAndOffset[0] = mTokensStage0[index].set;
        iSetAndCompAndOffset[1] = mTokensStage0[index].comp;
        iSetAndCompAndOffset[2] = 1;
        break;
      }
      case 1: {
        iSetAndCompAndOffset[0] = mTokensStage1[2*index+1].set;
        iSetAndCompAndOffset[1] = mTokensStage1[2*index+1].comp;
        iSetAndCompAndOffset[2] = 0 == mTokensStage1[2*index].set ? 1 : 0;
        break;
      }
      case 2: {
        iSetAndCompAndOffset[0] = mTokensStage2[3*index+2].set;
        iSetAndCompAndOffset[1] = mTokensStage2[3*index+2].comp;
        iSetAndCompAndOffset[2] = (0 == mTokensStage2[3*index].set && 0 == mTokensStage2[3*index+1].set) ? 1 : 0;
        break;
      }
    }
    return iSetAndCompAndOffset;
  }

  std::vector<int> Bookmarks::getIndices(const int& stage, const Context::Model& model) {
    int numComponentsSets = mContext->numComponentsSets(stage);
    int numComponents = mContext->numComponents(stage);
    mIndexArray = std::vector<int>(numComponents*numComponentsSets, 0);
    switch(stage) {
      case 0: {
        for(const auto& token : mTokensStage0)
          if(token.model == model || token.model == Context::nomm)
            mIndexArray[token.set*numComponents + token.comp] = 1;
        break;
      }
      case 1: {
        for(int i = 0; i < mTokensStage1.size(); i+=2)
          if(mTokensStage1[i].model == mContext->mState[0].model && mTokensStage1[i].set == mContext->mState[0].set && mTokensStage1[i].comp == mContext->mState[0].comp)
            if(mTokensStage1[i+1].model == model || mTokensStage1[i+1].model == Context::nomm)
              mIndexArray[mTokensStage1[i+1].set*numComponents + mTokensStage1[i+1].comp] = 1;
        break;
      }
      case 2: {
        for(int i = 0; i < mTokensStage2.size(); i+= 3)
          if(mTokensStage2[i].model == mContext->mState[0].model && mTokensStage2[i].set == mContext->mState[0].set && mTokensStage2[i].comp == mContext->mState[0].comp
             && mTokensStage2[i+1].model == mContext->mState[1].model && mTokensStage2[i+1].set == mContext->mState[1].set && mTokensStage2[i+1].comp == mContext->mState[1].comp)
            if(mTokensStage2[i+2].model == model || mTokensStage2[i+2].model == Context::nomm)
              mIndexArray[mTokensStage2[i+2].set*numComponents + mTokensStage2[i+2].comp] = 1;
        break;
      }
      default: {break;}
    }
    return mIndexArray;
  }

  void Bookmarks::add(const int& stage, const Context::Model& model, const int& iCompSet, const int& iComp) {
    int numComponents = mContext->numComponents(stage);
    std::cout << "[Info](Bookmarks::add) mIndexArray =" << std::endl;
    int i = 0;
    for(const auto& elem : mIndexArray) std::cout << elem << (++i%numComponents == 0 ? "\n" : ", ");
    std::cout << std::endl;

    switch(stage) {
      case 0: {
        if(mIndexArray[iCompSet*numComponents + iComp] == 1) return;
        mIndexArray[iCompSet*numComponents + iComp] = 1;
        int showCanonBasis = 1;
        mTokensStage0.push_back(Context::Token(iCompSet <= showCanonBasis ? Context::nomm : model, iCompSet, iComp));
        std::cout << "[Info](Bookmarks::add) Bookmarks:" << std::endl;
        for(const auto& token : mTokensStage0) std::cout << token.model << " " << token.set << " " << token.comp << std::endl;

        auto [parametersTmm, parametersGmm] = mContext->getParametersStd140(stage, iCompSet, iComp);
        mParametersTmm1D.insert(mParametersTmm1D.end(), parametersTmm.begin(), parametersTmm.end());
        mParametersGmm1D.insert(mParametersGmm1D.end(), parametersGmm.begin(), parametersGmm.end());
        std::cout << "[Info](Bookmarks::add) mParametersTmm1D =" << std::endl;
        i = 0;
        for(const auto& elem : mParametersTmm1D) std::cout << elem << (++i%4 == 0 ? "\n" : ", ");
        std::cout << "[Info](Bookmarks::add) mParametersGmm1D =" << std::endl;
        for(const auto& elem : mParametersGmm1D) std::cout << elem << (++i%4 == 0 ? "\n" : ", ");
        break;
      }
      case 1: {
        if(mIndexArray[iCompSet*numComponents + iComp] == 1) return;
        mIndexArray[iCompSet*numComponents + iComp] = 1;
        int showCanonBasis = mContext->showCanonicalBasis(stage);
        mTokensStage1.insert(mTokensStage1.end(), {mContext->mState[0],
                                                   Context::Token(iCompSet <= showCanonBasis ? Context::nomm : model, iCompSet, iComp)});
        std::cout << "[Info](Bookmarks::add) Bookmarks:" << std::endl;
        for(const auto& token : mTokensStage1) std::cout << token.model << " " << token.set << " " << token.comp << std::endl;

        auto [parametersTmm, parametersGmm] = mContext->getParametersStd140(stage, iCompSet, iComp);
        mParametersTmm2D.insert(mParametersTmm2D.end(), parametersTmm.begin(), parametersTmm.end());
        mParametersGmm2D.insert(mParametersGmm2D.end(), parametersGmm.begin(), parametersGmm.end());
        std::cout << "[Info](Bookmarks::add) mParametersTmm2D =" << std::endl;
        int i = 0;
        for(const auto& elem : mParametersTmm2D) std::cout << elem << (++i%4 == 0 ? "\n" : ", ");
        std::cout << "[Info](Bookmarks::add) mParametersGmm2D =" << std::endl;
        i = 0;
        for(const auto& elem : mParametersGmm2D) std::cout << elem << (++i%4 == 0 ? "\n" : ", ");
        break;

        break;
      }
      case 2: {
        if(mIndexArray[iCompSet*numComponents + iComp] == 1) return;
        mIndexArray[iCompSet*numComponents + iComp] = 1;
        int showCanonBasis = mContext->showCanonicalBasis(stage);
        mTokensStage2.insert(mTokensStage2.end(), {mContext->mState[0], mContext->mState[1],
                                                   Context::Token(iCompSet <= showCanonBasis ? Context::nomm : model, iCompSet, iComp)});
        std::cout << "[Info](Bookmarks::add) Bookmarks:" << std::endl;
        for(const auto& token : mTokensStage2) std::cout << token.model << " " << token.set << " " << token.comp << std::endl;
        auto [parametersTmm, parametersGmm] = mContext->getParametersStd140(stage, iCompSet, iComp);
        mParametersTmm3D.insert(mParametersTmm3D.end(), parametersTmm.begin(), parametersTmm.end());
        mParametersGmm3D.insert(mParametersGmm3D.end(), parametersGmm.begin(), parametersGmm.end());
        std::cout << "[Info](Bookmarks::add) mParametersTmm3D =" << std::endl;
        int i = 0;
        for(const auto& elem : mParametersTmm3D) std::cout << elem << (++i%4 == 0 ? "\n" : ", ");
        std::cout << "[Info](Bookmarks::add) mParametersGmm3D =" << std::endl;
        i = 0;
        for(const auto& elem : mParametersGmm3D) std::cout << elem << (++i%4 == 0 ? "\n" : ", ");
        break;
      }
      default: {
        break;
      }
    }
  }
  void Bookmarks::remove(const int& stage, const Context::Model& model, const int& iCompSet, const int& iComp) {
    int numComponents = mContext->numComponents(stage);
    int numDistributionsTmm = mContext->numDistributions(Context::tmm);
    int numDistributionsGmm = mContext->numDistributions(Context::gmm);
    std::cout << "[Info](Bookmarks::remove) mIndexArray =" << std::endl;
    int i = 0;
    for(const auto& elem : mIndexArray) std::cout << elem << (++i%numComponents == 0 ? "\n" : ", ");

    switch(stage) {
      case 0: {
        if(mIndexArray[iCompSet*numComponents + iComp] == 0) return;
        mIndexArray[iCompSet*numComponents + iComp] = 0;
        int showCanonBasis = mContext->showCanonicalBasis(stage);
        auto aModel = iCompSet <= showCanonBasis ? Context::nomm : model;

        for(auto it = mTokensStage0.begin(); it != mTokensStage0.end();) {
          if(it[0].set == iCompSet && it[0].comp == iComp && it[0].model == aModel) {
             it = mTokensStage0.erase(it);
             int i = std::distance(mTokensStage0.begin(), it);
             mParametersTmm1D.erase(mParametersTmm1D.begin()+i*4*numDistributionsTmm, mParametersTmm1D.begin()+(i+1)*4*numDistributionsTmm);
             mParametersGmm1D.erase(mParametersGmm1D.begin()+i*4*numDistributionsGmm, mParametersGmm1D.begin()+(i+1)*4*numDistributionsGmm);
             std::cout << "[Info](Bookmarks::remove) mParametersTmm1D =" << std::endl;
             i = 0;
             for(const auto& elem : mParametersTmm1D) std::cout << elem << (++i%4 == 0 ? "\n" : ", ");
             std::cout << "[Info](Bookmarks::add) mParametersGmm1D =" << std::endl;
             for(const auto& elem : mParametersGmm1D) std::cout << elem << (++i%4 == 0 ? "\n" : ", ");
             break;
           }
          else
            ++it;
        }
        std::cout << "[Info](Bookmarks::remove) Bookmarks:" << std::endl;
        for(const auto& token : mTokensStage0) std::cout << token.model << " " << token.set << " " << token.comp << std::endl;
        break;
      }
      case 1: {
        if(mIndexArray[iCompSet*numComponents + iComp] == 0) return;
        mIndexArray[iCompSet*numComponents + iComp] = 0;
        int showCanonBasis = mContext->showCanonicalBasis(stage);
        auto aModel = iCompSet <= showCanonBasis ? Context::nomm : model;

        for(auto it = mTokensStage1.begin(); it != mTokensStage1.end();) {
          if(it[0].set == mContext->mState[0].set && it[0].comp == mContext->mState[0].comp && it[0].model == mContext->mState[0].model
             && it[1].set == iCompSet && it[1].comp == iComp && it[1].model == aModel) {
             int i = std::distance(mTokensStage1.begin(), it)/2;
             mParametersTmm2D.erase(mParametersTmm2D.begin()+i*8*numDistributionsTmm, mParametersTmm2D.begin()+(i+1)*8*numDistributionsTmm);
             mParametersGmm2D.erase(mParametersGmm2D.begin()+i*8*numDistributionsGmm, mParametersGmm2D.begin()+(i+1)*8*numDistributionsGmm);
             it = mTokensStage1.erase(it, it+2);
             std::cout << "[Info](Bookmarks::remove) mParametersTmm2D =" << std::endl;
             i = 0;
             for(const auto& elem : mParametersTmm2D) std::cout << elem << (++i%4 == 0 ? "\n" : ", ");
             std::cout << "[Info](Bookmarks::remove) mParametersGmm2D =" << std::endl;
             i = 0;
             for(const auto& elem : mParametersGmm2D) std::cout << elem << (++i%4 == 0 ? "\n" : ", ");
             break;
           }
          else
            it+= 2;
        }
        std::cout << "[Info](Bookmarks::remove) Bookmarks:" << std::endl;
        for(const auto& token : mTokensStage1) std::cout << token.model << " " << token.set << " " << token.comp << std::endl;
        break;
      }
      case 2: {
        if(mIndexArray[iCompSet*numComponents + iComp] == 0) return;
        mIndexArray[iCompSet*numComponents + iComp] = 0;
        int showCanonBasis = mContext->showCanonicalBasis(stage);
        auto aModel = iCompSet <= showCanonBasis ? Context::nomm : model;

        for(auto it = mTokensStage2.begin(); it != mTokensStage2.end();) {
          if(it[0].set == mContext->mState[0].set && it[0].comp == mContext->mState[0].comp && it[0].model == mContext->mState[0].model
             && it[1].set == mContext->mState[1].set && it[1].comp == mContext->mState[1].comp && it[1].model == mContext->mState[1].model
             && it[2].set == iCompSet && it[2].comp == iComp && it[2].model == aModel) {
             int i = std::distance(mTokensStage2.begin(), it)/3;
             mParametersTmm3D.erase(mParametersTmm3D.begin()+i*16*numDistributionsTmm, mParametersTmm3D.begin()+(i+1)*16*numDistributionsTmm);
             mParametersGmm3D.erase(mParametersGmm3D.begin()+i*16*numDistributionsGmm, mParametersGmm3D.begin()+(i+1)*16*numDistributionsGmm);
             it = mTokensStage2.erase(it, it+3);
             std::cout << "[Info](Bookmarks::remove) mParametersTmm3D =" << std::endl;
             i = 0;
             for(const auto& elem : mParametersTmm3D) std::cout << elem << (++i%4 == 0 ? "\n" : ", ");
             std::cout << "[Info](Bookmarks::remove) mParametersGmm3D =" << std::endl;
             i = 0;
             for(const auto& elem : mParametersGmm3D) std::cout << elem << (++i%4 == 0 ? "\n" : ", ");
             break;
           }
          else
            it+= 3;
        }
        std::cout << "[Info](Bookmarks::remove) Bookmarks:" << std::endl;
        for(const auto& token : mTokensStage2) std::cout << token.model << " " << token.set << " " << token.comp << std::endl;
        break;
      }
      default: {
        break;
      }
    }
  }
  void Bookmarks::remove(const int& stage, const int& index) {
    int numComponents = mContext->numComponents(stage);
    int numDistributionsTmm = mContext->numDistributions(Context::tmm);
    int numDistributionsGmm = mContext->numDistributions(Context::gmm);
    std::cout << "[Info](Bookmarks::remove) mIndexArray =" << std::endl;
    int i = 0;
    for(const auto& elem : mIndexArray) std::cout << elem << (++i%numComponents == 0 ? "\n" : ", ");

    switch(stage) {
      case 0: {
        if(index > mTokensStage0.size()) return;
        Context::Token token = mTokensStage0[index];
        mIndexArray[token.set*numComponents + token.comp] = 0;
        mTokensStage0.erase(mTokensStage0.begin()+index);
        mParametersTmm1D.erase(mParametersTmm1D.begin()+index*4*numDistributionsTmm, mParametersTmm1D.begin()+(index+1)*4*numDistributionsTmm);
        mParametersGmm1D.erase(mParametersGmm1D.begin()+index*4*numDistributionsGmm, mParametersGmm1D.begin()+(index+1)*4*numDistributionsGmm);
        std::cout << "[Info](Bookmarks::remove) mParametersTmm1D =" << std::endl;
        int i = 0;
        for(const auto& elem : mParametersTmm1D) std::cout << elem << (++i%4 == 0 ? "\n" : ", ");
        std::cout << "[Info](Bookmarks::add) mParametersGmm1D =" << std::endl;
        for(const auto& elem : mParametersGmm1D) std::cout << elem << (++i%4 == 0 ? "\n" : ", ");
        break;
      }
      case 1: {
        if(index > mTokensStage1.size()/2) return;
        Context::Token token = mTokensStage1[2*index+1];
        mIndexArray[token.set*numComponents + token.comp] = 0;
        mTokensStage1.erase(mTokensStage1.begin()+2*index, mTokensStage1.begin()+2*index+2);
        mParametersTmm2D.erase(mParametersTmm2D.begin()+index*8*numDistributionsTmm, mParametersTmm2D.begin()+(index+1)*8*numDistributionsTmm);
        mParametersGmm2D.erase(mParametersGmm2D.begin()+index*8*numDistributionsGmm, mParametersGmm2D.begin()+(index+1)*8*numDistributionsGmm);
        std::cout << "[Info](Bookmarks::remove) mParametersTmm2D =" << std::endl;
        int i = 0;
        for(const auto& elem : mParametersTmm2D) std::cout << elem << (++i%4 == 0 ? "\n" : ", ");
        std::cout << "[Info](Bookmarks::remove) mParametersGmm2D =" << std::endl;
        i = 0;
        for(const auto& elem : mParametersGmm2D) std::cout << elem << (++i%4 == 0 ? "\n" : ", ");
        break;
      }
      case 2: {
        if(index > mTokensStage2.size()/3) return;
        Context::Token token = mTokensStage2[3*index+2];
        mIndexArray[token.set*numComponents + token.comp] = 0;
        mTokensStage2.erase(mTokensStage2.begin()+3*index, mTokensStage2.begin()+3*index+3);
        mParametersTmm3D.erase(mParametersTmm3D.begin()+index*16*numDistributionsTmm, mParametersTmm3D.begin()+(index+1)*16*numDistributionsTmm);
        mParametersGmm3D.erase(mParametersGmm3D.begin()+index*16*numDistributionsGmm, mParametersGmm3D.begin()+(index+1)*16*numDistributionsGmm);
        std::cout << "[Info](Bookmarks::remove) mParametersTmm3D =" << std::endl;
        int i = 0;
        for(const auto& elem : mParametersTmm3D) std::cout << elem << (++i%4 == 0 ? "\n" : ", ");
        std::cout << "[Info](Bookmarks::remove) mParametersGmm3D =" << std::endl;
        i = 0;
        for(const auto& elem : mParametersGmm3D) std::cout << elem << (++i%4 == 0 ? "\n" : ", ");
        break;
      }
      default: {
        break;
      }
    }
  }
  int Bookmarks::size(const int& stage) const {
    switch(stage) {
      case 0: {return mTokensStage0.size();}
      case 1: {return mTokensStage1.size()/2;}
      case 2: {return mTokensStage2.size()/3;}
      default: {return 0;}
    }
  }
} // namespace context
