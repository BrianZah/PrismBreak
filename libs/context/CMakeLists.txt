add_library(context ./context/src/Context.cpp
                    ./context/src/Context_read.cpp
                    ./context/src/Context_internal.cpp
                    ./context/src/Context_TMMFunctions.cpp
                    ./context/src/Context_GMMFunctions.cpp
                    ./context/src/Context_metrics.cpp
                    ./context/src/Bookmarks.cpp)
target_include_directories(context PUBLIC ./)
target_link_libraries(context eigen glm csv)
