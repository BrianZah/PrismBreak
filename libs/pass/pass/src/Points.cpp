#include "pass/Base.hpp"
#include "pass/Points.hpp"

#include <memory>
#include <glm/gtx/io.hpp>

#include "context/Context.hpp"
#include "vis/Camera.hpp"
#include "glPP/Shader.hpp"
#include "glPP/Texture.hpp"
#include "glPP/Buffer.hpp"
#include "glPP/Framebuffer.hpp"

namespace pass{
  Points::Points()
  : mContext(nullptr), mCamera(nullptr),
    mPointsBuffer(), mPointsVa(), mPointsFbo(), mPointsImage(), mPointsDepth(), mPointProbabilities()
  {
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
  }

  void Points::init(const std::shared_ptr<const context::Context>& context,
                    const std::shared_ptr<const vis::Camera>& camera)
  {
    mContext = context;
    mCamera = camera;

    mPointsImage = glPP::Texture<GL_TEXTURE_2D>(GL_RGBA16F, {mCamera->getResolution().x, mCamera->getResolution().y}, GL_NEAREST, GL_CLAMP_TO_EDGE);
    mPointsDepth = glPP::Renderbuffer(GL_DEPTH24_STENCIL8, mCamera->getResolution().x, mCamera->getResolution().y);
    mPointsFbo.attach(mPointsImage, GL_COLOR_ATTACHMENT0);
    mPointsFbo.attach(mPointsDepth, GL_DEPTH_STENCIL_ATTACHMENT);

    mModel = context::Context::tmm;
    mEncoding = std;
    mStdRange = {0.0f, 20.0f};
    mProbabilityRange = {0.0f, 100.0f};

    //updateSpecs();
    //mShader.refresh();
    //setFixUniforms();
  }

  void Points::updateSpecs() {
    mShader.setSpecs({
      glPP::Shader::Specs(GL_VERTEX_SHADER, std::string(CMAKE_SHADERDIR) + "points.vs",
                        "const int numDistributions = " + std::to_string(mContext->numDistributions(mModel)) + ";"),
      glPP::Shader::Specs(GL_FRAGMENT_SHADER, std::string(CMAKE_SHADERDIR) + "points.fs",
                        "const int numDistributions = " + std::to_string(mContext->numDistributions(mModel)) + ";")
    });
  }

  void Points::enter() {
    updateSpecs();
    mShader.refresh();
    setFixUniforms();

    mPointProbabilities.setData(mContext->getPointProbabilities(mModel));
    mPointsBuffer.setData(mContext->dataAs3DPoints(mModel));
    mPointsVa.setAttribute(0, 3, GL_FLOAT, 0);
    mPointsVa.setAttribute(1, 1, GL_FLOAT, 3*sizeof(float));
    mPointsVa.setAttribute(2, 1, GL_FLOAT, 4*sizeof(float));
    mPointsVa.bindVertexBuffer(mPointsBuffer, 0, 5*sizeof(float));
  }

  void Points::resizeTextures(const int& width, const int& height) {
    mPointsImage.resizeAndClear({width, height});
    mPointsDepth = glPP::Renderbuffer(GL_DEPTH24_STENCIL8, width, height);
    mPointsFbo.attach(mPointsImage, GL_COLOR_ATTACHMENT0);
    mPointsFbo.attach(mPointsDepth, GL_DEPTH_STENCIL_ATTACHMENT);

    mShader.use();
    mShader.setMat4("projection", mCamera->getProjectionMatrix());
    mShader.setIvec2("windowSize", mPointsImage.size(0), mPointsImage.size(1));
  }
  void Points::setFixUniforms() const {
    mShader.use();
    mShader.setMat4("projection", mCamera->getProjectionMatrix());
    mShader.setIvec2("windowSize", mPointsImage.size(0), mPointsImage.size(1));
    mShader.setUint("encoding", mEncoding);
  }
  void Points::execute() const {
    mShader.use();
    mShader.setMat4("view", mCamera->getViewMatrix());
    mShader.setVec2("range", std == mEncoding ? mStdRange : mProbabilityRange);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    mPointsFbo.bind();
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      mPointProbabilities.bind(GL_SHADER_STORAGE_BUFFER, 1);
      mPointsVa.drawArrays(GL_POINTS, 0, mContext->numPoints());
    mPointsFbo.unbind();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  void Points::setModel(const context::Context::Model& model, const bool& update) {
    if(model == mModel) return;
    mModel = model;
    if(not update) return;
    enter();
  }

  Points::Encoding Points::getEncoding() const {return mEncoding;}
  void Points::setEncoding(const Encoding& encoding) {
    mEncoding = encoding;
    mShader.use();
    mShader.setUint("encoding", mEncoding);
  }

  std::array<float, 2>& Points::refRange() {return std == mEncoding ? mStdRange : mProbabilityRange;}

  const glPP::Texture<GL_TEXTURE_2D>& Points::getPoints() const {
    return mPointsImage;
  }

} // namespace pass
