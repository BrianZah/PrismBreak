add_subdirectory(glfw-3.3-stable)
add_subdirectory(glm-0.9.9.8/glm)

add_library(glad glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)

add_library(eigen INTERFACE)
target_include_directories(eigen INTERFACE eigen-3.4.1)

add_library(imgui imgui-1.89.9/imgui.cpp
                  imgui-1.89.9/imgui_draw.cpp
                  imgui-1.89.9/imgui_tables.cpp
                  imgui-1.89.9/imgui_widgets.cpp
                  imgui-1.89.9/imgui_demo.cpp
                  imgui-1.89.9/backends/imgui_impl_opengl3.cpp
                  imgui-1.89.9/backends/imgui_impl_glfw.cpp)
target_link_libraries(imgui glfw)
target_include_directories(imgui PUBLIC imgui-1.89.9 imgui-1.89.9/backends)

add_subdirectory(ImGuiFileDialog-master)

add_library(stbi INTERFACE)
target_include_directories(stbi INTERFACE stbi)
