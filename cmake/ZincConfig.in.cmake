#
# - Zinc Config File.
# Defines:
#
#  ZINC_INCLUDE_DIRS - Where to find the Zinc header files
#  ZINC_LIBRARIES    - List of libraries for using Zinc
#  ZINC_DEFINITIONS  - List of definitions that *must* be set
#  ZINC_FOUND        - Set to TRUE to conform like other FIND_PACKAGE calls
#
# The ZINC_DEFINITIONS need to be added to your project like so:
#
#  FOREACH(DEF ${ZINC_DEFINITIONS}
#      ADD_DEFINTIONS( -D${DEF} )
#  ENDFORECH(DEF ${ZINC_DEFINITIONS}
#
# The static version of the library can be found by setting
# ZINC_USE_STATIC true (if it is available).
#
GET_FILENAME_COMPONENT( SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH )
IF(EXISTS "${SELF_DIR}/CMakeCache.txt")
    # In build tree
    INCLUDE("${SELF_DIR}/ZincBuildTreeSettings.cmake")
ELSE()
    GET_FILENAME_COMPONENT( ZINC_INCLUDE_DIRS "${SELF_DIR}/@CONF_REL_INCLUDE_DIR@" ABSOLUTE )
ENDIF()

#INCLUDE( \${SELF_DIR}/${ZINC_LIB_GROUP}-targets.cmake )
# Our library dependencies (contains definitions for IMPORTED targets)
IF( NOT DEFINED _ZINC_CONFIG_CMAKE )
SET( _ZINC_CONFIG_CMAKE TRUE )
INCLUDE( ${SELF_DIR}/ZincLibraryDepends.cmake )
ENDIF( NOT DEFINED _ZINC_CONFIG_CMAKE )
INCLUDE( ${SELF_DIR}/ZincLibrarySetup.cmake )

#SET( ZINC_INCLUDE_DIRS ${ZINC_INCLUDE_DIRS} ${INCLUDE_DIRS} )
SET( ZINC_LIBRARIES ${ZINC_LIBRARY} )
#SET( ZINC_DEFINITIONS ${DEPENDENT_DEFINITIONS} ${EXTRA_COMPILER_DEFINITIONS} )

SET( ZINC_FOUND TRUE )

