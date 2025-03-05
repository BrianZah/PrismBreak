add_library(menu INTERFACE)
target_include_directories(menu INTERFACE ./)
target_link_libraries(menu INTERFACE glfwCPP imgui ImGuiFileDialog)
