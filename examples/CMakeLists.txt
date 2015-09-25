add_definitions("-D HAVE_CONFIG_H")

include_directories(${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_SOURCE_DIR}/src/lib)

set(examples request)

foreach(example ${examples})
    add_executable(${example} ${example}.cpp)
    target_link_libraries(${example} libdopamine)
endforeach()
