#include "glfw_Window.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace glfw{
Window::Window(const int& width, const int& height, const int& versionMajor, const int& versionMinor, const bool& frameLimit)
: mWidth(width), mHeight(height)
{
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, versionMajor);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, versionMinor);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  mGlfwWindow = glfwCreateWindow(mWidth, mHeight, "PrismBreak", NULL, NULL);
  if(mGlfwWindow == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(mGlfwWindow);
  //glfwSetFramebufferSizeCallback(mGlfwWindow, framebuffer_size_callback);
  glfwSwapInterval(frameLimit ? 1 : 0);

  if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return;
  }
  mInitializationFailed = false;
}

const bool Window::initializationFailed() const {return mInitializationFailed;}
const bool Window::sizeHasChanged() const {return mSizeHasChanged;}
const bool Window::shouldClose() const {return glfwWindowShouldClose(mGlfwWindow);}
void Window::disableFrameLimit() const {glfwSwapInterval(0);}

void Window::update() {
  glfwSwapBuffers(mGlfwWindow);
  glfwPollEvents();

  int oldWindowWidth = mWidth;
  int oldWindowHight = mHeight;
  glfwGetWindowSize(mGlfwWindow, &mWidth, &mHeight);

  mSizeHasChanged = mWidth != oldWindowWidth || mHeight != oldWindowHight;
}

GLFWwindow* const Window::getGLFWwindowPtr() const {return mGlfwWindow;}
const int& Window::getWidth() const {return mWidth;}
const int& Window::getHeight() const {return mHeight;}
const float Window::getRatio() const {return 1.0f*mWidth/mHeight;}

Window::~Window() {
  glfwDestroyWindow(mGlfwWindow);
  glfwTerminate();
}
} // namespace glfw
