add_library(pass ./src/pass_RayCast.cpp
                 ./src/pass_Points.cpp
                 ./src/pass_Prism.cpp
                 ./src/pass_Prism_Visibility.cpp
                 ./src/pass_Bookmarks.cpp
                 ./src/pass_TextureToScreen.cpp)
target_link_libraries(pass stbi glCPP vis context)
target_include_directories(pass PUBLIC ./)
target_compile_definitions(pass PRIVATE CMAKE_SHADERDIR="${CMAKE_CURRENT_SOURCE_DIR}/shader/"
                                        CMAKE_TEXTUREDIR="${CMAKE_CURRENT_SOURCE_DIR}/textures/")
