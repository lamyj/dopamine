find_package(DCMTK REQUIRED)
find_package(MongoDB REQUIRED)

include_directories(${DCMTK_INCLUDE_DIRS} ${MONGO_DB_INCLUDE_DIRS})

add_definitions(-DBOOST_FILESYSTEM_VERSION=3)
add_definitions(-DHAVE_CONFIG_H)
add_library(libresearch_pacs SHARED
    "AlwaysTrue.cpp" "And.cpp" "IsPrivateTag.cpp" "Not.cpp" "Or.cpp" "TagMatch.cpp" "VRMatch.cpp"
    "DataSetToBSON.cpp" "BSONToDataSet.cpp"
    "DcmQueryRetrieveSCP.cpp" "FindResponseGenerator.cpp" "AuthenticatorCSV.cpp")
set_target_properties(libresearch_pacs PROPERTIES OUTPUT_NAME research_pacs)
target_link_libraries(libresearch_pacs
    ${MONGO_DB_LIBRARIES} ${DCMTK_LIBRARIES}
    wrap pthread)
