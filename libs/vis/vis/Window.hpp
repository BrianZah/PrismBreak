#ifndef VIS_WINDOW_HPP
#define VIS_WINDOW_HPP

#include <functional>
#include <iostream>
#include <chrono>
#include <memory>

#include "context/Context.hpp"
#include "context/Bookmarks.hpp"
#include "pass/Points.hpp"
#include "pass/Prism.hpp"
#include "pass/Bookmarks.hpp"
#include "pass/RayCast.hpp"
#include "pass/TextureToScreen.hpp"

#include "glfwPP/Window.hpp"
#include "vis/Camera.hpp"
#include "vis/Controls.hpp"
#include "vis/PrismFramebuffer.hpp"

namespace vis{
  class Window{
  private:
    class CameraOptimizer{
    private:
      friend class vis::Window;
      enum Mode{converged, warmup, globalSearch, moveCamera, optimize, search};
      Window& mWindow;
      std::shared_ptr<vis::Camera>& mCamera;
      pass::RayCast& mRayCast;
      Mode mMode;
      int mNumCamsGlobalSearch = 10;
      glm::vec3 mOffset;
      int mSteps;
      Eigen::Vector3f mStepWidth;
      Eigen::Vector3f mGradient;
      glm::vec3 mTargetPosition;
      int mMaxSteps;
      CameraOptimizer(Window& window);
      glm::vec3 orthogonalize(const glm::vec3& basis, const glm::vec3& input) const;
      glm::vec3 sphericalToCartesian(const float& radius, const float& inc, const float azi) const;
      glm::vec3 cartesianToSpherical(const float& x, const float& y, const float z) const;
      void placeCamerasFor(const Mode& mode);
      bool operator()();
    public:
      void start(const int& maxSteps = 1);
      bool running();
      void stop();
    };
    class PerformanceTest{
    private:
      Window& mWindow;
      std::chrono::time_point<std::chrono::high_resolution_clock> mStart;
      std::chrono::time_point<std::chrono::high_resolution_clock> mLast;
      bool mIsRunning = false;
      const int mSteps = 600;
      int mFrameCount = 0;
      std::vector<double> mTimes;
      bool mInitRun = true;
      double mMinimumMsPerFrame = 10000.0;
      double mMaximumMsPerFrame = 0.0;
    public:
      PerformanceTest(Window& window);
      void start();
      const bool& isRunning() const;
      void executeIf(const bool& condition);
      void executeTest2();
    };
    class LoadingTest{
    private:
      Window& mWindow;
      std::chrono::time_point<std::chrono::high_resolution_clock> mStart;
      std::chrono::time_point<std::chrono::high_resolution_clock> mLast;
      bool mIsRunning = false;
      const int mRuns = 10;
      const int mTasks = 4;
      std::vector<std::string> mDataSets = {"../data/simplex(dim=6_points=10000)",
                                            "../data/simplex(dim=12_points=10000)",
                                           "../data/simplex(dim=18_points=10000)"};
      int mFrameCount = 0;
      std::vector<double> mTimes;
      bool mInitRun = true;
    public:
      LoadingTest(Window& window);
      void start();
      const bool& isRunning() const;
      void executeIf(const bool& condition);
    };
  public:
    enum RenderPass{prism, rayCast};
  private:
    friend class CameraOptimizer;
    friend class PerformanceTest;
    friend class LoadingTest;

    std::shared_ptr<vis::Camera> mCamera;
    std::shared_ptr<vis::Camera> mCameraAlt;
    vis::Controls mControls;
    CameraOptimizer mOptimizeCamera;

    int mPosX;
    int mPosY;
    int mWidth;
    int mHeight;

    bool mRelocated = false;
    bool mInitiatedRendering = false;
    int mRenderedBufferFrames = 0;

    RenderPass mMode = prism;

    std::shared_ptr<context::Context> mContext;
    glfwPP::Window& mWindow;
    std::shared_ptr<context::Bookmarks> mBookmarksContext;
    PrismFramebuffer mFramebuffer;
    pass::Prism mPrism;
    pass::Bookmarks mBookmarks;
    pass::Points mPoints;
    pass::RayCast mRayCast;
    pass::TextureToScreen mToScreen;

    PerformanceTest mPerformanceTest;
    LoadingTest mLoadingTest;

    float mTargetedCumulatedXOffset = 0.0f;
    float mStartCumulatedXOffset;
    int mSteps = 0;
    int mStep = mSteps;

  public:
    Window(const std::string& pathToData, glfwPP::Window &parentWindow,
           const int& posX, const int& posY, const int& width, const int& height);

    void loadContext(const std::string& pathToData);
    std::shared_ptr<const context::Context> getContext() const;
    void prepareContext(const context::Context::Model& model);

    void setExternalGuiWantCaptureMouse(const std::function<bool()>& externalGuiWantCaptureMouse);
    void setExternalGuiWantCaptureKeyboard(const std::function<bool()>& externalGuiWantCaptureKeyboard);
    void refreshShader();

private:
    void enterNextStage(const int& iCompSet, const int& iComp);
    void startBookmarkFocus(const int& index);
    void focusCameraOnBookmarkIf(const bool& condition);
    void startFocusOnFacet();
    void startFullRotation();
    void rotate(const bool& condition);
    void enterPreviousStage();
    void updateModeIf(const bool& condition);
    void updateBookmarksIf(const bool& condition);
public:
    void update();
    void setPosX(const int& posX);
    void setWidth(const int& width);
    void setHeight(const int& height);
    void initiateRendering();
    void initiateLowResolution();

    pass::Prism& refPrism();
    const pass::Prism& getPrism() const;
    pass::RayCast& refRayCast();
    const pass::RayCast& getRayCast() const;
    pass::Points& refPoints();
    const pass::Points& getPoints() const;
    pass::Bookmarks& refBookmarks();
    const pass::Bookmarks& getBookmarks() const;

    const RenderPass& getRenderPass() const;

    const int& getPosX() const;
    const int& getWidth() const;
    const int& getHeight() const;
    const float getRatio() const;
    const bool cameraIsMoving() const;
    CameraOptimizer& refCameraOptimizer();
    PerformanceTest& refPerformanceTest();
    LoadingTest& refLoadingTest();
  }; // class Window
} // namespace vis
#endif // VIS_WINDOW_HPP
