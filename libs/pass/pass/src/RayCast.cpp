#include "pass/Base.hpp"
#include "pass/RayCast.hpp"

#include <memory>
#include <glm/gtx/io.hpp>

#include "context/Context.hpp"
#include "vis/Camera.hpp"
#include "glPP/Shader.hpp"
#include "glPP/Texture.hpp"
#include "glPP/Buffer.hpp"
#include "pass/Rendering.hpp"

#include "stb_image.h"

#include "glm/gtx/string_cast.hpp"

namespace pass{
  RayCast::RayCast()
  : mContext(nullptr), mCamera(nullptr), mModelParameters(), mEvalCams(0), mEvalData(0)
  {}

  void RayCast::init(const std::shared_ptr<const context::Context>& context,
                     const std::shared_ptr<const vis::Camera>& camera,
                     const glPP::Texture<GL_TEXTURE_2D>* points)
  {
    mContext = context;
    mCamera = camera;
    mPoints = points;

    mAlbedo = glPP::Texture<GL_TEXTURE_2D>(GL_RGBA16F, {mPoints->size(0), mPoints->size(1)}, GL_NEAREST, GL_CLAMP_TO_EDGE);
    mSum = glPP::Texture<GL_TEXTURE_1D>(GL_R32F, {mEvalCams.size()+1}, GL_NEAREST, GL_CLAMP_TO_EDGE);
    std::array<int, 2> selected = {-1, -1};
    mSelected = glPP::Texture<GL_TEXTURE_1D>(selected, GL_RED_INTEGER, GL_R16I, {2}, GL_NEAREST, GL_CLAMP_TO_EDGE);

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
    } else {std::cout << "[Error](RayCast::init) " << path << " not found" << std::endl;}

    mScaleByWeight = true;
    mThreshold = 2.0f;
    mThresholdStored = 0.005f;
    mAdoptToStd = true;
    mModel = context::Context::tmm;

