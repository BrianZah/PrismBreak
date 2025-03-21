#include "vis/Controls.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <functional>
#include <memory>
#include <iostream>

#include "vis/Camera.hpp"

namespace vis{
  Controls::Controls(GLFWwindow* const glfwWindow,
                     const std::shared_ptr<vis::Camera>& camera,
                     const std::shared_ptr<vis::Camera>& cameraAlt,
                     const std::function<void()>& refreshShader)
  : mGlfwWindow(glfwWindow), mCamera(camera), mCameraAlt(cameraAlt), mRefreshShader(refreshShader)
  {
    glfwSetWindowUserPointer(mGlfwWindow, this);
    glfwSetKeyCallback(mGlfwWindow, [] (GLFWwindow* const glfwWindow, int key, int scancode, int action, int mods) {
      Controls* controls= (Controls*)glfwGetWindowUserPointer(glfwWindow);
      controls->keyCallback(glfwWindow, key, scancode, action, mods);
    });
    glfwSetMouseButtonCallback(mGlfwWindow, [] (GLFWwindow* const glfwWindow, int button, int action, int mods) {
      Controls* controls = (Controls*)glfwGetWindowUserPointer(glfwWindow);
      controls->mouseButtonCallback(glfwWindow, button, action, mods);
    });
    glfwSetCursorPosCallback(mGlfwWindow, [] (GLFWwindow* const glfwWindow, double xpos, double ypos) {
      Controls* controls = (Controls*)glfwGetWindowUserPointer(glfwWindow);
      controls->cursorPosCallback(glfwWindow, xpos, ypos);
    });
    glfwSetScrollCallback(mGlfwWindow, [] (GLFWwindow* const glfwWindow, double xoffset, double yoffset) {
      Controls* controls = (Controls*)glfwGetWindowUserPointer(glfwWindow);
      controls->scrollCallback(glfwWindow, xoffset, yoffset);
    });
  }

  Controls::Controls() {}
  void Controls::init(GLFWwindow* const glfwWindow,
                      const std::shared_ptr<vis::Camera>& camera,
                      const std::shared_ptr<vis::Camera>& cameraAlt,
                      const std::function<void()>& refreshShader)
  {
    mGlfwWindow = glfwWindow;
    mCamera = camera;
    mCameraAlt = cameraAlt;
    mRefreshShader = refreshShader;

    glfwSetWindowUserPointer(mGlfwWindow, this);
    glfwSetKeyCallback(mGlfwWindow, [] (GLFWwindow* const glfwWindow, int key, int scancode, int action, int mods) {
      Controls* controls = (Controls*)glfwGetWindowUserPointer(glfwWindow);
      controls->keyCallback(glfwWindow, key, scancode, action, mods);
    });
    glfwSetMouseButtonCallback(mGlfwWindow, [] (GLFWwindow* const glfwWindow, int button, int action, int mods) {
      Controls* controls = (Controls*)glfwGetWindowUserPointer(glfwWindow);
      controls->mouseButtonCallback(glfwWindow, button, action, mods);
    });
    glfwSetCursorPosCallback(mGlfwWindow, [] (GLFWwindow* const glfwWindow, double xpos, double ypos) {
      Controls* controls = (Controls*)glfwGetWindowUserPointer(glfwWindow);
      controls->cursorPosCallback(glfwWindow, xpos, ypos);
    });
    glfwSetScrollCallback(mGlfwWindow, [] (GLFWwindow* const glfwWindow, double xoffset, double yoffset) {
      Controls* controls = (Controls*)glfwGetWindowUserPointer(glfwWindow);
      controls->scrollCallback(glfwWindow, xoffset, yoffset);
    });
  }

  void Controls::setExternalGuiWantCaptureMouse(const std::function<bool()>& externalGuiWantCaptureMouse) {
    mExternalGuiWantCaptureMouse = externalGuiWantCaptureMouse;
  }

