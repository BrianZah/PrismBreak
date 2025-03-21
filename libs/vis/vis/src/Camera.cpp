#include "vis/Camera.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cmath>
#include <tuple>

#include <iostream>
#include <glm/gtx/string_cast.hpp>

namespace vis{

  Camera::Camera(const int& imageWidth, const int& imageHeight,
                 const float& radius, const glm::vec3& center, const glm::vec3& up,
                 const float& fieldOfView,
                 const float& nearPlane, const float& farPlane)
  {
    init(imageWidth, imageHeight, radius, center, up, fieldOfView, nearPlane, farPlane);
  }

  void Camera::init(const int& imageWidth, const int& imageHeight,
                    const float& radius, const glm::vec3& center, const glm::vec3& up,
                    const float& fieldOfView,
                    const float& nearPlane, const float& farPlane) {
    mResolution = glm::ivec2(imageWidth, imageHeight);
    mFieldOfView = fieldOfView;
    mNearPlane = nearPlane;
    mFarPlane = farPlane;

    mInitRadius = radius;
    mInitCenter = center;
    mInitUp = up;
    resetView();
  }

  void Camera::setResolution(const int& imageWidth, const int& imageHeight) {
    mResolution = glm::ivec2(imageWidth, imageHeight);
  }
  const glm::ivec2& Camera::getResolution() const {return mResolution;}

  void Camera::resetView() {
    mCenter = mInitCenter;
    mPosition = mInitCenter + glm::vec3(0.0f, 0.0f, mInitRadius);
    mUp = mInitUp;

    mFront = glm::normalize(mCenter-mPosition);
    mRight = glm::normalize(glm::cross(mFront, mUp));

    mCumulatedXOffset = 0.0f;
  }
  glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(mPosition, mCenter, mUp);
  }
  glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(glm::radians(mFieldOfView), 1.0f*mResolution.x/mResolution.y, mNearPlane, mFarPlane);
    //return glm::ortho(-24.f, 24.f, -18.f, 18.f, sNearPlane, sFarPlane);
  }

  const glm::vec3& Camera::getPosition() const {return mPosition;}
  const glm::vec3& Camera::getCenter() const {return mCenter;}
  const glm::vec3& Camera::getUpVector() const {return mUp;}
  const float& Camera::getCumulatedXOffset() const {return mCumulatedXOffset;}

  std::tuple<glm::vec3, glm::mat4> Camera::positionAndViewMatrixPlusDelta(
    const float& horizantalAngle, const float& verticalAngle, const float& rollAngle
  ) const {
    glm::quat rotation = glm::angleAxis(glm::radians(0.5f*-horizantalAngle), mUp);
    glm::mat4 rot = glm::mat4_cast(rotation);
    glm::vec3 position = glm::vec3(rot*glm::vec4(mPosition-mCenter, 1.0f)) + mCenter;
    glm::vec3 front = glm::normalize(mCenter-position);
    glm::vec3 right = glm::normalize(glm::cross(front, mUp));

    rotation = glm::angleAxis(glm::radians(0.5f*verticalAngle), right);
    rot = glm::mat4_cast(rotation);
    position = glm::vec3(rot*glm::vec4(position-mCenter, 1.0f)) + mCenter;
    front = glm::normalize(mCenter-position);
    glm::vec3 up = glm::normalize(glm::cross(right, front));

    rotation = glm::angleAxis(glm::radians(rollAngle), front);
    rot = glm::mat4_cast(rotation);
    up = glm::normalize(glm::vec3(rot*glm::vec4(up, 1.0f)));

    return {position, glm::lookAt(position, mCenter, up)};
  }

  void Camera::moveOnSphere(const float& xoffset, const float& yoffset) {
    glm::quat rotation = glm::angleAxis(glm::radians(0.5f*-xoffset), mUp);
    glm::mat4 rot = glm::mat4_cast(rotation);
    mPosition = glm::vec3(rot*glm::vec4(mPosition-mCenter, 1.0f)) + mCenter;
    mFront = glm::normalize(mCenter-mPosition);
    mRight = glm::normalize(glm::cross(mFront, mUp));
    mCumulatedXOffset+= 0.5f*-xoffset;
    //std::cout << "[Info](Camera::moveOnSphere) mCumulatedXOffset = " << mCumulatedXOffset << std::endl;

    rotation = glm::angleAxis(glm::radians(0.5f*yoffset), mRight);
    rot = glm::mat4_cast(rotation);
    mPosition = glm::vec3(rot*glm::vec4(mPosition-mCenter, 1.0f)) + mCenter;
    mFront = glm::normalize(mCenter-mPosition);
    mUp = glm::normalize(glm::cross(mRight, mFront));
  }
  void Camera::moveTowardsSphereCenter(const float& zoffset) {
    //mPosition+= glm::distance(mCenter, mPosition)*zoffset*deltaTime*mFront;
    float distance = (zoffset >= 0) ? 0.98f*zoffset : 1/(0.98f*zoffset);
    mPosition = mCenter - glm::distance(mCenter, mPosition)*distance*distance*mFront;
    //std::cout << "mPosition = " << glm::to_string(mPosition) << std::endl;
  }
  void Camera::roll(const float& xoffset) {
    glm::quat rot = glm::angleAxis(glm::radians(xoffset), mFront);
    glm::mat4 trans = glm::mat4_cast(rot);
    mUp = glm::normalize(glm::vec3(trans * glm::vec4(mUp, 1.0f)));
    mRight = glm::normalize(glm::cross(mFront, mUp));
  }
  void Camera::move(const float& xoffset, const float& yoffset, const float& zoffset) {
    //xoffset*= sMouseSensitivity;
    mCenter-= mRight*xoffset;
    mPosition-= mRight*xoffset;

    //yoffset*= sMouseSensitivity;
    mCenter-= mUp*yoffset;
    mPosition-= mUp*yoffset;
  }
} // namespace vis
