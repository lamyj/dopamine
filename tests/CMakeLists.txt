
add_subdirectory(tools)

find_package(DCMTK REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)
find_package(Qt4 REQUIRED QtCore QtGui)
include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/lib
                    ${QT_INCLUDES}
                    ${MONGO_DB_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_definitions(-DHAVE_CONFIG_H)
ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)

file(GLOB_RECURSE tests *.cpp)
foreach(test_file ${tests})
    get_filename_component(test ${test_file} NAME_WE)
    add_executable(${test} ${test_file})
    target_link_libraries(${test} libdopamine ${Boost_LIBRARIES}
                                              ${MONGO_DB_LIBRARIES}
                                              ${QT_LIBRARIES})
    file(READ ${test_file} content)
    
    set(pattern "BOOST_(AUTO|FIXTURE)_TEST_CASE\\(([^),]+)")
    
    string(REGEX MATCHALL ${pattern} cases ${content})
    
    foreach(case ${cases})
        string(REGEX REPLACE ${pattern} "\\2" case ${case})
        add_test("${test}_${case}" "${test}" "--run_test=${case}")
    endforeach()
endforeach()
