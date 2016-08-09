find_package(JsonCpp REQUIRED)
find_package(Log4Cpp REQUIRED)
find_package(MongoDB REQUIRED)
find_package(Odil REQUIRED)

file(GLOB_RECURSE files "dopamine/*.cpp")
file(GLOB_RECURSE headers "dopamine/*.h")
file(GLOB_RECURSE templates "dopamine/*.txx")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${JsonCpp_INCLUDE_DIRS} ${Log4Cpp_INCLUDE_DIRS} ${MongoDB_INCLUDE_DIRS}
    ${Odil_INCLUDE_DIRS})

add_library(libdopamine SHARED ${files} ${headers} ${templates})
set_target_properties(libdopamine PROPERTIES OUTPUT_NAME dopamine)

target_link_libraries(
    libdopamine ${JsonCpp_LIBRARIES} ${Log4Cpp_LIBRARIES} ${MongoDB_LIBRARIES}
    ${Odil_LIBRARIES} ldap)

install(
    DIRECTORY dopamine/ DESTINATION include/
    FILES_MATCHING PATTERN "*.h" PATTERN "*.txx")
install(
    TARGETS libdopamine
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

file(
    GLOB_RECURSE files
    "authenticator/*" "ConverterBSON/*" "core/*" "dbconnection/*" "services/*")
add_custom_target(
    libdopamineOLD
    ${CMAKE_COMMAND} -E echo "Old dopamine library" SOURCES ${files})