  void Controls::setExternalGuiWantCaptureKeyboard(const std::function<bool()>& externalGuiWantCaptureKeyboard) {
    mExternalGuiWantCaptureKeyboard = externalGuiWantCaptureKeyboard;
  }
  void Controls::externalUserInteractionAppeared() {
    //mScrollCallback = true;
    mExternalMouseLeft = true;
    //mLastScrollCallback = glfwGetTime();
  }

  void Controls::update() {
    float currentFrame = glfwGetTime();
    mDeltaTime = currentFrame-mLastFrame;
    mLastFrame = currentFrame;

    mKeyCallback = false;
    //if(currentFrame > mLastScrollCallback+0.5f)
    mScrollCallback = false;
  }

  void Controls::keyCallback(GLFWwindow* const glfwWindow, int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(glfwWindow, true);

    if(mExternalGuiWantCaptureKeyboard()) return;

    if(key == GLFW_KEY_1 && action == GLFW_PRESS) {
      mKeyCallback = true;
      mRefreshShader();
    }
    if(key == GLFW_KEY_R && action == GLFW_PRESS) {
      mKeyCallback = true;
      mCamera->resetView();
      mCameraAlt->resetView();
    }
    if(key == GLFW_KEY_RIGHT_SHIFT || key == GLFW_KEY_LEFT_SHIFT) {
      if(action == GLFW_PRESS) {
        mKeyCallback = true;
        mShift = true;
      }
      if(action == GLFW_RELEASE) {
        mKeyCallback = true;
        mShift = false;
      }
    }
    if(key == GLFW_KEY_RIGHT_CONTROL || key == GLFW_KEY_LEFT_CONTROL) {
      if(action == GLFW_PRESS) {
        mKeyCallback = true;
        mCtrl = true;
      }
      if(action == GLFW_RELEASE) {
        mKeyCallback = true;
        mCtrl = false;
      }
    }
    if(key == GLFW_KEY_RIGHT_ALT || key == GLFW_KEY_LEFT_ALT) {
      if(action == GLFW_PRESS) {
        mKeyCallback = true;
        mAlt = true;
      }
      if(action == GLFW_RELEASE) {
        mKeyCallback = true;
        mAlt = mAltLock || false;
      }
    }
    if(key == GLFW_KEY_D && action == GLFW_PRESS) {
      mD = true;
    }
    if(key == GLFW_KEY_A && action == GLFW_PRESS) {
      mA = true;
    }
  }

  void Controls::cursorPosCallback(GLFWwindow* const glfwWindow, double xpos, double ypos) {
    //std::cout << "[Info] (Controls::cursorPosCallback) mCursorPos = " << mCursorPos[0] << ", " << mCursorPos[1] << std::endl;
    //mCursorPos = {-1.0, -1.0};
    glfwGetCursorPos(mGlfwWindow, &(mCursorPos[0]), &(mCursorPos[1]));

    if(mExternalGuiWantCaptureMouse()) return;

    if(mMouseLeft) {
      if(mFirstMouse) {
        mLastX = xpos;
        mLastY = ypos;
        mFirstMouse = false;
      }
      float xoffset = xpos - mLastX;
      float yoffset = mLastY - ypos;
      mLastX = xpos;
      mLastY = ypos;
      if(std::sqrt(xoffset*xoffset+yoffset*yoffset) > 0.025f) {
        mScrollCallback = true;
        mLastScrollCallback = glfwGetTime();
      }
      mAlt ? mCameraAlt->moveOnSphere(xoffset, yoffset)
           : mCamera->moveOnSphere(xoffset, 0.0f);
    }

    else if(mMouseRight) {
      if(mFirstMouse) {
        mLastX = xpos;
        mLastY = ypos;
        mFirstMouse = false;
      }
      float xoffset = mMouseSensitivity*(xpos - mLastX);
      float yoffset = mMouseSensitivity*(mLastY - ypos);
      mLastX = xpos;
      mLastY = ypos;
      if(std::sqrt(xoffset*xoffset+yoffset*yoffset) > 0.025f) {
        mScrollCallback = true;
        mLastScrollCallback = glfwGetTime();
      }
      mAlt ? mCameraAlt->move(xoffset, yoffset)
           : mCamera->move(xoffset, yoffset);
    }
    else if(mMouseMiddle) {
      if (mFirstMouse) {
        mLastX = xpos;
        mLastY = ypos;
        mFirstMouse = false;
      }
      float xoffset = xpos - mLastX;
      float yoffset = mLastY - ypos;
      mLastX = xpos;
      mLastY = ypos;
      if(std::sqrt(xoffset*xoffset+yoffset*yoffset) > 0.025f) {
        mScrollCallback = true;
        mLastScrollCallback = glfwGetTime();
      }
      //mModel->rotate(xoffset, yoffset, 0.0f);
      //mCamera->roll(xoffset);
      //mCameraAlt->moveOnSphere(xoffset, yoffset);
    }
  }

