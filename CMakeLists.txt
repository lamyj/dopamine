cmake_minimum_required(VERSION 2.8)

project(research_pacs)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})
add_subdirectory("lib")

find_package(GDCM REQUIRED)
find_package(MongoDB REQUIRED)
include_directories(${GDCM_INCLUDE_DIRS} lib)
add_executable(test "test.cpp")
target_link_libraries(test libresearch_pacs ${GDCM_LIBRARIES} ${MONGO_DB_LIBRARIES})
