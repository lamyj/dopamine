find_package(Boost REQUIRED COMPONENTS filesystem system)
find_package(LDAP REQUIRED)
find_package(Log4Cpp REQUIRED)
find_package(MongoClient REQUIRED)
find_package(Odil REQUIRED)

file(GLOB_RECURSE sources "dopamine/*.cpp")
file(GLOB_RECURSE headers "dopamine/*.h")
file(GLOB_RECURSE templates "dopamine/*.txx")
list(SORT sources)
list(SORT headers)
list(SORT templates)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS} ${LDAP_INCLUDE_DIRS}
    ${Log4Cpp_INCLUDE_DIRS} ${MongoClient_INCLUDE_DIRS} ${Odil_INCLUDE_DIRS})

link_directories(
    ${Boost_LIBRARY_DIRS} ${LDAP_LIBRARY_DIRS} ${Log4Cpp_LIBRARY_DIRS} 
    ${MongoClient_LIBRARY_DIRS} ${Odil_LIBRARY_DIRS})

add_library(libdopamine SHARED ${sources} ${headers} ${templates})
set_target_properties(libdopamine PROPERTIES OUTPUT_NAME dopamine)
target_link_libraries(
    libdopamine ${Boost_LIBRARIES} ${LDAP_LIBRARIES} ${Log4Cpp_LIBRARIES} 
        ${MongoClient_LIBRARIES} ${Odil_LIBRARIES})
set_target_properties(libdopamine PROPERTIES
    VERSION ${dopamine_VERSION} 
    SOVERSION ${dopamine_MAJOR_VERSION})

install(
    DIRECTORY dopamine/ DESTINATION include/dopamine
    FILES_MATCHING PATTERN "*.h" PATTERN "*.txx")
install(
    TARGETS libdopamine
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)
