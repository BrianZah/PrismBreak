#include "vis/Window.hpp"

#include <functional>
#include <numbers>

#include "context/Context.hpp"
#include "context/Bookmarks.hpp"
#include "pass/RayCast.hpp"
#include "pass/Prism.hpp"
#include "pass/Bookmarks.hpp"
#include "pass/Points.hpp"
#include "pass/TextureToScreen.hpp"

#include "glfwPP/Window.hpp"
#include "vis/Camera.hpp"
#include "vis/Controls.hpp"
#include "vis/PrismFramebuffer.hpp"

namespace vis{
  Window::Window(const std::string& pathToData,
                 glfwPP::Window &parentWindow,
                 const int& posX, const int& posY,
                 const int& width, const int& height)
  : mWindow(parentWindow), mCamera(new Camera), mCameraAlt(new Camera),
    mBookmarksContext(new context::Bookmarks),
    mPerformanceTest(*this), mLoadingTest(*this)
  {
    mMode = prism;

    mPosX = posX;
    mPosY = posY;
    mWidth = width;
    mHeight = height;

    mCamera->init(width, height, 50.0f);
    mCameraAlt->init(width, height, 25.0f);
    mControls.init(mWindow.getGLFWwindowPtr(), mCamera, mCameraAlt,
                   std::bind(&Window::refreshShader, this));

    mContext = std::make_shared<context::Context>(pathToData);
    mContext->prepare(context::Context::tmm, 0, 0, 0);
    mBookmarksContext->init(mContext);
    mPrism.init(mContext, mBookmarksContext, mCamera, mCameraAlt);
    mBookmarks.init(mContext, mBookmarksContext, mCamera, mCameraAlt, mPrism.getFrontFacePosZ());
    mPoints.init(mContext, mCameraAlt);
    mRayCast.init(mContext, mCameraAlt, &mPoints.getPoints());
    mFramebuffer.init(width, height);
    mToScreen.init(&mFramebuffer.getColor());

    glEnable(GL_SCISSOR_TEST);
    glViewport(mPosX, mPosY, mWidth, mHeight);
    glScissor (mPosX, mPosY, mWidth, mHeight);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::cout << "[Info](Window::Window) complete" << std::endl;
  }

  void Window::loadContext(const std::string& pathToData) {
    mCamera->resetView();
    mCameraAlt->resetView();

    mContext = std::make_shared<context::Context>(pathToData);
    mContext->prepare(prism == mMode ? mPrism.getModel() : mRayCast.getModel(), 0, 0, 0);
    mBookmarksContext = std::make_shared<context::Bookmarks>();
    mBookmarksContext->init(mContext);

    mPrism.init(mContext, mBookmarksContext, mCamera, mCameraAlt);
    mBookmarks.init(mContext, mBookmarksContext, mCamera, mCameraAlt, mPrism.getFrontFacePosZ());
    mPoints.init(mContext, mCameraAlt);
    mRayCast.init(mContext, mCameraAlt, &mPoints.getPoints());
    if(rayCast == mMode) {
      mToScreen.setScreenTexture(&mFramebuffer.getColor());
      mControls.switchToCamera();
      mMode = prism;
    }

    std::cout << "[Info] (Window::loadContext) complete" << std::endl;
  }

  std::shared_ptr<const context::Context> Window::getContext() const {return mContext;}

  void Window::prepareContext(const context::Context::Model& model) {
    if(mPrism.getModel() == model) return;
    mContext->resizeComponentsAndWeights(model, mPrism.getStage());
    mContext->prepareModelComponents(model, mPrism.getStage());
  }

  void Window::setExternalGuiWantCaptureMouse(const std::function<bool()>& externalGuiWantCaptureMouse) {
    mControls.setExternalGuiWantCaptureMouse(externalGuiWantCaptureMouse);
  }

  void Window::setExternalGuiWantCaptureKeyboard(const std::function<bool()>& externalGuiWantCaptureKeyboard) {
    mControls.setExternalGuiWantCaptureKeyboard(externalGuiWantCaptureKeyboard);
  }

  void Window::refreshShader() {
    mPrism.refresh();
    mBookmarks.refresh();
    mPoints.refresh();
    mRayCast.refresh();
    mToScreen.refresh();
  }

