add_subdirectory(tools)

find_package(Boost REQUIRED COMPONENTS system unit_test_framework)
find_package(Log4Cpp REQUIRED)
find_package(MongoClient REQUIRED)
find_package(Odil REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/lib ${CMAKE_CURRENT_SOURCE_DIR} 
    ${Boost_INCLUDE_DIRS} ${Log4Cpp_INCLUDE_DIRS} ${MongoClient_INCLUDE_DIRS} 
    ${Odil_INCLUDE_DIRS})
add_definitions(-DBOOST_TEST_DYN_LINK)
link_directories(
    ${Boost_LIBRARY_DIRS} ${Log4Cpp_LIBRARY_DIRS} ${MongoClient_LIBRARY_DIRS}
    ${Odil_LIBRARY_DIRS})

file(GLOB_RECURSE tests code/*.cpp)

file(GLOB_RECURSE fixtures fixtures/*.cpp)
add_library(fixtures ${fixtures})

foreach(test_file ${tests})
    get_filename_component(test ${test_file} NAME_WE)

    add_executable(${test} ${test_file})
    target_link_libraries(
        ${test} 
        libdopamine fixtures pthread 
        ${Boost_LIBRARIES} ${Log4Cpp_LIBRARIES} ${MongoClient_LIBRARIES} 
        ${Odil_LIBRARIES})

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