    mColorsStudent.resize(mContext->numDistributions(context::Context::tmm)*3);
    for(int i = 0; i < mColorsStudent.size(); ++i) mColorsStudent[i] = colors[i%colors.size()];
    mColorsGaussian.resize(mContext->numDistributions(context::Context::gmm)*3);
    for(int i = 0; i < mColorsGaussian.size(); ++i) mColorsGaussian[i] = colors[i%colors.size()];
  }

  std::vector<unsigned char> RayCast::attributesToFontDigits(const std::vector<std::string>& attributes, const int& numLetters) const {
    std::vector<unsigned char> fontDigits(attributes.size()*numLetters);
    int j = 0;
    for(const auto& attribute : attributes) {
      for(int i = 0; i < numLetters; ++i) {
        fontDigits[j++] = i < attribute.size() ? attribute[i]-'a'+97 : 32;
      }
    }
    return fontDigits;
  }

  void RayCast::updateSpecs() {
    mShader.setSpecs({glPP::Shader::Specs(
      GL_COMPUTE_SHADER,
      std::string(CMAKE_SHADERDIR) + "rayCast.cs",
      "const int maxNumCams = " + std::to_string(mEvalCams.size()+1) + ";",
      "const int mixtureModel = " + std::to_string(mModel) + ";",
      "const int rendering = " + std::to_string(mRendering) + ";",
      "const int projMethod = " + std::to_string(mProjectionMethod) + ";",
      "const int numDistributions = " + std::to_string(mContext->numDistributions(mModel)) + ";",
      "const int tokenLength = " + std::to_string(mTokenLenght) + ";"
    )});
  }

  void RayCast::resizeTextures(const int& width, const int& height) {
    mAlbedo.resizeAndClear({width, height});

    mShader.use();
    mShader.setMat4("projection", mCamera->getProjectionMatrix());
  }

  void RayCast::setContext(const std::shared_ptr<const context::Context>& context) {
    mContext = context;
  }

  void RayCast::enter() {
    updateSpecs();
    refresh();
    std::cout << "[Info](RayCast::enter)" << std::endl;
    //mShader.refresh();
    //setFixUniforms();
  }

  void RayCast::setFixUniforms() const {
    mShader.use();

    auto modelParameters = mContext->getParametersStd140(mModel, 3);
    mModelParameters.resizeAndClear(modelParameters.size()*sizeof(typename decltype(modelParameters)::value_type));
    mModelParameters.setSubData(modelParameters);

    auto hight = mContext->getHight(mModel);
    auto scaleInv = mContext->getScaleInv(mModel);
    mShader.setInt("dimensions", mContext->dimensions());
    mShader.setFloat("height_alt", hight, mContext->numDistributions(mModel));
    mShader.setMat3("scaleInv_alt", scaleInv, mContext->numDistributions(mModel));
    mShader.setMat4("projection", mCamera->getProjectionMatrix());

    auto weights = mScaleByWeight ? mContext->getWeights(mModel)
                                  : std::vector<float>(mContext->numDistributions(mModel), 1.0f);
    mShader.setFloat("weights", weights, weights.size());

    auto& colors = context::Context::tmm == mModel ? mColorsStudent : mColorsGaussian;
    for(auto& color : colors) mShader.setVec3("colors", colors, colors.size()/3);
  }

  void RayCast::setNumEvalCameras(const int& numCams) {
    mEvalCams.resize(numCams);
    updateSpecs();
    refresh();
    //mShader.refresh();
    //setFixUniforms();
    mSum.resizeAndClear({mEvalCams.size()+1});
  }

  void RayCast::setEvalCamera(const int& index, const glm::vec3& position, const glm::mat4& view) {
    mEvalCams[index].position = position;
    mEvalCams[index].view = view;
  }

  const glm::vec3& RayCast::getEvalCameraPosition(const int& index) const {
    return mEvalCams[index].position;
  }

  void RayCast::setCursor(const int& xPos, const int& yPos) {
    mCursor = glm::ivec2(xPos, mPoints->size(1)-yPos);
  }

  void RayCast::execute() const {
    mShader.use();

    mShader.setIvec2("cursor", mCursor);
    mShader.setVec3("camPos", mCamera->getPosition());
    mShader.setMat4("view", mCamera->getViewMatrix());
    mShader.setInt("numCams", mEvalCams.size()+1);
    for(int i = 0; i < mEvalCams.size(); ++i) {
      mShader.setVec3("camPos[" + std::to_string(i+1) + "]", mEvalCams[i].position);
      mShader.setMat4("view[" + std::to_string(i+1) + "]", mEvalCams[i].view);
    }
    mShader.setFloat("userInput.threshold", mThreshold);
    mShader.setBool("userInput.adoptToStd", mAdoptToStd);
    mShader.setIvec3("selectedTiles", mContext->getState(3), 3);

    mModelParameters.bind(GL_UNIFORM_BUFFER, 3);

    mShader.setFloat("camPosTMinusMeanT_ScaleInv_camPosMinusMean[" + std::to_string(0) + "]",
                     mContext->getCamPosTMinusMeanT_ScaleInv_camPosMinusMean(mModel, mCamera->getPosition()), mContext->numDistributions(mModel));
    mShader.setVec3("scaleInv_camPosMinusMean[" + std::to_string(0) + "]",
                    mContext->getScaleInv_camPosMinusMean(mModel, mCamera->getPosition()), mContext->numDistributions(mModel));

    for(int j = 0; j < mEvalCams.size(); ++j) {
      //index = i*(mEvalCams.size()+1) + j+1;
      mShader.setFloat("camPosTMinusMeanT_ScaleInv_camPosMinusMean[" + std::to_string(j+1) + "]",
                       mContext->getCamPosTMinusMeanT_ScaleInv_camPosMinusMean(mModel, mCamera->getPosition()), mContext->numDistributions(mModel));
      mShader.setVec3("scaleInv_camPosMinusMean[" + std::to_string(j+1) + "]",
                      mContext->getScaleInv_camPosMinusMean(mModel, mCamera->getPosition()), mContext->numDistributions(mModel));
      }

    //static bool firstIteration = true;
    //if(firstIteration) {
      //std::cout << "camPosTMinusMeanT_ScaleInv_camPosMinusMean = " << mContext->camPosTMinusMeanT_ScaleInv_camPosMinusMean(mCamera->getPosition(), 0) << std::endl;
      //std::cout << "viewBoxTScaleInv_meanMinusCamPos\n" << mContext->viewBoxTScaleInv_meanMinusCamPos(mCamera->getPosition(), 0) << std::endl;
      //firstIteration = false;
    //}

    mPoints->bindImage(0, GL_READ_ONLY);
    mAlbedo.bindImage(1, GL_WRITE_ONLY);
    mSum.bindImage(2, GL_READ_WRITE);
    mSelected.bindImage(3, GL_WRITE_ONLY);
    mAttributes.bindImage(6, GL_READ_ONLY);

    mFont.bind(0);

    glDispatchCompute(std::ceil(mAlbedo.size(0)/16), std::ceil(mAlbedo.size(1)/16), 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    try{
      mEvalData = mSum.getImage<Eigen::VectorXf>();
      mSum.clearImage();
      static int i = 0;
      if(++i == 200) {
        i = 0;
        //for(const auto& elem : mEvalData) std::cout << elem << " ";
        //std::cout << std::endl;
      }
    } catch(std::string msg) {
      std::cout << msg << std::endl;
    }
  }

  std::array<int, 2> RayCast::getSelectedDistributionAndPoint() {
    if(-1 == mCursor.x) return {-1, -1};

      //std::cout << "[Info] (RayCast::getSelectedDistributionAndPoint) Cursor = " << mCursor.x << ", " << mCursor.y << std::endl;
      mCursor = glm::ivec2(-1, -1);
      //std::cout << glPP::getAllErrors() << std::endl;
      auto disPoint = mSelected.getImage<std::vector<int>>();
      //std::cout << glPP::getAllErrors() << std::endl;
      try{
         std::array<int, 2> clear = {-1, -1};
         mSelected.setImage(clear);
       } catch(const std::string& err) {
         std::cout << err << std::endl;
      }
      //std::cout << glPP::getAllErrors() << std::endl;
      //std::cout << "[Info] (RayCast::execute) index, distribution = " << disPoint[1] << ", " << disPoint[0] << std::endl;
      //auto values = mContext->getPointValues(disPoint[1]);
      //for(int i = 0; i < values.size(); ++i) {
      //  std::cout << mContext->getAttributes()[i] << " = " << values[i] << std::endl;
      //}
    return {disPoint[0], disPoint[1]};

  }

  void RayCast::setThreshold(const float& threshold) {mThreshold = threshold;}

  const Rendering& RayCast::getRendering() const {return mRendering;}
  void RayCast::setRendering(const Rendering& rendering, const bool& update) {
    if(rendering == mRendering) return;
    mRendering = rendering;
    if(not update) return;
    updateSpecs();
    refresh();
  }
  const context::Context::Model& RayCast::getModel() const {return mModel;}
  void RayCast::setModel(const context::Context::Model& model, const bool& update) {
    if(model == mModel) return;
    mModel = model;
    if(not update) return;
    updateSpecs();
    auto modelParameters = mContext->getParametersStd140(mModel, 3);
    mModelParameters.resizeAndClear(modelParameters.size()*sizeof(typename decltype(modelParameters)::value_type));
    mModelParameters.setSubData(modelParameters);
    refresh();
  }

  const ProjectionMethod& RayCast::getProjectionMethod() const {return mProjectionMethod;}
  void RayCast::setProjectionMethod(const ProjectionMethod& projectionMethod, const bool& update) {
    if(projectionMethod == mProjectionMethod) return;
    mProjectionMethod = projectionMethod;
    if(not update) return;
    updateSpecs();
    refresh();
  }

  void RayCast::scaleByWeight(const bool& on) {
    if(on == mScaleByWeight) return;
    mScaleByWeight = on;
    auto weights = mScaleByWeight ? mContext->getWeights(mModel)
                                  : std::vector<float>(mContext->numDistributions(mModel), 1.0f);
    mShader.use();
    mShader.setFloat("weights", weights, weights.size());
  }

  void RayCast::adoptThresholdToStd(const bool& active) {
    if(active == mAdoptToStd) return;
    mAdoptToStd = active;
    float thresholdLoaded = mThresholdStored;
    mThresholdStored = mThreshold;
    mThreshold = thresholdLoaded;
  }

  void RayCast::setColor(const int& i, const std::array<float, 3>& color, const bool& update) {
    auto& colors = context::Context::tmm == mModel ? mColorsStudent : mColorsGaussian;
    colors[i*3+0] = color[0];
    colors[i*3+1] = color[1];
    colors[i*3+2] = color[2];
    if(not update) return;
    mShader.use();
    mShader.setVec3("colors[" + std::to_string(i) + "]", color);
  }

  const glPP::Texture<GL_TEXTURE_2D>& RayCast::getAlbedo() const {return mAlbedo;}
  const Eigen::VectorXf& RayCast::evalData() const {return mEvalData;}

} // namespace pass
