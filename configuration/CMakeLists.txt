# install configuration
file(GLOB_RECURSE conffile RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.ini")
add_custom_target(
    ConfigurationFiles ${CMAKE_COMMAND} -E echo "Configuration files" SOURCES ${conffile})

install(FILES dopamine.ini DESTINATION ${CMAKE_INSTALL_PREFIX}/etc)
