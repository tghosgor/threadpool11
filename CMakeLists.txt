cmake_minimum_required (VERSION 2.8)
project (threadpool11)
set (version_major 0)
set (version_minor 1)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)

add_subdirectory(threadpool11)
add_subdirectory(threadpool11_demo)

file(COPY ./threadpool11/include DESTINATION ./build)

