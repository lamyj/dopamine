find_package(DCMTK REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/lib)
link_directories(${Boost_LIBRARY_DIRS})

add_definitions(-DHAVE_CONFIG_H)
ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)

