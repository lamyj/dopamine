find_package(Boost COMPONENTS filesystem system regex random REQUIRED)
find_package(ICU REQUIRED)
find_package(JsonCpp REQUIRED)
find_package(MongoDB REQUIRED)
find_package(Odil REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS} ${ICU_INCLUDE_DIRS} ${JsonCpp_INCLUDE_DIR}
    ${MONGO_DB_INCLUDE_DIRS} ${Odil_INCLUDE_DIR})

add_definitions(-DBOOST_FILESYSTEM_VERSION=3)
add_definitions(-DHAVE_CONFIG_H)

file(GLOB_RECURSE files "*.cpp")
file(GLOB_RECURSE headers "*.h")
file(GLOB_RECURSE templates "*.txx")
add_library(libdopamine SHARED ${files} ${headers} ${templates})

set_target_properties(libdopamine PROPERTIES OUTPUT_NAME dopamine)
target_link_libraries(
    libdopamine
    ${Boost_LIBRARIES} boost_random boost_regex ${ICU_LIBRARIES}
    ${JsonCpp_LIBRARY} ${MONGO_DB_LIBRARIES} ${Odil_LIBRARY}
    mimetic log4cpp ldap ssl)

# deploy the library
install(TARGETS libdopamine
        LIBRARY DESTINATION lib
        COMPONENT libraries)

# copy all header files to the include directory
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                         GROUP_READ GROUP_EXECUTE
                         WORLD_READ WORLD_EXECUTE
        DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                              GROUP_READ GROUP_EXECUTE
                              WORLD_READ WORLD_EXECUTE
        DESTINATION include/dopamine
        FILES_MATCHING PATTERN "*.h")
