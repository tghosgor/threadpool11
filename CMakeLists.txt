cmake_minimum_required(VERSION 2.8)
project(threadpool11)

set(CMAKE_VERBOSE_MAKEFILE on)

# SET a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # SET the possible values of build type for cmake-gui
endif()

find_package(Boost)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIR})
else()
  include_directories("ext/include")
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug" "Perf")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_EXE_LINK_FLAGS_RELEASE} -O2")

  set(CMAKE_CXX_FLAGS_PERF "${CMAKE_CXX_FLAGS_RELEASE} -Wno-inline -pg")

  set(CMAKE_STATIC_LINKER_FLAGS_PERF "${CMAKE_LINKER_FLAGS_RELEASE} -pg")

  set(CMAKE_EXE_LINK_FLAGS "${CMAKE_EXE_LINK_FLAGS} -pthread")
  set(CMAKE_EXE_LINK_FLAGS_RELEASE "${CMAKE_EXE_LINK_FLAGS_RELEASE} -pthread -O2")
elseif(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE} /GL")
  set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /LTCG")
endif()

add_subdirectory(threadpool11)
add_subdirectory(threadpool11_demo)
