#include "pass/Base.hpp"
#include "pass/Bookmarks.hpp"

#include <memory>
#include <string>
#include <glm/gtx/io.hpp>
#include <glm/gtx/string_cast.hpp>

#include "context/Context.hpp"
#include "context/Bookmarks.hpp"
#include "vis/Camera.hpp"
#include "glPP/Shader.hpp"
#include "glPP/Texture.hpp"
#include "glPP/Buffer.hpp"
#include "glPP/Framebuffer.hpp"
#include "pass/Rendering.hpp"

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace pass{
  Bookmarks::Bookmarks()
  : mContext(nullptr), mBookmarksContext(nullptr),
    mCamera(nullptr), mCameraTile(nullptr), mStage(0),
    mVertexBuffer(), mVertexArray(), mModelParameters(),
    mThreshold(0.005f), mDomainScale({5.0f, 4.0f/3.0f}), mDistributionDepth(3.5f)
  {}

  void Bookmarks::init(const std::shared_ptr<context::Context>& context,
                       const std::shared_ptr<context::Bookmarks>& bookmarksContext,
                       const std::shared_ptr<const vis::Camera>& camera,
                       const std::shared_ptr<vis::Camera>& cameraTile,
                       const float& prismTilePosZ)
  {
    mContext = context;
    mBookmarksContext = bookmarksContext;
    mCamera = camera;
    mCameraTile = cameraTile;
    mStage = 0;
    mPrismTilePosZ = prismTilePosZ;
    mNumPhysTilesPerFacet = 5;
    auto [vertices, indices] = getVerticalWindowedStripe(mNumPhysTilesPerFacet);
    mNumVertices = indices.size();
    float tileHeight = 2.0f/mNumPhysTilesPerFacet;
    float ratio = 1.0f*mCamera->getResolution().y/mCamera->getResolution().x;
    float tileWidth = ratio*tileHeight;
    mTileSize = glm::vec2(tileWidth, tileHeight);
    mVertexBuffer.setData(vertices);
    mVertexArray.setAttribute(0, 3, GL_FLOAT, 0*sizeof(float));
    mVertexArray.bindVertexBuffer(mVertexBuffer, 0, 3*sizeof(float));
    mElementBuffer.setData(indices);
    mVertexArray.bindElementBuffer(mElementBuffer);

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
    } else {std::cout << "[Error] (Bookmarks::init) " << path << " not found" << std::endl;}

    update();

    std::string shaderpath = std::string(CMAKE_SHADERDIR) + "bookmarks/";
    mWireShader.setSpecs({
      glPP::Shader::Specs(GL_VERTEX_SHADER, shaderpath + "wire.vs"),
      glPP::Shader::Specs(GL_GEOMETRY_SHADER, shaderpath + "wire.gs"),
      glPP::Shader::Specs(GL_FRAGMENT_SHADER, shaderpath + "wire.fs")
    });
    mWireShader.refresh();
    mWireShader.use();
    mWireShader.setMat4("view", glm::mat4(1.0f));
    mWireShader.setMat4("projection", glm::mat4(1.0f));

    mScaleByWeight = true;
    mThreshold = 2.0f;
    mThresholdStored = 0.005f;
    mAdoptToStd = true;
    mDomainScale = {10.0f, 4.0f/3.0f};
    mCodomainScale = 1.0f;
    mDistributionDepth = 3.5f;

    mColorsStudent.resize(mContext->numDistributions(context::Context::tmm)*3);
    for(int i = 0; i < mColorsStudent.size(); ++i) mColorsStudent[i] = colors[i%colors.size()];
    mColorsGaussian.resize(mContext->numDistributions(context::Context::gmm)*3);
    for(int i = 0; i < mColorsGaussian.size(); ++i) mColorsGaussian[i] = colors[i%colors.size()];
  }

  std::vector<unsigned char> Bookmarks::attributesToFontDigits(const std::vector<std::string>& attributes, const int& numLetters) const {
    std::vector<unsigned char> fontDigits(attributes.size()*numLetters);
    int j = 0;
    for(const auto& attribute : attributes) {
      for(int i = 0; i < numLetters; ++i) {
        fontDigits[j++] = i < attribute.size() ? attribute[i]-'a'+97 : 32;
      }
    }
    return fontDigits;
  }

  void Bookmarks::update() {
    std::string shaderpath = std::string(CMAKE_SHADERDIR) + "bookmarks/level" + std::to_string(mStage);
    mShader.setSpecs({
      glPP::Shader::Specs(GL_VERTEX_SHADER, shaderpath + ".vs"),
      glPP::Shader::Specs(GL_FRAGMENT_SHADER, shaderpath + ".fs",
                        "const int mixtureModel = " + std::to_string(mModel) + ";",
                        "const int rendering = " + std::to_string(mRendering) + ";",
                        "const int numPhysFacets = " + std::to_string(1) + ";",
                        "const int numPhysTilesPerFacet = " + std::to_string(mNumPhysTilesPerFacet) + ";",
                        "uniform vec2 tileSize = vec2(" + std::to_string(mTileSize.x) + ", " + std::to_string(mTileSize.y) + ");",
                        "const int numComponents = " + std::to_string(mBookmarksContext->size(mStage)) + ";",
                        "const int dimensions = " + std::to_string(mContext->dimensions()) + ";",
                        "const int numDistributions = " + std::to_string(mContext->numDistributions(mModel)) + ";",
                        "const int numFacets = " + std::to_string(1) + ";",
                        "const int tokenLength = " + std::to_string(mTokenLenght) + ";")
    });
    refresh();
    //mShader.refresh();
    //setFixUniforms();
  }

  void Bookmarks::enter(const int& stage) {
    mStage = stage;
    update();
  }

  void Bookmarks::resizeTextures(const int& width, const int& height) {
    auto [vertices, indices] = getVerticalWindowedStripe(mNumPhysTilesPerFacet);
    mNumVertices = indices.size();
    float tileHeight = 2.0f/mNumPhysTilesPerFacet;
    float ratio = 1.0f*mCamera->getResolution().y/mCamera->getResolution().x;
    float tileWidth = ratio*tileHeight;
    mTileSize = glm::vec2(tileWidth, tileHeight);
    mVertexBuffer.setData(vertices);
    mVertexArray.setAttribute(0, 3, GL_FLOAT, 0*sizeof(float));
    mVertexArray.bindVertexBuffer(mVertexBuffer, 0, 3*sizeof(float));
    mElementBuffer.setData(indices);
    mVertexArray.bindElementBuffer(mElementBuffer);
    mShader.use();
    mShader.setVec2("tileSize", mTileSize);
  }

  void Bookmarks::setFixUniforms() const {
    mShader.use();
    mShader.setMat4("view", glm::mat4(1.0f));
    mShader.setMat4("projection", glm::mat4(1.0f));
    mShader.setVec2("tileSize", mTileSize);
    mShader.setFloat("prismTilePosZ", mPrismTilePosZ);

    auto tokens = mBookmarksContext->getTokens(mStage);
    if(not tokens.empty())
      mShader.setIvec3("selectedTiles", tokens, tokens.size()/3);

    auto modelParameters = mBookmarksContext->getParametersStd140(mModel, mStage);
    mModelParameters.resizeAndClear(modelParameters.size()*sizeof(typename decltype(modelParameters)::value_type));
    mModelParameters.setSubData(modelParameters);

    auto weights = mScaleByWeight ? mContext->getWeights(mModel)
                                  : std::vector<float>(mContext->numDistributions(mModel), 1.0f);
    mShader.use();
    mShader.setFloat("weights", weights, weights.size());

    auto& colors = context::Context::tmm == mModel ? mColorsStudent : mColorsGaussian;
    for(auto& color : colors) mShader.setVec3("colors", colors, colors.size()/3);
  }

  void Bookmarks::updateMetricsIf(const bool& condition) const {}

  void Bookmarks::execute() const {
      glEnable(GL_CULL_FACE);
      glFrontFace(GL_CCW);
      glCullFace(GL_BACK);
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LESS);
      //glClear(GL_DEPTH_BUFFER_BIT);
        mShader.use();
        // mShader.setFloat("camDegrees", mCamera->getCumulatedXOffset());
        mShader.setVec3("camPos", mCamera->getPosition());
        mShader.setFloat("userInput.threshold", mThreshold);
        mShader.setFloat("userInput.domainScale", mDomainScale[std::max(0, mStage-1)]);
        mShader.setFloat("userInput.codomainScale", mCodomainScale);
        mShader.setFloat("userInput.distanceminimizer", mDistributionDepth);
        mShader.setBool("userInput.adoptToStd", mAdoptToStd);
        mShader.setMat3("model", glm::mat3(mCameraTile->getViewMatrix()));

        mModelParameters.bind(GL_UNIFORM_BUFFER, 3);
        mAttributes.bindImage(6, GL_READ_ONLY);
        mFont.bind(0);

        mVertexArray.drawElements<unsigned int>(mDrawMode, mNumVertices);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        mWireShader.use();

          glEnable(GL_LINE_SMOOTH);
          //glLineWidth(1.0f);
          mVertexArray.drawElements<unsigned int>(mDrawMode, mNumVertices);
          glDisable(GL_LINE_SMOOTH);
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);
  }
  void Bookmarks::refresh() const {
    mShader.refresh();
    //mWireShader.refresh();
    setFixUniforms();
  }
  void Bookmarks::setDomainScale(const float& domainScale) {mDomainScale[std::max(0, mStage-1)] = domainScale;}
  void Bookmarks::setCodomainScale(const float& codomainScale) {mCodomainScale = codomainScale;}
  void Bookmarks::setThreshold(const float& threshold) {mThreshold = threshold;}
  void Bookmarks::setDistributionDepth(const float& distributionDepth) {mDistributionDepth = distributionDepth;}

  const context::Context::Model& Bookmarks::getModel() const {return mModel;}
  void Bookmarks::setModel(const context::Context::Model& model, const bool& update) {
    if(model == mModel) return;
    mModel = model;
    if(not update) return;
    this->update();
  }

  void Bookmarks::scaleByWeight(const bool& on) {
    if(on == mScaleByWeight) return;
    mScaleByWeight = on;
    auto weights = mScaleByWeight ? mContext->getWeights(mModel)
                                  : std::vector<float>(mContext->numDistributions(mModel), 1.0f);
    mShader.use();
    mShader.setFloat("weights", weights, weights.size());
  }

  void Bookmarks::adoptThresholdToStd(const bool& active) {
    if(active == mAdoptToStd) return;
    mAdoptToStd = active;
    float thresholdLoaded = mThresholdStored;
    mThresholdStored = mThreshold;
    mThreshold = thresholdLoaded;
  }

  void Bookmarks::setColor(const int& i, const std::array<float, 3>& color, const bool& update) {
    auto& colors = context::Context::tmm == mModel ? mColorsStudent : mColorsGaussian;
    colors[i*3+0] = color[0];
    colors[i*3+1] = color[1];
    colors[i*3+2] = color[2];
    if(not update) return;
    mShader.use();
    mShader.setVec3("colors[" + std::to_string(i) + "]", color);
  }

  const Rendering& Bookmarks::getRendering() const {return mRendering;}
  void Bookmarks::setRendering(const Rendering& rendering, const bool& update) {
    if(rendering == mRendering) return;
    mRendering = rendering;
    if(not update) return;
    this->update();
    //setFixUniforms();
  }

} // namespace pass
