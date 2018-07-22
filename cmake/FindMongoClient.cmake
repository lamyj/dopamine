# - Try to find MongoDB C++ client
# Once done this will define
#  MongoClient_FOUND - System has MongoDB C++ client
#  MongoClient_INCLUDE_DIRS - The MongoDB C++ client include directories
#  MongoClient_LIBRARIES - The libraries needed to use MongoDB C++ client
#  MongoClient_DEFINITIONS - Compiler switches required for using MongoDB C++ client

find_path(MongoClient_INCLUDE_DIR "mongo/client/dbclient.h")
find_library(MongoClient_LIBRARY NAMES mongoclient)

set(MongoClient_LIBRARIES ${MongoClient_LIBRARY})
set(MongoClient_INCLUDE_DIRS ${MongoClient_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set MongoClient_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(
    MongoClient DEFAULT_MSG MongoClient_LIBRARY MongoClient_INCLUDE_DIR)

mark_as_advanced(MongoClient_INCLUDE_DIR MongoClient_LIBRARY)
