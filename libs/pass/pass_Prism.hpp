#ifndef PASS_PRISM_HPP
#define PASS_PRISM_HPP

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
  class Prism : public Base{
  public:
    enum Metric{variance = 0, sparsity = 1, visibility = 2};
  public:
    Prism();
    void init(const std::shared_ptr<Context>& context,
              const std::shared_ptr<context::Bookmarks>& bookmarksContext,
              const std::shared_ptr<const vis::Camera>& camera,
              const std::shared_ptr<vis::Camera>& cameraTile);
    void updateSpecs();
    void prepareBuffersAndTextures();
    std::vector<unsigned char> attributesToFontDigits(const std::vector<std::string>& attributes, const int& numLetters) const;

    void resizeTextures(const int& width, const int& height) override;
    void setFixUniforms() const override;
    void execute() const override;

    std::array<int, 3> getUIElementSetAndCompAndIndex();
    void setCursorPos(const std::array<double, 2>& cursorPos);

    const int& getStage() const;
    int enterNextStage();
    int enter();
    int enterPreviousStage();
    void updateBookmarks();

    const Metric& getMetric() const;
    void sortTiles(const Metric& metric);

    const Rendering& getRendering() const;
    void setRendering(const Rendering& rendering, const bool& update);

    float& refThreshold();
    const float& getThreshold() const;

    float& refDomainScale();
    const float& getDomainScale() const;

    float& refCodomainScale();
    const float& getCodomainScale() const;

    float& refDistributionDepth();
    const float& getDistributionDepth() const;

    const float& getFrontFacePosZ() const;

    const Context::Model& getModel() const;
    void setModel(const Context::Model& model, const bool& update);

    void scaleByWeight(const bool& on);
    void adoptThresholdToStd(const bool& active);

    std::array<float, 3> getColor(const int& i) const;
    void setColor(const int& i, const std::array<float, 3>& color, const bool& update);
  private:
    class Visibility{
    public:
      std::vector<float> get() const;
    };
    friend class Visibility;

    std::vector<float> getViewPointEntropy(const Eigen::VectorXf& evalData, const int& area) const;
    std::vector<float> getSumSoftMax(const Eigen::VectorXf& evalData, const Eigen::VectorXi& numPixels) const;
    std::vector<float> getSumSoftMax(const Eigen::VectorXf& evalData, const int& numPixels) const;
    //void updateVisibilityMetric() const;
    void updateVisibility() const;

    static constexpr float sScalar = 20.0f;
    std::shared_ptr<Context> mContext;
    std::shared_ptr<context::Bookmarks> mBookmarksContext;
    std::shared_ptr<const vis::Camera> mCamera;
    std::shared_ptr<const vis::Camera> mCameraTile;

    int mNumVertices;
    gl::Buffer mVertexBuffer;
    gl::VertexArray mVertexArray;

    gl::Shader mWireShader;
    GLenum mDrawMode = GL_TRIANGLES;
    int mNumPhysTilesPerFacet;
    float mTileSize;
    int mStage;

    gl::Buffer mUboNormals;
    gl::Buffer mUboTileCenters;
    mutable gl::Buffer mModelParameters;
    bool mScaleByWeight;

    Visibility mVisibility;

    Metric mMetric = variance;
    Rendering mRendering = mip;
    mutable gl::Texture<GL_TEXTURE_1D> mIndexArray;
    std::vector<int> mEigenvalueIndices;
    std::vector<int> mSparsityIndices;
    mutable std::vector<int> mSoftMaxIndices;
    glm::ivec2 mCursorPos;

    gl::Texture<GL_TEXTURE_2D> mFont;
    gl::Texture<GL_TEXTURE_1D> mBookmarks;
    gl::Texture<GL_TEXTURE_2D> mComponents;
    gl::Texture<GL_TEXTURE_1D> mAttributes;
    mutable gl::Texture<GL_TEXTURE_2D> mMetrics;
    mutable gl::Texture<GL_TEXTURE_3D> mOverlap;
    mutable gl::Texture<GL_TEXTURE_1D> mNumPixels;
    mutable gl::Texture<GL_TEXTURE_2D> mOverlapNew;
    gl::Texture<GL_TEXTURE_1D> mUIElement;

    mutable bool mUpdateVisibilityMetric = false;
    int mNumMetrics;
    int mTokenLenght;

    std::array<float, 2> mDomainScale;
    float mThreshold;
    float mThresholdStored;
    float mCodomainScale;
    bool mAdoptToStd;
    float mDistributionDepth;
    float mFrontFacePosZ;
    Context::Model mModel = Context::tmm;
    std::vector<float> mColorsStudent;
    std::vector<float> mColorsGaussian;

    constexpr std::array<float, 6*2*3> hexagonalPrismInSphere(const float& rectangleRatio = 1.0f, const float& sphereRadius = 1.0f);
    constexpr std::array<int, 2> gridSize(const int& length);
    template<class Array>
    constexpr std::vector<float> generateWindowedRing_alt(const Array& ring, const int& rowsPerFace, const int& colsPerFace);
    template<int numFaces, class Array>
    constexpr std::array<float, 4*numFaces> getFaceNormals(const Array& ring) const;
    template<class Iterator>
    inline std::vector<float> getTileCenters(Iterator begin, Iterator end) const;
  }; // class Prism

  constexpr std::array<float, 6*2*3> Prism::hexagonalPrismInSphere(const float& rectangleRatio, const float& sphereRadius) {
    float y = std::sqrt(sphereRadius*sphereRadius/(4*rectangleRatio*rectangleRatio+1));
    float x0 = std::sqrt(sphereRadius*sphereRadius - y*y);
    float x1 = rectangleRatio*y;
    float z = std::sqrt(sphereRadius*sphereRadius - y*y - x1*x1);
    return {-x0, y,  0, -x0, -y,  0,
            -x1, y,  z, -x1, -y,  z,
             x1, y,  z,  x1, -y,  z,
             x0, y,  0,  x0, -y,  0,
             x1, y, -z,  x1, -y, -z,
            -x1, y, -z, -x1, -y, -z};
  }

  constexpr std::array<int, 2> Prism::gridSize(const int& length) {
    return {int(std::round(std::sqrt(length))), int(std::ceil(std::sqrt(length)))};
  }

  // input: vertical Lines as 2 Points, windows per face
  template<class Array>
  constexpr std::vector<float> Prism::generateWindowedRing_alt(const Array& ring, const int& rowsPerFace, const int& colsPerFace) {
    constexpr int pointSize = 3;
    int numFaces = ring.size()/(2*pointSize);
    std::vector<float> vertices;

    for(int i = 0; i < numFaces; ++i) {
      std::array<float, 3> p0 = {ring[i*pointSize*2], ring[i*pointSize*2+1], ring[i*pointSize*2+2]};
      std::array<float, 3> p1 = {ring[i*pointSize*2+3], ring[i*pointSize*2+4], ring[i*pointSize*2+5]};
      std::array<float, 3> p2 = {ring[(i+1)%numFaces*pointSize*2], ring[(i+1)%numFaces*pointSize*2+1], ring[(i+1)%numFaces*pointSize*2+2]};
      std::array<float, 3> p3 = {ring[(i+1)%numFaces*pointSize*2+3], ring[(i+1)%numFaces*pointSize*2+4], ring[(i+1)%numFaces*pointSize*2+5]};
      auto interpolate = [&p0, &p1, &p2, &p3](const float& a, const float& b) -> std::array<float, 3> {
        return { (1.0f-a)*((1.0f-b)*p0[0] + b*p1[0]) + a*((1.0f-b)*p2[0] + b*p3[0]),
                 (1.0f-a)*((1.0f-b)*p0[1] + b*p1[1]) + a*((1.0f-b)*p2[1] + b*p3[1]),
                 (1.0f-a)*((1.0f-b)*p0[2] + b*p1[2]) + a*((1.0f-b)*p2[2] + b*p3[2]) };
      };

      for(int row = 0; row < rowsPerFace; ++row) {
        float b0 = float(row)/rowsPerFace;
        float b1 = float(row+1)/rowsPerFace;
        std::array<float, 3> q0 = interpolate(0.0f, b0);
        std::array<float, 3> q1 = interpolate(0.0f, b1);
        for(int col = 0; col < colsPerFace; ++col) {
          float a = float(col+1)/colsPerFace;
          std::array<float, 3> q2 = interpolate(a, b0);
          std::array<float, 3> q3 = interpolate(a, b1);

          vertices.insert(vertices.end(), q0.begin(), q0.end());
          vertices.insert(vertices.end(), q1.begin(), q1.end());
          vertices.insert(vertices.end(), q3.begin(), q3.end());

          vertices.insert(vertices.end(), q3.begin(), q3.end());
          vertices.insert(vertices.end(), q2.begin(), q2.end());
          vertices.insert(vertices.end(), q0.begin(), q0.end());

          q0 = q2;
          q1 = q3;
        }
      }
    }
    return vertices;
  }

  template<int numFaces, class Array>
  constexpr std::array<float, 4*numFaces> Prism::getFaceNormals(const Array& ring) const {
    std::array<float, 4*numFaces> normals;
    for(int i = 0; i < numFaces; ++i) {
      std::array<float, 3> p0 = {ring[i*3*2], ring[i*3*2+1], ring[i*3*2+2]};
      std::array<float, 3> p1 = {ring[i*3*2+3], ring[i*3*2+4], ring[i*3*2+5]};
      std::array<float, 3> p2 = {ring[(i+1)%numFaces*3*2], ring[(i+1)%numFaces*3*2+1], ring[(i+1)%numFaces*3*2+2]};

      std::array<float, 3> v0 = {p1[0]-p0[0], p1[1]-p0[1], p1[2]-p0[2]};
      std::array<float, 3> v1 = {p2[0]-p0[0], p2[1]-p0[1], p2[2]-p0[2]};

      std::array<float, 3> v2 = {v0[1]*v1[2]-v0[2]*v1[1], v0[2]*v1[0]-v0[0]*v1[2], v0[0]*v1[1]-v0[1]*v1[0]};
      float length = std::sqrt(v2[0]*v2[0] + v2[1]*v2[1] + v2[2]*v2[2]);
      normals[i*4 + 0] = v2[0]/length;
      normals[i*4 + 1] = v2[1]/length;
      normals[i*4 + 2] = v2[2]/length;
      normals[i*4 + 3] = 0;
    }
    return normals;
  }

  template<class Iterator>
  inline std::vector<float> Prism::getTileCenters(Iterator begin, Iterator end) const {
    const int pointsPerTile = 6;
    std::vector<float> centers;
    centers.reserve(4*std::distance(begin, end)/(pointsPerTile*3));
    for(; begin < end; begin+= 3*pointsPerTile) {
      centers.insert(centers.end(), {0.25f*(begin[0]+begin[3]+begin[12]+begin[6]),
                                     0.25f*(begin[1]+begin[4]+begin[13]+begin[7]),
                                     0.25f*(begin[2]+begin[5]+begin[14]+begin[8]),
                                     0.0f});
    }
    return centers;
  }
} // namespace pass

#endif // PASS_PRISM_HPP
