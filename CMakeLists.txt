project(dopamine)
cmake_minimum_required(VERSION 2.6.0)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

option(BUILD_EXAMPLES "Build the examples directory." ON)
set(dopamine_MAJOR_VERSION 0)
set(dopamine_MINOR_VERSION 5)
set(dopamine_PATCH_VERSION 2)
set(dopamine_VERSION
  ${dopamine_MAJOR_VERSION}.${dopamine_MINOR_VERSION}.${dopamine_PATCH_VERSION})

include(CTest)

add_definitions(-std=c++0x)

add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

add_subdirectory(configuration)
