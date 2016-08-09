file(GLOB_RECURSE files "*cpp")
add_custom_target(
    dopamineOLD_APPS
    ${CMAKE_COMMAND} -E echo "Old dopamine apps" SOURCES ${files})
