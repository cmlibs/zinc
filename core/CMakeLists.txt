# OpenCMISS-Zinc Library
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Set the minimum version of cmake required to 2.8.1
# The FATAL_ERROR is needed if using cmake < 2.4

# This is the project name and shows up in ides
project(ZincCore)

IF( MSVC )
	SET( ZINC_C_INLINE __inline )
ELSE()
	SET( ZINC_C_INLINE "static inline" )
ENDIF()

set( ZINC_CONFIGURE_HDR ${PROJECT_BINARY_DIR}/source/api/zinc/zincconfigure.h )
set( ZINC_SHARED_OBJECT_HDR ${PROJECT_BINARY_DIR}/source/api/zinc/zincsharedobject.h )
configure_file( ${PROJECT_SOURCE_DIR}/source/configure/zincconfigure.h.cmake
	${ZINC_CONFIGURE_HDR} )
configure_file( ${PROJECT_SOURCE_DIR}/source/configure/zincsharedobject.h.cmake
	${ZINC_SHARED_OBJECT_HDR} )
set( ZINC_CONFIGURE_HDRS ${ZINC_CONFIGURE_HDR} ${ZINC_SHARED_OBJECT_HDR} )

# Load in source files, each source file list may contain several
# CMake variables see comments for a list of _SRCS included from
# a particular file (similarly that file defines a _HDRS variable).
# Defines API_HDRS, API_TYPES_HDRS
include(source/cmake/api_source_files.cmake )

# Defines APIPP_HDRS, APIPP_TYPES_HDRS
include(source/cmake/apipp_source_files.cmake )

# Defines COMMAND_SRCS, CONTEXT_SRCS
include(source/cmake/command_source_files.cmake )

# Defines COMPUTED_FIELD_CORE_SRCS, COMPUTED_FIELD_GRAPHICS_SRCS,
# COMPUTED_FIELD_SRCS (contains previous two)
include(source/cmake/computed_field_source_files.cmake )

# Defines FINITE_ELEMENT_CORE_SRCS, FINITE_ELEMENT_GRAPHICS_SRCS,
# FINITE_ELEMENT_ADDITIONAL_SRCS, FINITE_ELEMENT_SRCS
# (group definition which includes the previous three)
include(source/cmake/finite_element_source_files.cmake )

# Defines GENERAL_SRCS
include(source/cmake/general_source_files.cmake )

# Defines GRAPHICS_SRCS
include(source/cmake/graphics_source_files.cmake )

# Defines IMAGE_PROCESSING_SRCS
include(source/cmake/image_processing_source_files.cmake )

# Defines COMFILE_SRCS, CURVE_SRCS, ELEMENT_SRCS, EMOTER_SRCS
# FIELD_IO_SRCS, INTERACTION_SRCS, IO_DEVICES_SRCS, MESH_SRCS, NODE_SRCS,
# REGION_SRCS, SELECTION_SRCS, THREE_D_DRAWING_SRCS, TIME_SRCS
include(source/cmake/misc_source_files.cmake )

# Defines OPENCASCADE_SRCS
#include(source/cmake/opencascade_source_files.cmake )

# Defines ZINC_LIB_SRCS, ZINC_LIB_HDRS
include(source/cmake/zinc_lib_source_files.cmake)

# Generate the version.cpp / version.h files
set(ZINC_VERSION_SRC ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/source/configure/version.cpp.cmake
	${ZINC_VERSION_SRC})
list(APPEND ZINC_LIB_SRCS ${ZINC_VERSION_SRC})

#print_list_duplications( ZINC_LIB_SRCS )
#print_list_duplications( ZINC_LIB_HDRS )

include_directories( source source/api ${CMAKE_CURRENT_BINARY_DIR}/source
	${CMAKE_CURRENT_BINARY_DIR}/source/api ${INCLUDE_DIRS})

if( MSVC_IDE )
	# I want to group the source files by the directory layout for Visual Studio
	GROUP_SOURCE_FILES(${ZINC_LIB_SRCS} ${ZINC_LIB_HDRS} ${ZINC_CONFIGURE_HDRS})
	add_definitions( "/wd4244 /nologo" )
endif()

