include(packages_finder)

set(Boost_components filesystem system)
set(packages Boost Odil)

packages_finder("${packages}" include_dirs libraries library_dirs)

include_directories(${CMAKE_SOURCE_DIR}/src/lib ${include_dirs})
link_directories(${library_dirs})

file(GLOB files *.cpp)

foreach(file ${files})
    get_filename_component(name ${file} NAME_WE)

    add_executable(${name} ${file})
    target_link_libraries(${name} ${libraries} libdopamine)
endforeach()
