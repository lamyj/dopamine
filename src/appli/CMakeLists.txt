include(packages_finder)

set(packages Odil)

packages_finder("${packages}" include_dirs libraries library_dirs)

include_directories(
    ${CMAKE_SOURCE_DIR}/src/lib
    ${CMAKE_SOURCE_DIR}/src/mongo-cxx-driver/src
    ${CMAKE_BINARY_DIR}/src/mongo-cxx-driver/src
    ${include_dirs})
link_directories(${library_dirs})

file(GLOB files *.cpp)
set(old_files)

foreach(file ${files})
    if(NOT file MATCHES "${CMAKE_CURRENT_SOURCE_DIR}/dopamine.cpp")
        set(old_files ${old_files} ${file})
    else()
        get_filename_component(name ${file} NAME_WE)

        add_executable(${name} ${file})
        target_link_libraries(${name} ${libraries} libdopamine)
    endif()
endforeach()

add_custom_target(
    dopamineOLD_APPS
    ${CMAKE_COMMAND} -E echo "Old dopamine apps" SOURCES ${old_files})
