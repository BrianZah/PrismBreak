include_directories(include)
#add_subdirectory(src)

add_library(glad src/glad.c)
target_include_directories(glad PUBLIC include)
#add_library(glad include/glad/glad.h src/glad.c)
#install(TARGETS glad DESTINATION lib)
#install(FILES include/glad/glad.h DESTINATION include/glad)
