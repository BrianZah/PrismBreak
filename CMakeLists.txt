cmake_minimum_required(VERSION 3.23)
project(Probability-Vis)

file(GLOB LIBS "${CMAKE_CURRENT_SOURCE_DIR}/libs")
file(GLOB LIBS_3RD_PARTY "${CMAKE_CURRENT_SOURCE_DIR}/libs_3rd_party")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXECUTABLE_SUFFIX ".exe")

include(CTest)

message("CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
message("CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs_3rd_party)
#add_subdirectory( ${LIBS}/logger )
add_subdirectory(${LIBS}/csv)
add_subdirectory(${LIBS}/context)
add_subdirectory(${LIBS}/gl)
add_subdirectory(${LIBS}/glfw)
add_subdirectory(${LIBS}/vis)
add_subdirectory(${LIBS}/menu)
add_subdirectory(${LIBS}/pass)
add_subdirectory(${LIBS}/gui)

set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_executable(main main.cpp)
target_link_libraries(main gui csv context)
