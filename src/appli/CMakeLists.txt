

find_package(DCMTK REQUIRED)

include_directories(${DCMTK_INCLUDE_DIRS} "../lib")

add_definitions(-DHAVE_CONFIG_H)

add_executable(research_pacs "research_pacs.cpp")
target_link_libraries(research_pacs libresearch_pacs ssl crypt ${DCMTK_LIBRARIES} oflog z )
