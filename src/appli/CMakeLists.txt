find_package(DCMTK REQUIRED)

include_directories(${DCMTK_INCLUDE_DIRS}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../lib)


add_executable(dopamine "dopamine.cpp")
target_link_libraries(dopamine libdopamine log4cpp ssl
                               crypt ${DCMTK_LIBRARIES} oflog z )
set_target_properties(dopamine PROPERTIES COMPILE_DEFINITIONS HAVE_CONFIG_H)

add_executable(dopamine-adm "dopamine-adm.cpp")
target_link_libraries(dopamine-adm libdopamine)
set_target_properties(dopamine-adm PROPERTIES COMPILE_DEFINITIONS HAVE_CONFIG_H)

add_executable(wado-rs "wado-rs.cpp")
target_link_libraries(wado-rs cgicc libdopamine)

add_executable(wado-uri "wado-uri.cpp")
target_link_libraries(wado-uri cgicc libdopamine ssl crypt oflog z )

add_executable(qido-rs "qido-rs.cpp")
target_link_libraries(qido-rs cgicc libdopamine)

add_executable(stow-rs "stow-rs.cpp")
target_link_libraries(stow-rs cgicc libdopamine)

install(TARGETS dopamine wado-rs wado-uri qido-rs stow-rs
        RUNTIME DESTINATION bin
        COMPONENT applications)

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
