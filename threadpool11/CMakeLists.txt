add_definitions(-Dthreadpool11_EXPORTING)

include_directories(include)

add_library(threadpool11_lib STATIC src/pool.cpp src/worker.cpp include/threadpool11/pool.hpp include/threadpool11/worker.hpp include/threadpool11/utility.hpp)

if (MAKE_CXX_COMPILER_ID MATCHES GNU OR CMAKE_CXX_COMPILER_ID MATCHES Clang)
    target_link_libraries(threadpool11_lib pthread)
endif()

if (MAKE_CXX_COMPILER_ID MATCHES GNU OR CMAKE_CXX_COMPILER_ID MATCHES Clang)
    install(FILES include/threadpool11/threadpool11.hpp DESTINATION include/threadpool11)
    install(FILES include/threadpool11/worker.hpp DESTINATION include/threadpool11)
    install(FILES include/threadpool11/pool.hpp DESTINATION include/threadpool11)
    install(FILES include/threadpool11/utility.hpp DESTINATION include/threadpool11)
    install(TARGETS threadpool11_lib LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
endif()
