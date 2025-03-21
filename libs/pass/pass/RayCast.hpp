#ifndef PASS_RAYCAST_HPP
#define PASS_RAYCAST_HPP

#include "pass/Base.hpp"

#include <memory>
#include <array>

#include "context/Context.hpp"
#include "vis/Camera.hpp"
#include "glPP/Shader.hpp"
#include "glPP/Texture.hpp"
#include "glPP/Buffer.hpp"
#include "pass/Rendering.hpp"

namespace pass{
  class RayCast : public Base{
  public:
    //RayCast(const std::shared_ptr<const vis::Camera>& camera, const int& textureWidth, const int& textureHeight);
    RayCast();
    void init(const std::shared_ptr<const context::Context>& context,
              const std::shared_ptr<const vis::Camera>& camera,
              const glPP::Texture<GL_TEXTURE_2D>* points);

    std::vector<unsigned char> attributesToFontDigits(const std::vector<std::string>& attributes, const int& numLetters) const;
    void resizeTextures(const int& width, const int& height) override;
    void setContext(const std::shared_ptr<const context::Context>& context);
    void enter();
    void updateSpecs();
    void setFixUniforms() const override;
    void execute() const override;

    std::array<int, 2> getSelectedDistributionAndPoint();

    const Rendering& getRendering() const;
    void setRendering(const Rendering& rendering, const bool& update);
    const context::Context::Model& getModel() const;
    void setModel(const context::Context::Model& model, const bool& update);
    const ProjectionMethod& getProjectionMethod() const;
    void scaleByWeight(const bool& on);
    void adoptThresholdToStd(const bool& active);
    void setColor(const int& i, const std::array<float, 3>& color, const bool& update);
    void setProjectionMethod(const ProjectionMethod& projectionMethod, const bool& update);
    void setThreshold(const float& threshold);

    const glPP::Texture<GL_TEXTURE_2D>& getAlbedo() const;

    void setCursor(const int& xPos, const int& yPos);
    void setNumEvalCameras(const int& numCams);
    void setEvalCamera(const int& index, const glm::vec3& position, const glm::mat4& view);
    const glm::vec3& getEvalCameraPosition(const int& index) const;
    const Eigen::VectorXf& evalData() const;
  private:
    struct EvalCamera{
      glm::vec3 position;
      glm::mat4 view;
    };

    std::shared_ptr<const context::Context> mContext;
    std::shared_ptr<const vis::Camera> mCamera;
    const glPP::Texture<GL_TEXTURE_2D>* mPoints;

    mutable glPP::Buffer mModelParameters;
    bool mScaleByWeight;
    bool mAdoptToStd;
    float mThreshold;
    float mThresholdStored;

    glPP::Texture<GL_TEXTURE_2D> mFont;
    glPP::Texture<GL_TEXTURE_1D> mAttributes;

    int mTokenLenght;

    Rendering mRendering = mip;
    context::Context::Model mModel = context::Context::tmm;
    std::vector<float> mColorsStudent;
    std::vector<float> mColorsGaussian;
    ProjectionMethod mProjectionMethod = projectDown;
    mutable glm::ivec2 mCursor = glm::ivec2(-1, -1);

    glPP::Texture<GL_TEXTURE_2D> mAlbedo;
    glPP::Texture<GL_TEXTURE_1D> mSum;
    glPP::Texture<GL_TEXTURE_1D> mSelected;

    std::vector<EvalCamera> mEvalCams;
    mutable Eigen::VectorXf mEvalData;
  }; // class Raycast
} // namespace pass

#endif // PASS_RAYCAST_HPP
