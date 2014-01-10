ADD_DEFINITIONS(-Dthreadpool11_EXPORTING)

INCLUDE_DIRECTORIES(include)

if(CMAKE_CXX_COMPILER_ID MATCHES GNU OR 
    CMAKE_CXX_COMPILER_ID MATCHES Clang)
	ADD_LIBRARY(threadpool11 STATIC src/Pool.cpp src/Worker.cpp)
	TARGET_LINK_LIBRARIES(threadpool11 pthread)
elseif(MSVC)
	ADD_LIBRARY(threadpool11 SHARED src/Pool.cpp src/Worker.cpp)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
INSTALL(FILES include/threadpool11/threadpool11.h DESTINATION include/threadpool11)
INSTALL(FILES include/threadpool11/Worker.h DESTINATION include/threadpool11)
INSTALL(FILES include/threadpool11/Pool.h DESTINATION include/threadpool11)
INSTALL(TARGETS threadpool11
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
endif()
