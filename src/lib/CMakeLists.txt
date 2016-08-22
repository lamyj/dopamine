include(packages_finder)

set(packages JsonCpp LDAP Log4Cpp Odil)
set(Boost_components filesystem system thread)

packages_finder("${packages}" include_dirs libraries library_dirs)

file(GLOB_RECURSE sources "dopamine/*.cpp")
file(GLOB_RECURSE headers "dopamine/*.h")
file(GLOB_RECURSE templates "dopamine/*.txx")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/mongo-cxx-driver/src
    ${CMAKE_BINARY_DIR}/src/mongo-cxx-driver/src
    ${include_dirs})
link_directories(${library_dirs})

add_library(libdopamine SHARED ${sources} ${headers} ${templates})
set_target_properties(libdopamine PROPERTIES OUTPUT_NAME dopamine)
target_link_libraries(libdopamine libdopamine-mongo ${libraries})

install(
    DIRECTORY dopamine/ DESTINATION include/
    FILES_MATCHING PATTERN "*.h" PATTERN "*.txx")
install(
    TARGETS libdopamine
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)
