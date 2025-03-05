#ifndef PASS_BOOKMARKS_HPP
#define PASS_BOOKMARKS_HPP

#include "pass_Base.hpp"

#include <memory>
#include <cmath>
#include <array>

#include "Context.hpp"
#include "context_Bookmarks.hpp"
#include "vis_Camera.hpp"
#include "gl_Shader.hpp"
#include "gl_Texture.hpp"
#include "gl_Buffer.hpp"
#include "gl_VertexArray.hpp"
#include "gl_Framebuffer.hpp"
#include "pass_Rendering.hpp"

namespace pass{
  class Bookmarks : public Base{
  public:
    Bookmarks();
    void init(const std::shared_ptr<Context>& context,
              const std::shared_ptr<context::Bookmarks>& bookmarksContext,
              const std::shared_ptr<const vis::Camera>& camera,
              const std::shared_ptr<vis::Camera>& cameraTile,
              const float& prismTilePosZ);

    void update();
    void enter(const int& stage);
    void resizeTextures(const int& width, const int& height) override;
    void setFixUniforms() const override;
    void execute() const override;
    void refresh() const override;

    void addBookmark(const double& xPos, const double& yPos);
    void removeBookmark(const double& xPos, const double& yPos);

    const gl::Texture<GL_TEXTURE_2D>& getBookmarks() const;

    void setDomainScale(const float& domainScale);
    void setCodomainScale(const float& codomainScale);
    void setThreshold(const float& threshold);
    void setDistributionDepth(const float& distributionDepth);
    const Context::Model& getModel() const;
    void setModel(const Context::Model& model, const bool& update);
    void scaleByWeight(const bool& on);
    void adoptThresholdToStd(const bool& active);
    void setColor(const int& i, const std::array<float, 3>& color, const bool& update);
    const Rendering& getRendering() const;
    void setRendering(const Rendering& rendering, const bool& update);
  private:
    void updateMetricsIf(const bool& condition) const;
    std::vector<unsigned char> attributesToFontDigits(const std::vector<std::string>& attributes, const int& numLetters) const;

    int mStage;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<context::Bookmarks> mBookmarksContext;
    std::shared_ptr<const vis::Camera> mCamera;
    std::shared_ptr<const vis::Camera> mCameraTile;

    std::array<float, 2> mDomainScale;
    float mCodomainScale;
    float mThreshold;
    float mThresholdStored;
    bool mAdoptToStd;
    bool mScaleByWeight;
    float mDistributionDepth;

    gl::Shader mWireShader;
    GLenum mDrawMode = GL_TRIANGLES;
    gl::Buffer mVertexBuffer;
    gl::Buffer mElementBuffer;
    gl::VertexArray mVertexArray;
    int mNumVertices;

    glm::vec2 mTileSize;
    int mNumPhysTilesPerFacet;
    float mPrismTilePosZ;

    mutable gl::Buffer mModelParameters;

    mutable bool mUpdateMetrics = false;
    mutable gl::Texture<GL_TEXTURE_2D> mSoftMax;
    mutable gl::Texture<GL_TEXTURE_3D> mOverlap;
    gl::Texture<GL_TEXTURE_2D> mSum;
    gl::Texture<GL_TEXTURE_2D> mSumSwap;
    mutable int mSwap = 0;

    gl::Texture<GL_TEXTURE_1D> mAttributes;
    gl::Texture<GL_TEXTURE_2D> mFont;

    int mTokenLenght;

    Context::Model mModel = Context::tmm;
    Rendering mRendering = mip;
    std::vector<float> mColorsStudent;
    std::vector<float> mColorsGaussian;

    inline std::tuple<std::vector<float>, std::vector<unsigned int>> getVerticalWindowedStripe(const int& numWindows = 5);
  }; // class Bookmarks

  // input: vertical Lines as 2 Points, windows per face
  inline std::tuple<std::vector<float>, std::vector<unsigned int>> Bookmarks::getVerticalWindowedStripe(const int& numWindows) {
    constexpr int pointSize = 3;
    float rightBorder = 1.0f;
    float upperBorder = 1.0f;
    float lowerBorder = -1.0f;
    float height = (upperBorder-lowerBorder)/numWindows;
    float ratio = 1.0f*mCamera->getResolution().y/mCamera->getResolution().x;
    float width = ratio*height;
    std::vector<float> vertices;
    for(int row = 0; row < numWindows+1; ++row) {
      vertices.insert(vertices.end(), {rightBorder-width, upperBorder-row*height, 0.0f});
      vertices.insert(vertices.end(), {rightBorder, upperBorder-row*height, 0.0f});
    }

    std::vector<unsigned int> indices;
    for(unsigned int row = 0; row < numWindows; ++row) {
      indices.insert(indices.end(), {row*2 + 1, row*2 + 0, (row+1)*2 + 0});
      indices.insert(indices.end(), {(row+1)*2 + 0, (row+1)*2 + 1, row*2 + 1});
    }

    return {vertices, indices};
  }
} // namespace pass

#endif // PASS_BOOKMARKS_HPP
