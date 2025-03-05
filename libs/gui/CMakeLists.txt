add_library(gui src/gui_Manager.cpp)
target_link_libraries(gui glCPP glfwCPP menu vis context)
target_include_directories(gui PUBLIC ./)