  void Controls::mouseButtonCallback(GLFWwindow* const glfwWindow, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
      mMouseLeft = true;
      if(mShift) {
        double xpos, ypos;
        glfwGetCursorPos(mGlfwWindow, &xpos, &ypos);
        mClickPos = {mouseLeft, xpos, ypos};
        //std::cout << "Cursor Position at (" << xpos << " : " << ypos << ")" << std::endl;
      }
      if(mCtrl) {
        double xpos, ypos;
        glfwGetCursorPos(mGlfwWindow, &xpos, &ypos);
        mClickPos = {mouseLeft, xpos, ypos};
        //std::cout << "Cursor Position at (" << xpos << " : " << ypos << ")" << std::endl;
      }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
      mMouseLeft = false;
      mFirstMouse = true;
      mExternalMouseLeft = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
      mMouseRight = true;
      if(mShift) mClickPos = {mouseRight, 0.0, 0.0};
      if(mCtrl) {
        double xpos, ypos;
        glfwGetCursorPos(mGlfwWindow, &xpos, &ypos);
        mClickPos = {mouseRight, xpos, ypos};
      }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
      mMouseRight = false;
      mFirstMouse = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS){
      mMouseMiddle = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
      mMouseMiddle = false;
      mFirstMouse = true;
    }
    //mLastScrollCallback = glfwGetTime();
  }
  void Controls::scrollCallback(GLFWwindow* const glfwWindow, double xoffset, double yoffset) {
    if(mExternalGuiWantCaptureMouse()) return;
    mAlt ? mCameraAlt->moveTowardsSphereCenter(yoffset)
         : mCamera->moveTowardsSphereCenter(yoffset);
    mScrollCallback = true;
    mLastScrollCallback = glfwGetTime();
  }

  const bool Controls::userInteraction() const {
    return //mMouseLeft || mMouseRight || mMouseMiddle ||
           mExternalMouseLeft ||
           mScrollCallback ||
           mKeyCallback;
  }

  std::array<double, 2> Controls::getCursorPos() const {return mCursorPos;}

  std::tuple<Controls::Button, double, double> Controls::getClickPos() {
    std::tuple<Button, double, double> clickPos = mClickPos;
    mClickPos = {none, 0.0, 0.0};
    return clickPos;
  }

  bool Controls::shiftClickAppeared() const {
    return mShift && (mMouseRight || mMouseLeft);
  }

  bool Controls::ctrlClickAppeared() const {
    return mCtrl && (mMouseRight || mMouseLeft);
  }

  bool Controls::aClickAppeared() {
    if(mA) {
      mA = false;
      return true;
    }
    return false;
  }

  bool Controls::dClickAppeared() {
    if(mD) {
      mD = false;
      return true;
    }
    return false;
  }

  void Controls::switchToCameraAlt() {mAltLock = true; mAlt = true;}
  void Controls::switchToCamera() {mAltLock = false; mAlt = false;}
} // namespace vis
