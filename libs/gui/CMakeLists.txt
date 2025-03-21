add_library(gui gui/src/Manager.cpp)
target_link_libraries(gui glPP glfwPP imgui ImGuiFileDialog vis context)
target_include_directories(gui PUBLIC ./)
target_compile_definitions(gui PRIVATE CMAKE_GUIFONT="${LIBS_3RD_PARTY}/imgui-1.89.9/misc/fonts/DroidSans.ttf"
                                       CMAKE_GUIFONTSIZE="25")
