add_subdirectory(tools)

include(packages_finder)

set(packages Boost MongoClient Odil)
set(Boost_components filesystem system unit_test_framework)

packages_finder("${packages}" include_dirs libraries library_dirs)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/lib ${CMAKE_CURRENT_SOURCE_DIR} ${include_dirs})
add_definitions(-DBOOST_TEST_DYN_LINK)
link_directories(${library_dirs})

file(GLOB_RECURSE tests code/*.cpp)

file(GLOB_RECURSE fixtures fixtures/*.cpp)
add_library(fixtures ${fixtures})

foreach(test_file ${tests})
    get_filename_component(test ${test_file} NAME_WE)

    add_executable(${test} ${test_file})
    target_link_libraries(${test} libdopamine fixtures pthread ${libraries})

    file(READ ${test_file} content)

    set(pattern "BOOST_(AUTO|FIXTURE)_TEST_CASE\\(([^),]+)")

    string(REGEX MATCHALL ${pattern} cases ${content})

    foreach(case ${cases})
        string(REGEX REPLACE ${pattern} "\\2" case ${case})
        if(NOT ${case} MATCHES "##")
            add_test("${test}_${case}" "${test}" "--run_test=${case}")
        endif()
    endforeach()
endforeach()
