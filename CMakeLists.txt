cmake_minimum_required(VERSION 2.8.0)

project("dopamine")
set(dopamine_MAJOR_VERSION 0)
set(dopamine_MINOR_VERSION 5)
set(dopamine_PATCH_VERSION 2)
set(dopamine_VERSION
  ${dopamine_MAJOR_VERSION}.${dopamine_MINOR_VERSION}.${dopamine_PATCH_VERSION})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(CTest)

# Add the C++0x or C++11 flag
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG(-std=c++11 COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG(-std=c++0x COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

add_subdirectory("src")
add_subdirectory("configuration")

if(BUILD_TESTING)
    add_subdirectory("tests")
endif()
