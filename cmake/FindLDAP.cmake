# - Try to find LDAP
# Once done this will define
#  LDAP_FOUND - System has LDAP
#  LDAP_INCLUDE_DIRS - The LDAP include directories
#  LDAP_LIBRARIES - The libraries needed to use LDAP
#  LDAP_DEFINITIONS - Compiler switches required for using LDAP

find_path(LDAP_INCLUDE_DIR "ldap.h")
find_library(LDAP_LIBRARY NAMES ldap)

set(LDAP_LIBRARIES ${LDAP_LIBRARY})
set(LDAP_INCLUDE_DIRS ${LDAP_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LDAP_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(
    LDAP DEFAULT_MSG LDAP_LIBRARY LDAP_INCLUDE_DIR)

mark_as_advanced(LDAP_INCLUDE_DIR LDAP_LIBRARY)
