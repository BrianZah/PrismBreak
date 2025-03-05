add_library(vis src/vis_Camera.cpp
                src/vis_Controls.cpp
                src/vis_Window.cpp
                src/vis_Window_CameraOptimizer.cpp
                src/vis_Window_PerformanceTest.cpp
                src/vis_PrismFramebuffer.cpp)
target_link_libraries(vis glad glfw glm glfwCPP pass)
target_include_directories(vis PUBLIC ./)
