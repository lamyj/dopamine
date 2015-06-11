find_package(DCMTK REQUIRED)
find_package(MongoDB REQUIRED)
find_package(Boost COMPONENTS filesystem system regex random REQUIRED)

# libraries ICU for character set management
SET(ICU_LIBRARIES icuuc icuio)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${DCMTK_INCLUDE_DIRS}
                    ${MONGO_DB_INCLUDE_DIRS}
                    ${Boost_INCLUDE_DIRS}
                    ${ICU_INCLUDE_DIRS})

add_definitions(-DBOOST_FILESYSTEM_VERSION=3)
add_definitions(-DHAVE_CONFIG_H)

file(GLOB_RECURSE files "*.cpp")
add_library(libdopamine SHARED ${files})

set_target_properties(libdopamine PROPERTIES OUTPUT_NAME dopamine)
target_link_libraries(libdopamine ${MONGO_DB_LIBRARIES}
                                  ${DCMTK_LIBRARIES}
                                  ${Boost_LIBRARIES}
                                  ${ICU_LIBRARIES}
                                  mimetic
                                  log4cpp ldap dcmtls ssl
                                  wrap pthread)

# deploy the library
install(TARGETS libdopamine
        LIBRARY DESTINATION /usr/lib
        COMPONENT libraries)

# copy all header files to the include directory
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                         GROUP_READ GROUP_EXECUTE
                         WORLD_READ WORLD_EXECUTE
        DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                              GROUP_READ GROUP_EXECUTE
                              WORLD_READ WORLD_EXECUTE
        DESTINATION /usr/include/dopamine/
        FILES_MATCHING PATTERN "*.h")
