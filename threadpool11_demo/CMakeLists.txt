include_directories(${threadpool11_SOURCE_DIR}/threadpool11/include)

add_executable(threadpool11_demo EXCLUDE_FROM_ALL src/main.cpp)
add_executable(threadpool11_tests EXCLUDE_FROM_ALL src/tests.cpp)
add_executable(threadpool11_openmp EXCLUDE_FROM_ALL src/openmp.cpp)

set_target_properties(threadpool11_openmp PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fopenmp"
                                                     LINK_FLAGS "${LINK_FLAGS} -fopenmp")

target_link_libraries(threadpool11_demo threadpool11)
target_link_libraries(threadpool11_tests threadpool11)
target_link_libraries(threadpool11_openmp threadpool11)
