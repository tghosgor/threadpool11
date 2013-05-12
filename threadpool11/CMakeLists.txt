add_definitions(-Dthreadpool11_EXPORTING)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

include_directories(./include)

add_library(threadpool11 SHARED src/Pool.cpp src/Worker.cpp)
target_link_libraries(threadpool11 pthread)

