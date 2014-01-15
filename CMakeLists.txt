cmake_minimum_required(VERSION 2.8)
project(threadpool11)

# SET a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # SET the possible values of build type for cmake-gui
endif()

if(Boost_FOUND OR BOOST_ROOT)
  include_directories(${Boost_INCLUDE_DIR})
else()
  include_directories("ext/include")
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug" "Perf")

if(CMAKE_CXX_COMPILER_ID MATCHES GNU OR
    CMAKE_CXX_COMPILER_ID MATCHES Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
  set(CMAKE_CXX_FLAGS_PERF "${CMAKE_CXX_FLAGS_RELEASE} -Wno-inline -pg")
  set(CMAKE_STATIC_LINKER_FLAGS_PERF "${CMAKE_LINKER_FLAGS_RELEASE} -pg")
elseif(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE} /GL")
  set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /LTCG")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)

add_subdirectory(threadpool11)
add_subdirectory(threadpool11_demo)

file(COPY ./threadpool11/include DESTINATION ${CMAKE_BINARY_DIR}/build)
