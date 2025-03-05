#include "vis_Window.hpp"

#include <functional>

#include "pass_RayCast.hpp"
#include "pass_Prism.hpp"
#include "pass_Bookmarks.hpp"
#include "pass_Points.hpp"
#include "pass_TextureToScreen.hpp"

#include "glfw_Window.hpp"
#include "vis_Camera.hpp"
#include "vis_Controls.hpp"
#include "vis_PrismFramebuffer.hpp"

#include <glm/gtx/string_cast.hpp>

namespace vis{
  //Window::Window(glfw::Window &parentWindow)
  //: mPosX(0), mPosY(0), mCamera(new Camera),
  //  mControls(parentWindow.getGLFWwindowPtr(), mCamera,
  //            std::bind(&Window::refreshShader, this)),
  //  mWidth(parentWindow.getWidth()), mHeight(parentWindow.getHeight()),
  //  mRayCast(mCamera, parentWindow.getWidth(), parentWindow.getHeight()),
  //  ssao(mCamera, &mRayCast.getPosition(), &mRayCast.getNormal()),
  //  illumination(&mRayCast.getAlbedo(), &mRayCast.getPosition(),
  //               &mRayCast.getNormal(), &ssao.getSsao()),
  //  mToScreen(&illumination.getIlluminatedScene())
  //{
  //  glEnable(GL_SCISSOR_TEST);
  //  glEnable(GL_BLEND);
  //  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //}

  Window::CameraOptimizer::CameraOptimizer(Window& window)
  : mWindow(window), mRayCast(window.mRayCast), mCamera(window.mCameraAlt), mMode(converged)
  {}

  void Window::CameraOptimizer::start(const int& maxSteps) {
    mMaxSteps = maxSteps;
    mMode = warmup;
  }
  bool Window::CameraOptimizer::running() {return mMode != converged;}
  void Window::CameraOptimizer::stop() {
    mMode = converged;
    mRayCast.setNumEvalCameras(0);
  }

  glm::vec3 Window::CameraOptimizer::orthogonalize(const glm::vec3& basis, const glm::vec3& input) const {
    return glm::normalize(input - (glm::dot(input, basis)/glm::dot(basis, basis))*basis);
  }

  glm::vec3 Window::CameraOptimizer::sphericalToCartesian(const float& radius, const float& inc, const float azi) const {
    return radius*glm::vec3(std::sin(inc)*std::cos(azi), std::sin(inc)*std::sin(azi), std::cos(inc));
  }

  glm::vec3 Window::CameraOptimizer::cartesianToSpherical(const float& x, const float& y, const float z) const {
    return glm::vec3(std::sqrt(x*x+y*y+z*z), std::atan2(std::sqrt(x*x+y*y), z), std::atan2(y, x));
  }

  void Window::CameraOptimizer::placeCamerasFor(const Mode& mode) {
    switch(mode) {
      case globalSearch: {
        std::cout << "globalSearch" << std::endl;
        for(int cam = 0; cam < mNumCamsGlobalSearch; ++cam) {
          float theta = std::acos(1.0f - 2.0f*(cam+0.5f)/mNumCamsGlobalSearch);
          float phi = std::numbers::pi * (1.0f+std::sqrt(5.0f)) * (cam+0.5f);
          glm::vec3 camPos = glm::vec3(std::sin(theta)*std::cos(phi),
                                       std::sin(theta)*std::sin(phi),
                                       std::cos(theta));
          float radius = glm::distance(mCamera->getCenter(), mCamera->getPosition());
          camPos = camPos*radius + mCamera->getCenter();
          glm::vec3 front = glm::normalize(mCamera->getCenter()-camPos);
          glm::vec3 up = glm::abs(front.y) < 0.99f
                         ? orthogonalize(front, glm::vec3(0.0f, 1.0f, 0.0f))
                         : orthogonalize(front, glm::vec3(0.0f, 0.0f, -1.0f));
          glm::mat4 view = glm::lookAt(camPos, mCamera->getCenter(), up);
          std::cout << "[Info] (Window::CameraOptimizer::placeCamerasFor) view = " << glm::to_string(view) << std::endl;
          std::cout << "[Info] (Window::CameraOptimizer::placeCamerasFor) camPos = " << glm::to_string(camPos) << std::endl;
          std::cout << "[Info] (Window::CameraOptimizer::placeCamerasFor) front = " << glm::to_string(front) << std::endl;
          std::cout << "[Info] (Window::CameraOptimizer::placeCamerasFor) up = " << glm::to_string(up) << std::endl;
          mRayCast.setEvalCamera(cam, camPos, view);
        }
        break;
      }
      case optimize: {
        std::cout << "optimize" << std::endl;
        auto [pos, view] = mCamera->positionAndViewMatrixPlusDelta(4.0f, 0.0f, 0.0f);
        mRayCast.setEvalCamera(0, pos, view);
        std::tie(pos, view) = mCamera->positionAndViewMatrixPlusDelta(0.0f, 4.0f, 0.0f);
        mRayCast.setEvalCamera(1, pos, view);
        std::tie(pos, view) = mCamera->positionAndViewMatrixPlusDelta(0.0f, 0.0f, 4.0f);
        mRayCast.setEvalCamera(2, pos, view);
        break;
      }
      case search: {
        std::cout << "stepWidth = " << mStepWidth.transpose() << std::endl;
        std::cout << "gradient = " << mGradient.transpose() << std::endl;
        auto [pos, view] = mCamera->positionAndViewMatrixPlusDelta(
          mStepWidth[2]*mGradient[0], mStepWidth[2]*mGradient[1], mStepWidth[2]*mGradient[2]);
        mRayCast.setEvalCamera(2, pos, view);
        std::tie(pos, view) = mCamera->positionAndViewMatrixPlusDelta(
          mStepWidth[1]*mGradient[0], mStepWidth[1]*mGradient[1], mStepWidth[1]*mGradient[2]);
        mRayCast.setEvalCamera(1, pos, view);
        std::tie(pos, view) = mCamera->positionAndViewMatrixPlusDelta(
          mStepWidth[0]*mGradient[0], mStepWidth[0]*mGradient[1], mStepWidth[0]*mGradient[2]);
        mRayCast.setEvalCamera(0, pos, view);
        break;
      }
    }
  }