set(ZINC_LIBRARY_NAME zinc)
if( ZINC_BUILD_SHARED_LIBRARY )
	set( ZINC_SHARED_TARGET ${ZINC_LIBRARY_NAME} )
	add_library(${ZINC_SHARED_TARGET} SHARED ${ZINC_LIB_SRCS} ${ZINC_LIB_HDRS} ${ZINC_CONFIGURE_HDRS})
	list( APPEND ZINC_SHARED_DEFINITIONS -DZINC_SHARED_OBJECT )
	set( ZINC_SHARED_DEFINITIONS ${ZINC_SHARED_DEFINITIONS} PARENT_SCOPE )
	target_link_libraries(${ZINC_LIBRARY_NAME} PRIVATE ${DEPENDENT_LIBS})
	if( WIN32 )
		target_link_libraries(${ZINC_LIBRARY_NAME} PRIVATE ${LINK_PUBLIC} winmm.lib)
	endif()
	set_target_properties( ${ZINC_SHARED_TARGET} PROPERTIES
		COMPILE_DEFINITIONS "ZINC_SHARED_OBJECT"
		DEFINE_SYMBOL "ZINC_EXPORTS")
endif()

if( ZINC_BUILD_STATIC_LIBRARY )
	SET( ZINC_STATIC_TARGET ${ZINC_LIBRARY_NAME}-static )
	add_library( ${ZINC_STATIC_TARGET} STATIC ${ZINC_LIB_SRCS} ${ZINC_LIB_HDRS} ${ZINC_CONFIGURE_HDRS} )
	target_link_libraries(${ZINC_STATIC_TARGET} PUBLIC ${DEPENDENT_LIBS})
	if(WIN32)
		target_link_libraries( ${ZINC_STATIC_TARGET} winmm.lib )
	endif()
	set_target_properties( ${ZINC_STATIC_TARGET} PROPERTIES
		OUTPUT_NAME "${ZINC_LIBRARY_NAME}" PREFIX "lib" )
endif()

set_target_properties( ${ZINC_SHARED_TARGET} ${ZINC_STATIC_TARGET} PROPERTIES
	OUTPUT_NAME_RELEASE ${ZINC_LIBRARY_NAME}${ZINC_DEVELOPER_VERSION}
	OUTPUT_NAME_DEBUG ${ZINC_LIBRARY_NAME}d${ZINC_DEVELOPER_VERSION}
	VERSION ${Zinc_VERSION}
	SOVERSION ${Zinc_VERSION_MAJOR} )
# VERSION property doesn't work on Windows so we will fake it here.
if( WIN32 AND ZINC_BUILD_SHARED_LIBRARY )
	set_target_properties( ${ZINC_SHARED_TARGET} PROPERTIES
		OUTPUT_NAME_RELEASE ${ZINC_SHARED_TARGET}${ZINC_DEVELOPER_VERSION}.${ZINC_VERSION}
		OUTPUT_NAME_DEBUG ${ZINC_SHARED_TARGET}d${ZINC_DEVELOPER_VERSION}.${ZINC_VERSION} )
endif()

# Install rules for Zinc
install(TARGETS
    ${ZINC_SHARED_TARGET} ${ZINC_STATIC_TARGET}
    EXPORT ${ZINC_EXPORT_SET_NAME}
    ARCHIVE DESTINATION ${ZINC_INSTALL_LIB_DIR}
    LIBRARY DESTINATION ${ZINC_INSTALL_LIB_DIR}
    RUNTIME DESTINATION ${ZINC_INSTALL_BIN_DIR}
    INCLUDES DESTINATION ${ZINC_INSTALL_INC_DIR}
)

# Zinc API header install rules
install(FILES
	${ZINC_SHARED_OBJECT_HDR}
	${API_HDRS}
	${APIPP_HDRS}
	DESTINATION ${ZINC_INSTALL_INC_DIR}/zinc )

install(FILES ${API_TYPES_HDRS} ${APIPP_TYPES_HDRS}
	DESTINATION ${ZINC_INSTALL_INC_DIR}/zinc/types)

#if( ZINC_SHARED_TARGET )
#	set( ZINC_SHARED_TARGET ${ZINC_SHARED_TARGET} PARENT_SCOPE )
#endif()
#if( ZINC_STATIC_TARGET )
#	set( ZINC_STATIC_TARGET ${ZINC_STATIC_TARGET} PARENT_SCOPE )
#endif()
