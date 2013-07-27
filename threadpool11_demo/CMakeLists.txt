include_directories(${threadpool11_SOURCE_DIR}/threadpool11/include)
link_directories(${threadpool11_BINARY_DIR}/threadpool11/bin)

add_executable(threadpool11_demo src/main.cpp)

target_link_libraries(threadpool11_demo threadpool11)

