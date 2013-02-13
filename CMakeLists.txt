cmake_minimum_required(VERSION 2.8)

project(research_pacs)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})
add_subdirectory("lib")

find_package(GDCM REQUIRED)
include_directories(${GDCM_INCLUDE_DIRS} lib)
add_definitions(-DHAVE_CONFIG_H)
add_executable(test "test.cpp")
target_link_libraries(test research_pacs ${GDCM_LIBRARIES})
