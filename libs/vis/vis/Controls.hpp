#ifndef VIS_CONTROLS_HPP
#define VIS_CONTROLS_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <functional>
#include <memory>
#include <array>

#include "vis/Camera.hpp"

namespace vis{
  class Controls{
    public:
      enum Button{none, mouseLeft, mouseRight};

      Controls(GLFWwindow* const glfwWindow,
               const std::shared_ptr<vis::Camera>& camera,
               const std::shared_ptr<vis::Camera>& cameraAlt,
               const std::function<void()>& refreshShader);

      Controls();
      void init(GLFWwindow* const glfwWindow,
                const std::shared_ptr<vis::Camera>& camera,
                const std::shared_ptr<vis::Camera>& cameraAlt,
                const std::function<void()>& refreshShader);

      void update();
      const bool userInteraction() const;
      std::array<double, 2> getCursorPos() const;
      std::tuple<Button, double, double> getClickPos();
      bool shiftClickAppeared() const;
      bool ctrlClickAppeared() const;
      void switchToCameraAlt();
      void switchToCamera();
      bool aClickAppeared();
      bool dClickAppeared();

      void setExternalGuiWantCaptureMouse(const std::function<bool()>& externalGuiWantCaptureMouse);
      void setExternalGuiWantCaptureKeyboard(const std::function<bool()>& externalGuiWantCaptureKeyboard);
      void externalUserInteractionAppeared();

    private:
      void keyCallback(GLFWwindow* const glfwWindow, int key, int scancode, int action, int mods);
      void cursorPosCallback(GLFWwindow * const glfwWindow, double xpos, double ypos);
      void mouseButtonCallback(GLFWwindow * const glfwWindow, int button, int action, int mods);
      void scrollCallback(GLFWwindow * const glfwWindow, double xoffset, double yoffset);

      GLFWwindow* mGlfwWindow = nullptr;
      std::shared_ptr<vis::Camera> mCamera;
      std::shared_ptr<vis::Camera> mCameraAlt;
      std::function<void()> mRefreshShader;
      std::function<const bool()> mExternalGuiWantCaptureMouse = [](){ return false; };
      std::function<const bool()> mExternalGuiWantCaptureKeyboard = [](){ return false; };

      float mMouseSensitivity = 0.01f;
      float mDeltaTime = 0.0f;
      float mLastFrame = 0.0f;
      float mLastScrollCallback = 0.0f;
      bool mFirstMouse = true;
      float mLastX = 0.0f;
      float mLastY = 0.0f;
      std::tuple<Button, double, double> mClickPos = {none, 0.0, 0.0};
      std::array<double, 2> mCursorPos = {-1.0, -1.0};

      bool mKeyCallback = false;
      bool mMouseLeft = false;
      bool mMouseRight = false;
      bool mMouseMiddle = false;
      bool mScrollCallback = false;
      bool mExternalMouseLeft = false;
      bool mShift = false;
      bool mCtrl = false;
      bool mAlt = false;
      bool mAltLock = false;
      bool mA = false;
      bool mD = false;
  }; // class Controls
} // namespace vis
#endif // VIS_CONTROLS_HPP
