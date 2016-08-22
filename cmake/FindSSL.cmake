# - Try to find SSL
# Once done this will define
#  SSL_FOUND - System has SSL
#  SSL_INCLUDE_DIRS - The SSL include directories
#  SSL_LIBRARIES - The libraries needed to use SSL
#  SSL_DEFINITIONS - Compiler switches required for using SSL

find_package(PkgConfig)
pkg_check_modules(PC_SSL QUIET libssl)
set(SSL_DEFINITIONS ${PC_SSL_CFLAGS_OTHER})

find_path(SSL_INCLUDE_DIR "openssl/ssl.h" HINTS ${PC_SSL_INCLUDE_DIRS})
find_library(SSL_LIBRARY NAMES ssl HINTS ${PC_SSL_LIBRARY_DIRS} )

set(SSL_LIBRARIES ${SSL_LIBRARY} crypto)
set(SSL_INCLUDE_DIRS ${SSL_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set SSL_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(
    SSL DEFAULT_MSG SSL_LIBRARY SSL_INCLUDE_DIR)

mark_as_advanced(SSL_INCLUDE_DIR SSL_LIBRARY)
