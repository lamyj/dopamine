find_package(DCMTK REQUIRED)

include_directories(${DCMTK_INCLUDE_DIRS}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

add_definitions(-DHAVE_CONFIG_H)

add_executable(research_pacs "research_pacs.cpp")
target_link_libraries(research_pacs libresearch_pacs log4cpp ssl crypt ${DCMTK_LIBRARIES} oflog z )

install(TARGETS research_pacs
        RUNTIME DESTINATION /usr/bin/
        COMPONENT applications)

# copy all header files to the include directory
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        DESTINATION /usr/include/research_pacs/
        FILES_MATCHING PATTERN "*.h")
