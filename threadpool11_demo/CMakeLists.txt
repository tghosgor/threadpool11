include_directories(${threadpool11_SOURCE_DIR}/threadpool11/include)

add_executable(threadpool11_demo src/main.cpp)
add_executable(threadpool11_tests src/tests.cpp)
add_executable(threadpool11_openmp src/openmp.cpp)

set_target_properties(threadpool11_openmp PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -fopenmp"
                                                     LINK_FLAGS "${LINK_FLAGS} -fopenmp")

target_link_libraries(threadpool11_demo threadpool11_lib)
target_link_libraries(threadpool11_tests threadpool11_lib)
target_link_libraries(threadpool11_openmp threadpool11_lib)
