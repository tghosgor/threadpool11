add_definitions(-Dthreadpool11_EXPORTING)

include_directories(./include)

add_library(threadpool11 SHARED src/Pool.cpp src/Worker.cpp)
if(CMAKE_COMPILER_IS_GNUCXX)
	target_link_libraries(threadpool11 pthread)
endif()

