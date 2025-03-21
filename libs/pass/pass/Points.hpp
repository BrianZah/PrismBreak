#ifndef PASS_POINTS_HPP
#define PASS_POINTS_HPP

#include "pass/Base.hpp"

#include <memory>
#include <array>

#include "context/Context.hpp"
#include "vis/Camera.hpp"
#include "glPP/Shader.hpp"
#include "glPP/Texture.hpp"
#include "glPP/Buffer.hpp"
#include "glPP/VertexArray.hpp"
#include "glPP/Framebuffer.hpp"

namespace pass{
  class Points : public Base{
  public:
    enum Encoding{std = 0, probabilities = 1};
    Points();
    void init(const std::shared_ptr<const context::Context>& context,
              const std::shared_ptr<const vis::Camera>& camera);
    void updateSpecs();
    void enter();
    void setContext(const std::shared_ptr<const context::Context>& context);
    void resizeTextures(const int& width, const int& height) override;
    void setFixUniforms() const override;
    void execute() const override;
    void setModel(const context::Context::Model& model, const bool& update);
    Encoding getEncoding() const;
    void setEncoding(const Encoding& encoding);
    std::array<float, 2>& refRange();

    const glPP::Texture<GL_TEXTURE_2D>& getPoints() const;
  private:
    std::shared_ptr<const context::Context> mContext;
    std::shared_ptr<const vis::Camera> mCamera;

    glPP::Buffer mPointProbabilities;
    glPP::Buffer mPointsBuffer;
    glPP::VertexArray mPointsVa;
    std::array<float, 4> mBackground = {0.0f, 0.0f, 0.0f, 0.0f};
    glPP::Framebuffer mPointsFbo;
    glPP::Texture<GL_TEXTURE_2D> mPointsImage;
    glPP::Renderbuffer mPointsDepth;

    context::Context::Model mModel;
    Encoding mEncoding;
    std::array<float, 2> mStdRange;
    std::array<float, 2> mProbabilityRange;
  }; // class Points
} // namespace pass

#endif // PASS_POINTS_HPP
