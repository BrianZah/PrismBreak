#ifndef GLFWPP_WINDOW_HPP
#define GLFWPP_WINDOW_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace glfwPP{
class Window{
public:
  Window(const int& width = 1920, const int& height = 1080,
         const int& versionMajor = 4, const int& versionMinor = 5,
         const bool& frameLimit = true);

  const bool initializationFailed() const;
  const bool sizeHasChanged() const;
  const bool shouldClose() const;
  void disableFrameLimit() const;

  void update();

  GLFWwindow* const getGLFWwindowPtr() const;
  const int& getWidth() const;
  const int& getHeight() const;
  const float getRatio() const;

  ~Window();
private:
  GLFWwindow* mGlfwWindow;
  int mWidth;
  int mHeight;

  bool mSizeHasChanged = true;
  bool mInitializationFailed = true;
};
} // namespace glfwPP

#endif//GLFWPP_WINDOW_HPP
