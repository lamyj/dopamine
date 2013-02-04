find_path(MONGO_DB_INCLUDE_DIR mongo/bson/bson.h)
find_library(MONGO_DB_CLIENT NAMES mongoclient)

find_package(Boost REQUIRED COMPONENTS filesystem system thread)
set(MONGO_DB_INCLUDE_DIRS ${MONGO_DB_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
set(MONGO_DB_LIBRARIES ${MONGO_DB_CLIENT} ${Boost_LIBRARIES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MongoDB DEFAULT_MSG 
    MONGO_DB_INCLUDE_DIRS MONGO_DB_LIBRARIES)