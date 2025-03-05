#ifndef PASS_RAYCAST_HPP
#define PASS_RAYCAST_HPP

#include "pass_Base.hpp"

#include <memory>
#include <array>

#include "Context.hpp"
#include "vis_Camera.hpp"
#include "gl_Shader.hpp"
#include "gl_Texture.hpp"
#include "gl_Buffer.hpp"
#include "pass_Rendering.hpp"

namespace pass{
  class RayCast : public Base{
  public:
    //RayCast(const std::shared_ptr<const vis::Camera>& camera, const int& textureWidth, const int& textureHeight);
    RayCast();
    void init(const std::shared_ptr<const Context>& context,
              const std::shared_ptr<const vis::Camera>& camera,
              const gl::Texture<GL_TEXTURE_2D>* points);

    std::vector<unsigned char> attributesToFontDigits(const std::vector<std::string>& attributes, const int& numLetters) const;
    void resizeTextures(const int& width, const int& height) override;
    void setContext(const std::shared_ptr<const Context>& context);
    void enter();
    void updateSpecs();
    void setFixUniforms() const override;
    void execute() const override;

    std::array<int, 2> getSelectedDistributionAndPoint();

    const Rendering& getRendering() const;
    void setRendering(const Rendering& rendering, const bool& update);
    const Context::Model& getModel() const;
    void setModel(const Context::Model& model, const bool& update);
    const ProjectionMethod& getProjectionMethod() const;
    void scaleByWeight(const bool& on);
    void adoptThresholdToStd(const bool& active);
    void setColor(const int& i, const std::array<float, 3>& color, const bool& update);
    void setProjectionMethod(const ProjectionMethod& projectionMethod, const bool& update);
    void setThreshold(const float& threshold);

    const gl::Texture<GL_TEXTURE_2D>& getAlbedo() const;

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

    std::shared_ptr<const Context> mContext;
    std::shared_ptr<const vis::Camera> mCamera;
    const gl::Texture<GL_TEXTURE_2D>* mPoints;

    mutable gl::Buffer mModelParameters;
    bool mScaleByWeight;
    bool mAdoptToStd;
    float mThreshold;
    float mThresholdStored;

    gl::Texture<GL_TEXTURE_2D> mFont;
    gl::Texture<GL_TEXTURE_1D> mAttributes;

    int mTokenLenght;

    Rendering mRendering = mip;
    Context::Model mModel = Context::tmm;
    std::vector<float> mColorsStudent;
    std::vector<float> mColorsGaussian;
    ProjectionMethod mProjectionMethod = projectDown;
    mutable glm::ivec2 mCursor = glm::ivec2(-1, -1);

    gl::Texture<GL_TEXTURE_2D> mAlbedo;
    gl::Texture<GL_TEXTURE_1D> mSum;
    gl::Texture<GL_TEXTURE_1D> mSelected;

    std::vector<EvalCamera> mEvalCams;
    mutable Eigen::VectorXf mEvalData;
  }; // class Raycast
} // namespace pass

#endif // PASS_RAYCAST_HPP
