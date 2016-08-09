find_package(DCMTK REQUIRED)

include_directories(${DCMTK_INCLUDE_DIRS})

add_definitions(-DHAVE_CONFIG_H)

file(GLOB_RECURSE files "*.cc")

add_custom_target(
    TestTools ${CMAKE_COMMAND} -E echo "Test tools" SOURCES ${files})