  void Window::enterNextStage(const int& iCompSet, const int& iComp) {
    if(2 == mPrism.getStage()) {
      mMode = rayCast;
      mContext->prepare(mRayCast.getModel(), 3, iCompSet, iComp);
      mControls.switchToCameraAlt();
      mRayCast.enter();
      mPoints.enter();
      mToScreen.setScreenTexture(&mRayCast.getAlbedo());
    } else {
      mContext->prepare(mPrism.getModel(), mPrism.getStage()+1, iCompSet, iComp);
      mPrism.enterNextStage();
      mBookmarks.enter(mPrism.getStage());
    }
  }

  void Window::startBookmarkFocus(const int& index) {
    mBookmarksContext->prepareContext(mPrism.getModel(), mPrism.getStage(), index);
    mPrism.enter();
    auto [iCompSet, iComp, offset] = mBookmarksContext->getIndexSetAndCompAndOffset(mPrism.getStage(), index);
    mTargetedCumulatedXOffset = (iCompSet-offset)*60.0f;
    mStartCumulatedXOffset = mCamera->getCumulatedXOffset();
    mSteps = std::min(std::abs(int(mTargetedCumulatedXOffset-mStartCumulatedXOffset)), 180);
    mStep = 0;
  }

  void Window::startFocusOnFacet() {
    int index = 0;
    if(mMode == rayCast) return;
    if(mControls.dClickAppeared()) {
      index = 1;
    } else if(mControls.aClickAppeared()) {
      index = -1;
    }
    if(index == 0) return;
    mStartCumulatedXOffset = mCamera->getCumulatedXOffset();
    mTargetedCumulatedXOffset = (std::round(mStartCumulatedXOffset/60.f) + index)*60.0f;
    mSteps = std::min(std::abs(int(mTargetedCumulatedXOffset-mStartCumulatedXOffset)), 180);
    mStep = 0;
  }

  void Window::startFullRotation() {
    if(mMode == prism) return;
    if(not mControls.dClickAppeared()) return;
    mStartCumulatedXOffset = mCamera->getCumulatedXOffset();
    mTargetedCumulatedXOffset = mStartCumulatedXOffset + 360.0f;
    mSteps = 720;
    mStep = 0;
  }

  void Window::rotate(const bool& condition) {
    if(not condition) return;
    mStep++;
    mCameraAlt->moveOnSphere(1.0f, 0);
  }

  void Window::focusCameraOnBookmarkIf(const bool& condition) {
    if(not condition) return;
    mStep++;
    float progress = 0.5f*(-std::cos(std::numbers::pi*(float(mStep)/mSteps))+1.0f);
    float nextCumulatedXOffset = progress*mTargetedCumulatedXOffset + (1.0f-progress)*mStartCumulatedXOffset;
    mCamera->moveOnSphere(-2.0f*(nextCumulatedXOffset-mCamera->getCumulatedXOffset()), 0);
  }

  void Window::enterPreviousStage() {
    if(mMode == prism) {
      mContext->resizeComponentsAndWeights(mPrism.getModel(), mPrism.getStage()-1);
      mContext->prepareModelComponents(mPrism.getModel(), mPrism.getStage()-1);
      mPrism.enterPreviousStage();
      mBookmarks.enter(mPrism.getStage());
    } else if(mMode == rayCast) {
      mMode = prism;
      mPrism.enter();
      mBookmarks.enter(mPrism.getStage());
      mControls.switchToCamera();
      mToScreen.setScreenTexture(&mFramebuffer.getColor());
    }

    std::cout << "[Info] (Window::enterPreviousStage) complete" << std::endl;
  }

  void Window::updateModeIf(const bool& condition) {
    if(not condition) return;
    auto [button, xPos, yPos] = mControls.getClickPos();
    if(button == Controls::mouseLeft) {
      switch(mMode) {
        case prism: {
          auto [iCompSet, iComp] = mFramebuffer.getFacetAndTile(xPos, yPos);
          if(0 > iComp) return;
          if(0 <= iCompSet) enterNextStage(iCompSet, iComp);
          break;
        }
        case rayCast: {
          mRayCast.setCursor(xPos, yPos);
          break;
        }
      }
    } else if(button == Controls::mouseRight) {
      enterPreviousStage();
    }
  }

