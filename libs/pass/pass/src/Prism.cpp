#include "pass/Base.hpp"
#include "pass/Prism.hpp"

#include <memory>
#include <numeric>
#include <chrono>
#include <glm/gtx/io.hpp>

#include "context/Context.hpp"
#include "context/Bookmarks.hpp"
#include "vis/Camera.hpp"
#include "glPP/Shader.hpp"
#include "glPP/Texture.hpp"
#include "glPP/Buffer.hpp"
#include "glPP/Framebuffer.hpp"
#include "pass/Rendering.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace pass{
  Prism::Prism()
  : mContext(nullptr), mBookmarksContext(nullptr),
    mCamera(nullptr), mCameraTile(nullptr), mStage(0),
    mVertexBuffer(), mVertexArray(), mModelParameters(), mScaleByWeight(true),
    mThreshold(0.005f), mDomainScale({5.0f, 4.0f/3.0f}), mDistributionDepth(3.5f)
  {}

  void Prism::init(const std::shared_ptr<context::Context>& context,
                   const std::shared_ptr<context::Bookmarks>& bookmarksContext,
                   const std::shared_ptr<const vis::Camera>& camera,
                   const std::shared_ptr<vis::Camera>& cameraTile)
  {
    mContext = context;
    mBookmarksContext = bookmarksContext;
    mCamera = camera;
    mCameraTile = cameraTile;
    mStage = 0;

    auto [rows, cols] = gridSize(mContext->dimensions());
    mNumPhysTilesPerFacet = rows*cols;
    auto hexagonalPrism = hexagonalPrismInSphere(float(cols)/rows, sScalar);
    mTileSize = (hexagonalPrism[1] - hexagonalPrism[4]) / rows;
    mFrontFacePosZ = hexagonalPrism[8];

    std::vector<float> vertices = generateWindowedRing_alt(hexagonalPrism, rows, cols);
    mNumVertices = vertices.size()/3;
    mVertexBuffer.setData(vertices);
    mVertexArray.setAttribute(0, 3, GL_FLOAT, 0*sizeof(float));
    mVertexArray.bindVertexBuffer(mVertexBuffer, 0, 3*sizeof(float));

    auto faceNormals = getFaceNormals<6>(hexagonalPrism);
    mUboNormals.setData(faceNormals);
    int pointsPerFace = mNumPhysTilesPerFacet*6;
    auto tileCentersFrontFace = getTileCenters(vertices.begin()+pointsPerFace*3, vertices.begin()+2*pointsPerFace*3);
    mUboTileCenters.setData(tileCentersFrontFace);

    std::array<int, 3> selected = {-1, -1, -1};
    mUIElement = glPP::Texture<GL_TEXTURE_1D>(selected, GL_RED_INTEGER, GL_R16I, {3}, GL_NEAREST, GL_CLAMP_TO_EDGE);

    mTokenLenght = 6;
    auto attributes = mContext->getAttributes();
    std::vector<unsigned char> fontDigits = attributesToFontDigits(attributes, mTokenLenght);
    mAttributes = glPP::Texture<GL_TEXTURE_1D>(fontDigits, GL_RED_INTEGER, GL_R8UI, {mTokenLenght*mContext->numComponents(0)}, GL_NEAREST, GL_CLAMP_TO_EDGE);

    int width, height, channels;
    std::string path = std::string(CMAKE_TEXTUREDIR) + "font.png";
    stbi_set_flip_vertically_on_load(true);
    unsigned char* fontArr = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if(fontArr) {
      std::vector<unsigned char> font(fontArr, fontArr+width*height*channels);
      mFont = glPP::Texture<GL_TEXTURE_2D>(font, GL_RGBA, GL_RGBA8, {width, height}, GL_LINEAR, GL_CLAMP_TO_EDGE);
      stbi_image_free(fontArr);
    } else {std::cout << "[Error](Prism::init) " << path << " not found" << std::endl;}

    mScaleByWeight = true;
    mThreshold = 2.0f;
    mThresholdStored = 0.005f;
    mAdoptToStd = true;
    mDomainScale = {10.0f, 4.0f/3.0f};
    mCodomainScale = 1.0f;
    mDistributionDepth = 3.5f;
    mModel = context::Context::tmm;

    mColorsStudent.resize(mContext->numDistributions(context::Context::tmm)*3);
    for(int i = 0; i < mColorsStudent.size(); ++i) mColorsStudent[i] = colors[i%colors.size()];
    mColorsGaussian.resize(mContext->numDistributions(context::Context::gmm)*3);
    for(int i = 0; i < mColorsGaussian.size(); ++i) mColorsGaussian[i] = colors[i%colors.size()];

    updateSpecs();
    refresh();
    //setFixUniforms();
    prepareBuffersAndTextures();

    std::string shaderpath = std::string(CMAKE_SHADERDIR) + "prism/";
    mWireShader.setSpecs({
      glPP::Shader::Specs(GL_VERTEX_SHADER, shaderpath + "wire.vs"),
      glPP::Shader::Specs(GL_GEOMETRY_SHADER, shaderpath + "wire.gs"),
      glPP::Shader::Specs(GL_FRAGMENT_SHADER, shaderpath + "wire.fs")
    });
    mWireShader.refresh();
    mWireShader.use();
    mWireShader.setMat4("projection", mCamera->getProjectionMatrix());
  }

  std::vector<unsigned char> Prism::attributesToFontDigits(const std::vector<std::string>& attributes, const int& numLetters) const {
    std::vector<unsigned char> fontDigits(attributes.size()*numLetters);
    int j = 0;
    for(const auto& attribute : attributes) {
      for(int i = 0; i < numLetters; ++i) {
        fontDigits[j++] = i < attribute.size() ? attribute[i]-'a'+97 : 32;
      }
    }
    return fontDigits;
  }

  void Prism::updateSpecs() {
    std::string shaderpath = std::string(CMAKE_SHADERDIR) + "prism/level" + std::to_string(mStage);
    mShader.setSpecs({
      glPP::Shader::Specs(GL_VERTEX_SHADER, shaderpath + ".vs"),
      glPP::Shader::Specs(GL_FRAGMENT_SHADER, shaderpath + ".fs",
                        "const int mixtureModel = " + std::to_string(mModel) + ";",
                        "const int rendering = " + std::to_string(mRendering) + ";",
                        "const int numPhysFacets = " + std::to_string(6) + ";",
                        "const int numPhysTilesPerFacet = " + std::to_string(mNumPhysTilesPerFacet) + ";",
                        "const float tileSize = " + std::to_string(mTileSize) + ";",
                        "const int numComponents = " + std::to_string(mContext->numComponents(mStage)) + ";",
                        "const int dimensions = " + std::to_string(mContext->dimensions()) + ";",
                        "const int numDistributions = " + std::to_string(mContext->numDistributions(mModel)) + ";",
                        "const int showCanonicalBasis = " + std::to_string(mContext->showCanonicalBasis(mStage)) + ";",
                        "const int numFacets = " + std::to_string(mContext->numDistributions(mModel)+1+mContext->showCanonicalBasis(mStage)) + ";",
                        "const int tokenLength = " + std::to_string(mTokenLenght) + ";")
    });
  }

  void Prism::resizeTextures(const int& width, const int& height) {
    mShader.use();
    mShader.setMat4("projection", mCamera->getProjectionMatrix());
    mWireShader.use();
    mWireShader.setMat4("projection", mCamera->getProjectionMatrix());
  }

  void Prism::prepareBuffersAndTextures() {
    auto modelParameters = mContext->getParametersStd140(mModel, mStage);
    mModelParameters.resizeAndClear(modelParameters.size()*sizeof(typename decltype(modelParameters)::value_type));
    mModelParameters.setSubData(modelParameters);

    const int numComponentsSets = mContext->numComponentsSets(mStage);
    const int numComponents = mContext->numComponents(mStage);

    auto bookmarks = mBookmarksContext->getIndices(mStage, mModel);
    mBookmarks = glPP::Texture<GL_TEXTURE_1D>(bookmarks, GL_RED_INTEGER, GL_R16I, {numComponents*numComponentsSets}, GL_NEAREST, GL_CLAMP_TO_EDGE);

    std::vector<float> components = mContext->getComponents(mStage);
    mComponents = glPP::Texture<GL_TEXTURE_2D>(components, GL_RED, GL_R32F, {mContext->dimensions(), numComponents*numComponentsSets}, GL_NEAREST, GL_CLAMP_TO_EDGE);

    auto eigenvalues = mContext->getScaledEigenValues(mModel, mStage);
    mEigenvalueIndices = std::vector<int>(eigenvalues.size());
    std::iota(mEigenvalueIndices.begin(), mEigenvalueIndices.end(), 0);
    if(1 == mContext->showCanonicalBasis(mStage)) {
      std::stable_sort(mEigenvalueIndices.begin(), mEigenvalueIndices.begin()+numComponents,
        [&eigenvalues](int i1, int i2) {return eigenvalues[i1] > eigenvalues[i2];});
    }

    auto sparsity = mContext->getSparsity(mStage);
    mSparsityIndices = std::vector<int>(sparsity.size());
    std::iota(mSparsityIndices.begin(), mSparsityIndices.end(), 0);
    for(int i = 0; i < sparsity.size()/numComponents; ++i) {
      std::stable_sort(mSparsityIndices.begin()+i*numComponents, mSparsityIndices.begin()+(i+1)*numComponents,
        [&sparsity](int i1, int i2) {return sparsity[i1] > sparsity[i2];});
    }

    mNumMetrics = 3+int(mStage == 2);
    mMetrics = glPP::Texture<GL_TEXTURE_2D>(GL_R32F, {numComponents*numComponentsSets, mNumMetrics}, GL_NEAREST, GL_CLAMP_TO_EDGE);
    mMetrics.setSubImage(eigenvalues, {0, 0}, {numComponents*numComponentsSets, 1});
    mMetrics.setSubImage(sparsity, {0, 1}, {numComponents*numComponentsSets, 1});

    int numDistributions = mContext->numDistributions(mModel);
    if(2 == mStage) {
      //mMetrics.setSubImage(sparsity, {0, 3}, {numComponents*numComponentsSets, 1});
      mOverlap = glPP::Texture<GL_TEXTURE_3D>(GL_R32F, {numDistributions*numDistributions, numComponents, numComponentsSets}, GL_NEAREST, GL_CLAMP_TO_EDGE);
      mNumPixels = glPP::Texture<GL_TEXTURE_1D>(GL_R32I, {numComponents*numComponentsSets}, GL_NEAREST, GL_CLAMP_TO_EDGE);
    }

    mUpdateVisibilityMetric = false;
    mOverlapNew = glPP::Texture<GL_TEXTURE_2D>(GL_R32F, {numDistributions*numDistributions, numComponents*numComponentsSets}, GL_NEAREST, GL_CLAMP_TO_EDGE);
    updateVisibility();

    sortTiles(mMetric);
  }

  void Prism::setFixUniforms() const {
    mShader.use();

    auto weights = mScaleByWeight ? mContext->getWeights(mModel)
                                  : std::vector<float>(mContext->numDistributions(mModel), 1.0f);
    mShader.setFloat("weights", weights, weights.size());
    mShader.setMat4("projection", mCamera->getProjectionMatrix());

    auto& colors = context::Context::tmm == mModel ? mColorsStudent : mColorsGaussian;
    for(auto& color : colors) mShader.setVec3("colors", colors, colors.size()/3);
  }

  std::vector<float> Prism::getViewPointEntropy(const Eigen::VectorXf& evalData, const int& area) const {
    int numDistributions = mContext->numDistributions(mModel);
    int numComponents = mContext->numComponents(mStage);
    int numComponentsSets = mContext->numComponentsSets(mStage);

    std::vector<float> entropy(numComponentsSets*numComponents, 0.0f);
    for(int tile = 0; tile < entropy.size(); ++tile) {
      int baseIndex = tile*numDistributions*numDistributions;
      float relativeAreaBackground = 1.0f;
      for(int i = 0; i < numDistributions; ++i) {
        float relativeAreaDis = evalData[baseIndex + i*numDistributions + i] / float(area);
        relativeAreaBackground-= relativeAreaDis;
        entropy[tile]-= relativeAreaDis < 1e-6 ? 0.0f : relativeAreaDis*std::log(relativeAreaDis);
      }
      entropy[tile]-= relativeAreaBackground < 1e-6 ? 0.0f : relativeAreaBackground*std::log(relativeAreaBackground);
      entropy[tile]/= log(numDistributions+1);
    }
    return entropy;
  }

  std::vector<float> Prism::getSumSoftMax(const Eigen::VectorXf& evalData, const Eigen::VectorXi& numPixels) const {
    int numDistributions = mContext->numDistributions(mModel);
    int numComponents = mContext->numComponents(mStage);
    int numComponentsSets = mContext->numComponentsSets(mStage);

    //std::vector<float> sumSoftMax(numComponentsSets*numComponents, 1.0f);
    std::vector<float> sumSoftMax(numComponentsSets*numComponents, 0.0f);
    for(int window = 0; window < sumSoftMax.size(); ++window) {
      int baseIndex = window*numDistributions*numDistributions;
      for(int i = 0; i < numDistributions; ++i) {
        // subtracting max makes softmax numerical stable
        const auto begin = evalData.begin()+baseIndex+i*numDistributions;
        const auto end = evalData.begin()+baseIndex+i*numDistributions+numDistributions;
        float max = *std::max_element(begin, end);
        float resolutionNormalizer = 256.0f*256.0f/float(numPixels[window]);
        float sum = 0.0f;
        for(int j = 0; j < numDistributions; ++j) {
          float value = i == j ? (evalData[baseIndex + i*numDistributions + j]-max) : (evalData[baseIndex + i*numDistributions + j]-max);
          sum+= std::exp(resolutionNormalizer*value);
        }
        //sumSoftMax[window]*= std::exp(resolutionNormalizer*(evalData[baseIndex + i*numDistributions + i]-max))/sum;
        sumSoftMax[window]+= std::exp(resolutionNormalizer*(evalData[baseIndex + i*numDistributions + i]-max))/sum/numDistributions;
      }
    }
    return sumSoftMax;
  }

  std::vector<float> Prism::getSumSoftMax(const Eigen::VectorXf& evalData, const int& numPixels) const {
    int numDistributions = mContext->numDistributions(mModel);
    int numComponents = mContext->numComponents(mStage);
    int numComponentsSets = mContext->numComponentsSets(mStage);

    //std::vector<float> sumSoftMax(numComponentsSets*numComponents, 1.0f);
    std::vector<float> sumSoftMax(numComponentsSets*numComponents, 0.0f);
    for(int window = 0; window < sumSoftMax.size(); ++window) {
      int baseIndex = window*numDistributions*numDistributions;
      for(int i = 0; i < numDistributions; ++i) {
        const auto begin = evalData.begin()+baseIndex+i*numDistributions;
        const auto end = evalData.begin()+baseIndex+i*numDistributions+numDistributions;
        float max = *std::max_element(begin, end);
        float resolutionNormalizer = 256.0f*256.0f/float(numPixels);
        float sum = 0.0f;
        for(int j = 0; j < numDistributions; ++j) {
          // subtracting max makes softmax numerical stable
          float value = i == j ? (evalData[baseIndex + i*numDistributions + j]-max) : (evalData[baseIndex + i*numDistributions + j]-max);
          sum+= std::exp(resolutionNormalizer*value);
        }
        //sumSoftMax[window]*= std::exp(resolutionNormalizer*(evalData[baseIndex + i*numDistributions + i]-max))/sum;
        // subtracting max makes softmax numerical stable
        sumSoftMax[window]+= std::exp(resolutionNormalizer*(evalData[baseIndex + i*numDistributions + i]-max))/sum/numDistributions;
      }
    }
    return sumSoftMax;
  }

  void Prism::updateVisibility() const {
    if(not mCalculateVisibility) {
      int numComponents = mContext->numComponents(mStage);
      int numComponentsSets = mContext->numComponentsSets(mStage);
      std::vector<float> sumSoftMax = std::vector<float>(numComponents*numComponentsSets, 0.0f);
      mSoftMaxIndices = std::vector<int>(sumSoftMax.size());
      std::iota(mSoftMaxIndices.begin(), mSoftMaxIndices.end(), 0);

      mMetrics.setSubImage(sumSoftMax, {0, 2}, {numComponents*numComponentsSets, 1});
      return;
    }

    std::string shaderpath = std::string(CMAKE_SHADERDIR) + "prism/visibility" + std::to_string(mStage) + ".cs";
    glPP::Shader shader = glPP::Shader({
      glPP::Shader::Specs(GL_COMPUTE_SHADER, shaderpath,
                        "const int mixtureModel = " + std::to_string(mModel) + ";",
                        "const int rendering = " + std::to_string(mRendering) + ";",
                        "const int numPhysFacets = " + std::to_string(6) + ";",
                        "const int numPhysTilesPerFacet = " + std::to_string(mNumPhysTilesPerFacet) + ";",
                        "const float tileSize = " + std::to_string(mTileSize) + ";",
                        "const int numComponents = " + std::to_string(mContext->numComponents(mStage)) + ";",
                        "const int dimensions = " + std::to_string(mContext->dimensions()) + ";",
                        "const int numDistributions = " + std::to_string(mContext->numDistributions(mModel)) + ";",
                        "const int showCanonicalBasis = " + std::to_string(mContext->showCanonicalBasis(mStage)) + ";",
                        "const int numFacets = " + std::to_string(mContext->numDistributions(mModel)+1+mContext->showCanonicalBasis(mStage)) + ";",
                        "const int tokenLength = " + std::to_string(mTokenLenght) + ";")
    });
    shader.use();
    auto weights = mScaleByWeight ? mContext->getWeights(mModel)
                                  : std::vector<float>(mContext->numDistributions(mModel), 1.0f);
    shader.setFloat("weights", weights, weights.size());
    shader.setMat4("projection", mCamera->getProjectionMatrix());
    shader.setFloat("userInput.threshold", mThreshold);
    shader.setFloat("userInput.codomainScale", mCodomainScale);
    shader.setFloat("userInput.domainScale", mDomainScale[std::max(0, mStage-1)]);
    shader.setFloat("userInput.distanceminimizer", mDistributionDepth);
    shader.setBool("userInput.adoptToStd", mAdoptToStd);
    shader.setVec3("camPos", mCamera->getPosition());
    shader.setMat4("view", mCamera->getViewMatrix());
    shader.setMat3("model", glm::mat3(mCameraTile->getViewMatrix()));

    mModelParameters.bind(GL_SHADER_STORAGE_BUFFER, 3);
    mOverlapNew.bindImage(4, GL_WRITE_ONLY);

    glDispatchCompute(512/16, 512/16, mOverlapNew.size(1));
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    auto evalData = mOverlapNew.getImage<Eigen::VectorXf>();
    mOverlapNew.clearImage();
    int numDistributions = mContext->numDistributions(mModel);
    int numComponents = mContext->numComponents(mStage);
    int numComponentsSets = mContext->numComponentsSets(mStage);


    std::vector<float> sumSoftMax = getSumSoftMax(evalData, 512*512);

    //std::vector<float> entropy = getViewPointEntropy(evalData, 512*512);
    //for(int tile = 0; tile < entropy.size(); ++tile) {
    //  std::cout << "entropy[" << tile << "] = " << entropy[tile] << std::endl;
    //}
    // initialize original index locations
    mSoftMaxIndices = std::vector<int>(sumSoftMax.size());
    std::iota(mSoftMaxIndices.begin(), mSoftMaxIndices.end(), 0);

    // sort indexes based on comparing values in v
    // using std::stable_sort instead of std::sort
    // to avoid unnecessary index re-orderings
    // when v contains elements of equal values
    for(int i = 0; i < sumSoftMax.size()/numComponents; ++i) {
      std::stable_sort(mSoftMaxIndices.begin()+i*numComponents, mSoftMaxIndices.begin()+(i+1)*numComponents,
        [&sumSoftMax](int i1, int i2) {return sumSoftMax[i1] > sumSoftMax[i2];});
    }
    mMetrics.setSubImage(sumSoftMax, {0, 2}, {numComponents*numComponentsSets, 1});
    //if(visibility == mMetric)
    //  mIndexArray = glPP::Texture<GL_TEXTURE_1D>(mSoftMaxIndices, GL_RED_INTEGER, GL_R16I, {mSoftMaxIndices.size()}, GL_NEAREST, GL_CLAMP_TO_EDGE);

  }

  const Prism::Metric& Prism::getMetric() const {return mMetric;}
  void Prism::sortTiles(const Metric& metric) {
    switch(metric) {
      case variance: {
        mIndexArray = glPP::Texture<GL_TEXTURE_1D>(mEigenvalueIndices, GL_RED_INTEGER, GL_R16I, {mEigenvalueIndices.size()}, GL_NEAREST, GL_CLAMP_TO_EDGE);
        break;
      }
      case sparsity: {
        mIndexArray = glPP::Texture<GL_TEXTURE_1D>(mSparsityIndices, GL_RED_INTEGER, GL_R16I, {mSparsityIndices.size()}, GL_NEAREST, GL_CLAMP_TO_EDGE);
        break;
      }
      case visibility: {
        updateVisibility();
        mIndexArray = glPP::Texture<GL_TEXTURE_1D>(mSoftMaxIndices, GL_RED_INTEGER, GL_R16I, {mSparsityIndices.size()}, GL_NEAREST, GL_CLAMP_TO_EDGE);
        break;
      }
    }
    mMetric = metric;
  }

  const bool& Prism::getCalculateVisibility() const {return mCalculateVisibility;}
  void Prism::setCalculateVisibility(const bool& calculateVisibility) {mCalculateVisibility = calculateVisibility;}

  const Rendering& Prism::getRendering() const {return mRendering;}
  void Prism::setRendering(const Rendering& rendering, const bool& update) {
    if(rendering == mRendering) return;
    mRendering = rendering;
    if(not update) return;
    updateSpecs();
    refresh();
  }

  float& Prism::refThreshold() {return mThreshold;}
  float& Prism::refDomainScale() {return mDomainScale[std::max(0, mStage-1)];}
  float& Prism::refCodomainScale() {return mCodomainScale;}
  float& Prism::refDistributionDepth() {return mDistributionDepth;}
  const float& Prism::getThreshold() const {return mThreshold;}
  const float& Prism::getDomainScale() const {return mDomainScale[std::max(0, mStage-1)];}
  const float& Prism::getCodomainScale() const {return mCodomainScale;}
  const float& Prism::getDistributionDepth() const {return mDistributionDepth;}
  const float& Prism::getFrontFacePosZ() const {return mFrontFacePosZ;}
  const context::Context::Model& Prism::getModel() const {return mModel;}
  void Prism::setModel(const context::Context::Model& model, const bool& update) {
    if(model == mModel) return;
    mModel = model;
    if(not update) return;
    updateSpecs();
    prepareBuffersAndTextures();
    refresh();
  }

  void Prism::scaleByWeight(const bool& on) {
    if(on == mScaleByWeight) return;
    mScaleByWeight = on;
    auto weights = mScaleByWeight ? mContext->getWeights(mModel)
                                  : std::vector<float>(mContext->numDistributions(mModel), 1.0f);
    mShader.use();
    mShader.setFloat("weights", weights, weights.size());
  }

  void Prism::adoptThresholdToStd(const bool& active) {
    if(active == mAdoptToStd) return;
    mAdoptToStd = active;
    float thresholdLoaded = mThresholdStored;
    mThresholdStored = mThreshold;
    mThreshold = thresholdLoaded;
  }

  std::array<float, 3> Prism::getColor(const int& i) const {
    return context::Context::tmm == mModel ? std::array<float, 3>{mColorsStudent[i*3], mColorsStudent[i*3+1], mColorsStudent[i*3+2]}
                                  : std::array<float, 3>{mColorsGaussian[i*3], mColorsGaussian[i*3+1] , mColorsGaussian[i*3+2]};
  }
  void Prism::setColor(const int& i, const std::array<float, 3>& color, const bool& update) {
    auto& colors = context::Context::tmm == mModel ? mColorsStudent : mColorsGaussian;
    colors[i*3+0] = color[0];
    colors[i*3+1] = color[1];
    colors[i*3+2] = color[2];
    if(not update) return;
    mShader.use();
    mShader.setVec3("colors[" + std::to_string(i) + "]", color);
  }

  void Prism::execute() const {
      glEnable(GL_CULL_FACE);
      glFrontFace(GL_CCW);
      glCullFace(GL_BACK);
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LESS);
        mShader.use();
        mShader.setIvec2("cursorPos", mCursorPos);
        mShader.setFloat("camDegrees", mCamera->getCumulatedXOffset());
        mShader.setFloat("userInput.threshold", mThreshold);
        mShader.setFloat("userInput.codomainScale", mCodomainScale);
        mShader.setFloat("userInput.domainScale", mDomainScale[std::max(0, mStage-1)]);
        mShader.setFloat("userInput.distanceminimizer", mDistributionDepth);
        mShader.setBool("userInput.adoptToStd", mAdoptToStd);
        mShader.setVec3("camPos", mCamera->getPosition());
        mShader.setMat4("view", mCamera->getViewMatrix());
        mShader.setMat3("model", glm::mat3(mCameraTile->getViewMatrix()));
        if(mStage > 0) mShader.setIvec3("selectedTiles", mContext->getState(mStage), mStage);

        mUboNormals.bind(GL_UNIFORM_BUFFER, 0);
        mUboTileCenters.bind(GL_UNIFORM_BUFFER, 1);
        mModelParameters.bind(GL_SHADER_STORAGE_BUFFER, 3);

        mIndexArray.bindImage(0, GL_READ_ONLY);
        mBookmarks.bindImage(1, GL_READ_ONLY);
        mComponents.bindImage(2, GL_READ_ONLY);
        mMetrics.bindImage(3, GL_READ_ONLY);
        mOverlap.bindImage(4, GL_WRITE_ONLY);
        mNumPixels.bindImage(5, GL_WRITE_ONLY);
        mAttributes.bindImage(6, GL_READ_ONLY);
        mUIElement.bindImage(7, GL_READ_WRITE);

        mFont.bind(0);

        mVertexArray.drawArrays(mDrawMode, 0, mNumVertices);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        if(2 == mStage) {
          auto evalData = mOverlap.getImage<Eigen::VectorXf>();
          auto numPixels = mNumPixels.getImage<Eigen::VectorXi>();
          mOverlap.clearImage();
          mNumPixels.clearImage(std::vector<int>{1});
          std::vector<float> sumSoftMax = getSumSoftMax(evalData, numPixels);
          mMetrics.setSubImage(sumSoftMax, {0, 3}, {sumSoftMax.size(), 1});
        };

        mWireShader.use();
        mWireShader.setMat4("view", mCamera->getViewMatrix());
          glEnable(GL_LINE_SMOOTH);
          //glLineWidth(1.0f);
          mVertexArray.drawArrays(mDrawMode, 0, mNumVertices);
          glDisable(GL_LINE_SMOOTH);
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);
  }

  std::array<int, 3> Prism::getUIElementSetAndCompAndIndex() {
    auto UIElement = mUIElement.getImage<std::vector<int>>();
    try{
       std::array<int, 3> clear;
       clear[0] = UIElement[2] == -1 ? -1 : UIElement[0];
       clear[1] = UIElement[2] == -1 ? -1 : UIElement[1];
       clear[2] = -1;
       mUIElement.setImage(clear);
     } catch(const std::string& err) {
       std::cout << err << std::endl;
    }
    return {UIElement[0], UIElement[1], UIElement[2]};
  }

  void Prism::setCursorPos(const std::array<double, 2>& cursorPos) {mCursorPos = {static_cast<int>(cursorPos[0]), static_cast<int>(cursorPos[1])};}

  const int& Prism::getStage() const {return mStage;}

  int Prism::enterNextStage() {
    if(mStage == 2) return mStage;
    mStage++;
    updateSpecs();
    refresh();
    prepareBuffersAndTextures();
    std::cout << "[Info](Prism::enterNextStage)" << std::endl;
    return mStage;
  }

  int Prism::enter() {
    updateSpecs();
    refresh();
    prepareBuffersAndTextures();
    return mStage;
  }

  int Prism::enterPreviousStage() {
    if(mStage == 0) return mStage;
    mStage--;
    updateSpecs();
    refresh();
    prepareBuffersAndTextures();
    std::cout << "[Info](Prism::enterPreviousStage)" << std::endl;
    return mStage;
  }

  void Prism::updateBookmarks() {
    int numComponents = mContext->numComponents(mStage);
    int numComponentsSets = mContext->numComponentsSets(mStage);
    auto bookmarks = mBookmarksContext->getIndices(mStage, mModel);
    mBookmarks = glPP::Texture<GL_TEXTURE_1D>(bookmarks, GL_RED_INTEGER, GL_R16I, {numComponents*numComponentsSets}, GL_NEAREST, GL_CLAMP_TO_EDGE);
  }

} // namespace pass
