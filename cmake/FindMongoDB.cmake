# - Try to find MongoDB
# Once done this will define
#  MongoDB_FOUND - System has MongoDB
#  MongoDB_INCLUDE_DIRS - The MongoDB include directories
#  MongoDB_LIBRARIES - The libraries needed to use MongoDB
#  MongoDB_DEFINITIONS - Compiler switches required for using MongoDB

find_path(MongoDB_INCLUDE_DIR "mongo/bson/bson.h")
find_library(MongoDB_LIBRARY NAMES mongoclient)

set(MongoDB_LIBRARIES ${MongoDB_LIBRARY} )
set(MongoDB_INCLUDE_DIRS ${MongoDB_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set MongoDB_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(
    MongoDB DEFAULT_MSG MongoDB_LIBRARY MongoDB_INCLUDE_DIR)
