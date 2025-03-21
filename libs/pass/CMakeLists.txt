add_library(pass ./pass/src/RayCast.cpp
                 ./pass/src/Points.cpp
                 ./pass/src/Prism.cpp
                 ./pass/src/Bookmarks.cpp
                 ./pass/src/TextureToScreen.cpp)
target_link_libraries(pass stbi glPP vis context)
target_include_directories(pass PUBLIC ./)
target_compile_definitions(pass PRIVATE CMAKE_SHADERDIR="${CMAKE_CURRENT_SOURCE_DIR}/pass/shader/"
                                        CMAKE_TEXTUREDIR="${CMAKE_CURRENT_SOURCE_DIR}/pass/textures/")
