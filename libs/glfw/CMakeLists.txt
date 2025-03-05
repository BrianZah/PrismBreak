add_library(glfwCPP src/glfw_Window.cpp)
target_link_libraries(glfwCPP glad glfw ${CMAKE_DL_LIBS})
target_include_directories(glfwCPP PUBLIC ./)
