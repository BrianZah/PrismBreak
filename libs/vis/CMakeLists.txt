add_library(vis vis/src/Camera.cpp
                vis/src/Controls.cpp
                vis/src/Window.cpp
                vis/src/Window_PerformanceTest.cpp
                vis/src/PrismFramebuffer.cpp)
target_link_libraries(vis glad glfw glm glPP glfwPP pass context)
target_include_directories(vis PUBLIC ./)
