find_package(DCMTK REQUIRED)

include_directories(${DCMTK_INCLUDE_DIRS})

add_definitions(-DHAVE_CONFIG_H)

file(GLOB_RECURSE files "*.cc")

add_executable(getscu ${files})
target_link_libraries(getscu ssl crypt ${DCMTK_LIBRARIES} oflog z )
set_target_properties(getscu PROPERTIES COMPILE_DEFINITIONS HAVE_CONFIG_H)
