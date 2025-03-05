#ifndef PASS_POINTS_HPP
#define PASS_POINTS_HPP

#include "pass_Base.hpp"

#include <memory>
#include <array>

#include "Context.hpp"
#include "vis_Camera.hpp"
#include "gl_Shader.hpp"
#include "gl_Texture.hpp"
#include "gl_Buffer.hpp"
#include "gl_VertexArray.hpp"
#include "gl_Framebuffer.hpp"

namespace pass{
  class Points : public Base{
  public:
    enum Encoding{std = 0, probabilities = 1};
    Points();
    void init(const std::shared_ptr<const Context>& context,
              const std::shared_ptr<const vis::Camera>& camera);
    void updateSpecs();
    void enter();
    void setContext(const std::shared_ptr<const Context>& context);
    void resizeTextures(const int& width, const int& height) override;
    void setFixUniforms() const override;
    void execute() const override;
    void setModel(const Context::Model& model, const bool& update);
    Encoding getEncoding() const;
    void setEncoding(const Encoding& encoding);
    std::array<float, 2>& refRange();

    const gl::Texture<GL_TEXTURE_2D>& getPoints() const;
  private:
    std::shared_ptr<const Context> mContext;
    std::shared_ptr<const vis::Camera> mCamera;

    gl::Buffer mPointProbabilities;
    gl::Buffer mPointsBuffer;
    gl::VertexArray mPointsVa;
    std::array<float, 4> mBackground = {0.0f, 0.0f, 0.0f, 0.0f};
    gl::Framebuffer mPointsFbo;
    gl::Texture<GL_TEXTURE_2D> mPointsImage;
    gl::Renderbuffer mPointsDepth;

    Context::Model mModel;
    Encoding mEncoding;
    std::array<float, 2> mStdRange;
    std::array<float, 2> mProbabilityRange;
  }; // class Points
} // namespace pass

#endif // PASS_POINTS_HPP
