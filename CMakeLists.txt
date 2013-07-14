cmake_minimum_required (VERSION 2.8)
project (threadpool11)
set(version_major 0)
set(version_minor 1)
set(version_patch 0)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "Perf")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "-std=c++11")
	set(CMAKE_CXX_FLAGS_DEBUG "-g")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -flto -fuse-linker-plugin")
	set(CMAKE_CXX_FLAGS_PERF "-O3 -DNDEBUG -flto -fuse-linker-plugin -Wno-inline -pg")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-flto -fuse-linker-plugin") 
	set(CMAKE_SHARED_LINKER_FLAGS_PERF "-flto -fuse-linker-plugin -pg") 
elseif(MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "/Od")
	set(CMAKE_CXX_FLAGS_RELEASE "/O3 /GL")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/LTCG")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/bin)

add_subdirectory(threadpool11)
add_subdirectory(threadpool11_demo)

file(COPY ./threadpool11/include DESTINATION ./build)

