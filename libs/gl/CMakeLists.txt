add_library(glCPP INTERFACE)
target_link_libraries(glCPP INTERFACE glad glm)
target_include_directories(glCPP INTERFACE ./)
