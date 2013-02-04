find_package(GDCM REQUIRED)
find_package(MongoDB REQUIRED)

set(GDCM_LIBRARIES "gdcmMSFF")

include_directories(${GDCM_INCLUDE_DIRS} ${MONGO_DB_INCLUDE_DIRS})

add_library(libresearch_pacs SHARED "DataSetToBSON.cpp" "BSONToDataSet.cpp")
target_link_libraries(libresearch_pacs ${GDCM_LIBRARIES} ${MONGO_DB_LIBRARIES})
