add_library(glfwPP glfwPP/src/Window.cpp)
target_link_libraries(glfwPP glad glfw ${CMAKE_DL_LIBS})
target_include_directories(glfwPP PUBLIC ./)
