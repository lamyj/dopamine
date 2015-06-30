# install configuration
file(GLOB_RECURSE conffile RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.ini")
foreach(HEADER ${conffile})
    string(REGEX MATCH "(.*)[/\\]" DIR ${HEADER})
    install(FILES ${HEADER}
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
            DESTINATION etc/dopamine/${DIR}
            COMPONENT headers)
endforeach(HEADER ${conffile})