  bool Window::CameraOptimizer::operator()() {
    switch(mMode) {
      case converged: {
        return true;
      }
      case warmup: {
        //placeCamerasFor(optimize);
        mRayCast.setNumEvalCameras(mNumCamsGlobalSearch);
        placeCamerasFor(globalSearch);
        //mMode = optimize;
        mMode = globalSearch;
        return false;
      }
      case globalSearch: {
        for(int cam = 0; cam < mNumCamsGlobalSearch; ++cam) {
          std::cout << cam << " = " << mRayCast.evalData()[cam] << std::endl;
        }
        int bestCamera;
        mRayCast.evalData().maxCoeff(&bestCamera);
        std::cout << "bestCamera = " << bestCamera << std::endl;
        mTargetPosition = mRayCast.getEvalCameraPosition(bestCamera);
        std::cout << "[Info] (Window::CameraOptimizer::placeCamerasFor) mTargetPosition = " << glm::to_string(mTargetPosition) << std::endl;
        mTargetPosition = cartesianToSpherical(mTargetPosition.x, mTargetPosition.y, mTargetPosition.z);
        glm::vec3 startPosition = cartesianToSpherical(mCamera->getPosition().x, mCamera->getPosition().y, mCamera->getPosition().z);
        std::cout << "[Info] (Window::CameraOptimizer::placeCamerasFor) mTargetPosition = " << glm::to_string(mTargetPosition) << std::endl;
        std::cout << "[Info] (Window::CameraOptimizer::placeCamerasFor) startPosition = " << glm::to_string(startPosition) << std::endl;
        mOffset = mTargetPosition - startPosition;
        mSteps = 10;
        mStepWidth.x() = mOffset.z/mSteps/std::numbers::pi * 360.0f;
        mStepWidth.y() = mOffset.y/mSteps/std::numbers::pi * 360.0f;
        mRayCast.setNumEvalCameras(0);
        mMode = moveCamera;
        return false;
      }
      case moveCamera: {
        //mCamera->moveOnSphere(mStepWidth.y(), 0.0f);
        mCamera->moveOnSphere(0.0f, mStepWidth.x());
        glm::vec3 startPosition = cartesianToSpherical(mCamera->getPosition().x, mCamera->getPosition().y, mCamera->getPosition().z);
        std::cout << "[Info] (Window::CameraOptimizer::placeCamerasFor) startPosition = " << glm::to_string(startPosition) << std::endl;
        if(--mSteps == 0) {
          mMode = converged;
          return true;
        }
        return false;
      }
      case optimize: {
        if(--mMaxSteps < 0) {
          mMode = converged;
          mRayCast.setNumEvalCameras(0);
          return true;
        }
        mGradient = {mRayCast.evalData()[1] - mRayCast.evalData()[0],
                     mRayCast.evalData()[2] - mRayCast.evalData()[0],
                     mRayCast.evalData()[3] - mRayCast.evalData()[0]};
        mStepWidth[2] = std::min(8.0f, std::max(4.0f, mGradient.norm()));
        mStepWidth[1] = 0.5f*mStepWidth[2];
        mStepWidth[0] = 0.25f*mStepWidth[2];
        mGradient.normalize();
        placeCamerasFor(search);
        mMode = search;
        return false;
      }
      case search: {
        int bestCamera;
        mRayCast.evalData().maxCoeff(&bestCamera);
        std::cout << mRayCast.evalData()[0] << " " << mRayCast.evalData()[1] << " " << mRayCast.evalData()[2] << " " << mRayCast.evalData()[3] << " ";
        std::cout << "bestCamera = " << bestCamera << std::endl;
        if(bestCamera != 0) {
          mCamera->moveOnSphere(mStepWidth[bestCamera-1]*mGradient[0], mStepWidth[bestCamera-1]*mGradient[1]);
          mCamera->roll(mStepWidth[bestCamera-1]*mGradient[2]);
          placeCamerasFor(optimize);
          mMode = optimize;
        } else {
          if(mStepWidth[2] < 0.1f) {
            mMode = converged;
            mRayCast.setNumEvalCameras(0);
            return true;
          }
          mStepWidth*= 0.125f;
          placeCamerasFor(search);
        }
        return false;
      }
    }
  }
} // namespace vis
