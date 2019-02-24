find_package(GTest REQUIRED)

include_directories("${threadpool11_SOURCE_DIR}/threadpool11/include" ${GTEST_INCLUDE_DIRS})

add_executable(threadpool11_test threadpool11_test.cpp)
target_link_libraries(threadpool11_test threadpool11 ${GTEST_BOTH_LIBRARIES})
add_test(threadpool11_test threadpool11_test)
