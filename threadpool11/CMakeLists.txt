add_definitions(-Dthreadpool11_EXPORTING)

include_directories(include)

add_library(threadpool11 STATIC src/pool.cpp src/worker.cpp include/threadpool11/pool.hpp include/threadpool11/worker.hpp include/threadpool11/utility.hpp)

if (CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries(threadpool11 pthread)
endif()

if (UNIX)
    install(FILES include/threadpool11/threadpool11.hpp DESTINATION include/threadpool11)
    install(FILES include/threadpool11/worker.hpp DESTINATION include/threadpool11)
    install(FILES include/threadpool11/pool.hpp DESTINATION include/threadpool11)
    install(FILES include/threadpool11/utility.hpp DESTINATION include/threadpool11)
    install(TARGETS threadpool11 DESTINATION lib)
endif()
