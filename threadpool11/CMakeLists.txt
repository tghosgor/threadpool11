add_definitions(-Dthreadpool11_EXPORTING)

include_directories(include)

add_library(threadpool11_lib STATIC src/Pool.cpp src/Worker.cpp include/threadpool11/Pool.h include/threadpool11/Worker.h include/threadpool11/Helper.hpp)

if(MAKE_CXX_COMPILER_ID MATCHES GNU OR
    CMAKE_CXX_COMPILER_ID MATCHES Clang)
  target_link_libraries(threadpool11_lib pthread)
endif()

if(MAKE_CXX_COMPILER_ID MATCHES GNU OR
    CMAKE_CXX_COMPILER_ID MATCHES Clang)
install(FILES include/threadpool11/threadpool11.h DESTINATION include/threadpool11)
install(FILES include/threadpool11/Worker.h DESTINATION include/threadpool11)
install(FILES include/threadpool11/Pool.h DESTINATION include/threadpool11)
install(FILES include/threadpool11/Helper.hpp DESTINATION include/threadpool11)
install(TARGETS threadpool11_lib
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
endif()
