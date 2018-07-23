find_package(Boost REQUIRED COMPONENTS filesystem system)
find_package(Log4Cpp REQUIRED)
find_package(MongoClient REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/lib
    ${Boost_INCLUDE_DIRS} ${Log4Cpp_INCLUDE_DIRS} ${MongoClient_INCLUDE_DIRS})
link_directories(
    ${Boost_LIBRARY_DIRS} ${Log4Cpp_LIBRARY_DIRS} ${MongoClient_LIBRARY_DIRS})

add_executable(dopamine dopamine.cpp)
target_link_libraries(
    dopamine 
    ${Boost_LIBRARIES} ${Log4Cpp_LIBRARIES} ${MongoClient_LIBRARIES} 
    libdopamine)

install(TARGETS dopamine DESTINATION bin)
