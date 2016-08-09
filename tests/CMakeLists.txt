add_subdirectory(tools)

find_package(Boost COMPONENTS unit_test_framework system thread REQUIRED)
find_package(MongoDB REQUIRED)
find_package(Odil REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/lib ${Boost_INCLUDE_DIRS} ${MongoDB_INCLUDE_DIRS}
    ${Odil_INCLUDE_DIRS})
add_definitions(-DBOOST_TEST_DYN_LINK)
link_directories(${Boost_LIBRARY_DIRS})

file(GLOB_RECURSE tests code/*.cpp)
set(old_tests)

foreach(test_file ${tests})
    if(test_file MATCHES "${CMAKE_CURRENT_SOURCE_DIR}/code/(core|dbconnection|services)")
        set(old_tests ${old_tests} ${test_file})
    else()
        get_filename_component(test ${test_file} NAME_WE)

        add_executable(${test} ${test_file})
        target_link_libraries(
            ${test} libdopamine ${Boost_LIBRARIES} ${MongoDB_LIBRARIES}
            ${Odil_LIBRARIES} ssl)

        file(READ ${test_file} content)

        set(pattern "BOOST_(AUTO|FIXTURE)_TEST_CASE\\(([^),]+)")

        string(REGEX MATCHALL ${pattern} cases ${content})

        foreach(case ${cases})
            string(REGEX REPLACE ${pattern} "\\2" case ${case})
            if(NOT ${case} MATCHES "##")
                add_test("${test}_${case}" "${test}" "--run_test=${case}")
            endif()
        endforeach()
    endif()
endforeach()

add_custom_target(
    Runner ${CMAKE_COMMAND} -E echo "Test runner" SOURCES run_tests.sh)

add_custom_target(
    OldTests ${CMAKE_COMMAND} -E echo "Old tests" SOURCES ${old_tests})
