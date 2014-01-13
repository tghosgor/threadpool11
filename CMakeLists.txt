cmake_minimum_required(VERSION 2.8)
project(threadpool11)

# SET a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # SET the possible values of build type for cmake-gui
endif()

SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug" "Perf")

if(CMAKE_CXX_COMPILER_ID MATCHES GNU OR
    CMAKE_CXX_COMPILER_ID MATCHES Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
  set(CMAKE_CXX_FLAGS_PERF "${CMAKE_CXX_FLAGS_RELEASE} -Wno-inline -pg")
  set(CMAKE_STATIC_LINKER_FLAGS_PERF "${CMAKE_LINKER_FLAGS_RELEASE} -pg")
elseif(MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "/Od")
  set(CMAKE_CXX_FLAGS_RELEASE "/O3 /GL")
  set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "/LTCG")
endif()

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)

ADD_SUBDIRECTORY(threadpool11)
ADD_SUBDIRECTORY(threadpool11_demo)

FILE(COPY ./threadpool11/include DESTINATION ${CMAKE_BINARY_DIR}/build)
