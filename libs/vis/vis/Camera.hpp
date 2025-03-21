#ifndef VIS_CAMERA_HPP
#define VIS_CAMERA_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <tuple>

namespace vis{
  class Camera{
  public:
    Camera(const int& imageWidth = 1920, const int& imageHeight = 1080,
           const float& radius = 1.5f,
           const glm::vec3& center = glm::vec3(0.0f, 0.0f, 0.0f),
           const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
           const float& fieldOfView = 45.0f,
           const float& nearPlane = 0.025f, const float& farPlane = 100.0f);
    void init(const int& imageWidth, const int& imageHeight,
              const float& radius = 1.5f,
              const glm::vec3& center = glm::vec3(0.0f, 0.0f, 0.0f),
              const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
              const float& fieldOfView = 45.0f,
              const float& nearPlane = 0.025f, const float& farPlane = 100.0f);

    void setResolution(const int& imageWidth, const int& imageHeight);
    const glm::ivec2& getResolution() const;

    void resetView();
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    const glm::vec3& getPosition() const;
    const glm::vec3& getCenter() const;
    const glm::vec3& getUpVector() const;

    const float& getCumulatedXOffset() const;
    std::tuple<glm::vec3, glm::mat4> positionAndViewMatrixPlusDelta(const float& horizantalAngle, const float& verticalAngle, const float& rollAngle) const;

    void moveOnSphere(const float& xoffset, const float& yoffset);
    void moveTowardsSphereCenter(const float& zoffset);
    void roll(const float& xoffset);
    void move(const float& xoffset, const float& yoffset, const float& zoffset = 0);

  private:
    glm::ivec2 mResolution;
    float mFieldOfView;
    float mNearPlane;
    float mFarPlane;

    float mInitRadius;
    glm::vec3 mInitCenter;
    glm::vec3 mInitUp;

    glm::vec3 mPosition;
    glm::vec3 mCenter;
    glm::vec3 mUp;

    glm::vec3 mFront;
    glm::vec3 mRight;

    float mCumulatedXOffset = 0.0f;
  }; // class Camera
} // namespace vis
#endif // VIS_CAMERA_HPP