  void Window::updateBookmarksIf(const bool& condition) {
    if(not condition) return;
    auto [button, xPos, yPos] = mControls.getClickPos();
    if(button == Controls::mouseLeft) {
      auto [iCompSet, iComp] = mFramebuffer.getFacetAndTile(xPos, yPos);
      if(0 > iComp) return;
      if(0 <= iCompSet) {
        mBookmarksContext->add(mPrism.getStage(), mPrism.getModel(), iCompSet, iComp);
        mPrism.updateBookmarks();
        mBookmarks.update();
      } else if(-1 <= iCompSet) startBookmarkFocus(iComp);
    }
    else if(button == Controls::mouseRight) {
      auto [iCompSet, iComp] = mFramebuffer.getFacetAndTile(xPos, yPos);
      if(0 > iComp) return;
      if(0 <= iCompSet) {
        mBookmarksContext->remove(mPrism.getStage(), mPrism.getModel(), iCompSet, iComp);
        mPrism.updateBookmarks();
        mBookmarks.update();
      } else if(-1 <= iCompSet) {
        mBookmarksContext->remove(mPrism.getStage(), iComp);
        mPrism.updateBookmarks();
        mBookmarks.update();
      }
    }
  }

  void Window::update() {
    if(true) {
      if(mWindow.sizeHasChanged()) {
        mWidth = mWindow.getWidth();
        mHeight = mWindow.getHeight();
        mCamera->setResolution(mWidth, mHeight);
        mCameraAlt->setResolution(mWidth, mHeight);
        mPoints.resizeTextures(mWidth, mHeight);
        mPrism.resizeTextures(mWidth, mHeight);
        mBookmarks.resizeTextures(mWidth, mHeight);
        mRayCast.resizeTextures(mWidth, mHeight);
        mFramebuffer.resizeTextures(mWidth, mHeight);
      }
      if(mWindow.sizeHasChanged() || mRelocated) {
        glViewport(mPosX, mPosY, mWidth, mHeight);
        glScissor (mPosX, mPosY, mWidth, mHeight);
        mRelocated = false;
      }
      mPerformanceTest.executeIf(mPerformanceTest.isRunning());
      mLoadingTest.executeIf(mLoadingTest.isRunning());
      updateBookmarksIf(mControls.ctrlClickAppeared() && prism == mMode);
      updateModeIf(mControls.shiftClickAppeared());
      startFocusOnFacet();
      focusCameraOnBookmarkIf(mSteps > mStep && mMode == prism);
      startFullRotation();
      rotate(mSteps > mStep && mMode == rayCast);
      switch(mMode) {
        case prism: {
          mFramebuffer.bind();
          mFramebuffer.clearAll();
          mPrism.setCursorPos(mControls.getCursorPos());
          mPrism.execute();
          mBookmarks.execute();
          mFramebuffer.unbind();
          break;
        }
        case rayCast: {
          mPoints.execute();
          mRayCast.execute();
          break;
        }
      }
      mToScreen.execute();
    }
    mControls.update();
  }

  pass::Prism& Window::refPrism() {return mPrism;}
  const pass::Prism& Window::getPrism() const {return mPrism;}
  pass::RayCast& Window::refRayCast() {return mRayCast;}
  const pass::RayCast& Window::getRayCast() const {return mRayCast;}
  pass::Points& Window::refPoints() {return mPoints;}
  const pass::Points& Window::getPoints() const {return mPoints;}
  pass::Bookmarks& Window::refBookmarks() {return mBookmarks;}
  const pass::Bookmarks& Window::getBookmarks() const{return mBookmarks;}

  const Window::RenderPass& Window::getRenderPass() const {return mMode;}

  void Window::setPosX(const int& posX){mPosX = posX; mRelocated = true;}
  void Window::initiateRendering(){mInitiatedRendering = true;}
  void Window::initiateLowResolution(){ mControls.externalUserInteractionAppeared(); }

  const int& Window::getPosX() const { return mPosX; }
  const int& Window::getWidth() const { return mWidth; }
  const int& Window::getHeight() const { return mHeight; }
  const float Window::getRatio() const { return 1.0f*mWidth/mHeight; }
  const bool Window::cameraIsMoving() const { return mControls.userInteraction(); }

  Window::PerformanceTest& Window::refPerformanceTest() {return mPerformanceTest;}
  Window::LoadingTest& Window::refLoadingTest() {return mLoadingTest;}
} // namespace vis
