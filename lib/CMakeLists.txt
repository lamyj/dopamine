find_package(DCMTK REQUIRED)
find_package(GDCM REQUIRED)
find_package(MongoDB REQUIRED)

set(GDCM_LIBRARIES "gdcmMSFF")

include_directories(${DCMTK_INCLUDE_DIRS} ${GDCM_INCLUDE_DIRS} ${MONGO_DB_INCLUDE_DIRS})

add_definitions(-DHAVE_CONFIG_H)
add_library(research_pacs SHARED
    "DataSetToBSON.cpp" "BSONToDataSet.cpp" "GenericSCP.cpp")
target_link_libraries(research_pacs
    ${GDCM_LIBRARIES} ${MONGO_DB_LIBRARIES} ${DCMTK_LIBRARIES}
    dcmdata oflog
    wrap)
