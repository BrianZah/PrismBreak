add_library(context ./src/Context.cpp
                    ./src/Context_read.cpp
                    ./src/Context_internal.cpp
                    ./src/Context_TMMFunctions.cpp
                    ./src/Context_GMMFunctions.cpp
                    ./src/Context_metrics.cpp
                    ./src/context_Bookmarks.cpp)
target_include_directories(context PUBLIC ./)
target_link_libraries(context eigen glm csv)
