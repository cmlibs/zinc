
 # ***** BEGIN LICENSE BLOCK *****
 # Version: MPL 1.1/GPL 2.0/LGPL 2.1
 #
 # The contents of this file are subject to the Mozilla Public License Version
 # 1.1 (the "License"); you may not use this file except in compliance with
 # the License. You may obtain a copy of the License at
 # http://www.mozilla.org/MPL/
 #
 # Software distributed under the License is distributed on an "AS IS" basis,
 # WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 # for the specific language governing rights and limitations under the
 # License.
 #
 # The Original Code is cmgui
 #
 # The Initial Developer of the Original Code is
 # Auckland Uniservices Ltd, Auckland, New Zealand.
 # Portions created by the Initial Developer are Copyright (C) 2011
 # the Initial Developer. All Rights Reserved.
 #
 # Contributor(s): 
 #
 # Alternatively, the contents of this file may be used under the terms of
 # either the GNU General Public License Version 2 or later (the "GPL"), or
 # the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 # in which case the provisions of the GPL or the LGPL are applicable instead
 # of those above. If you wish to allow use of your version of this file only
 # under the terms of either the GPL or the LGPL, and not to allow others to
 # use your version of this file under the terms of the MPL, indicate your
 # decision by deleting the provisions above and replace them with the notice
 # and other provisions required by the GPL or the LGPL. If you do not delete
 # the provisions above, a recipient may use your version of this file under
 # the terms of any one of the MPL, the GPL or the LGPL.
 #
 # ***** END LICENSE BLOCK ***** */

# This is the project name and shows up in ides
PROJECT( ZincUI )

# User interfaces
# wxwidgets - is a config file on win32 and not on configure enabled platforms
IF( WX_USER_INTERFACE )
	IF( WIN32 AND NOT CYGWIN AND NOT MSYS )
		# Using a config file so add this to the dependent configs
		SET( DEPENDENT_CONFIGS "${DEPENDENT_CONFIGS}\nINCLUDE(${wxWidgets_DIR}/wxWidgets-config.cmake)" )
	ELSE( WIN32 AND NOT CYGWIN AND NOT MSYS )
		IF( UNIX OR MSYS )
			IF( ${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug" )
				SET( wxWidgets_USE_DEBUG TRUE )
			ELSE( ${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug" )
				SET( wxWidgets_USE_DEBUG FALSE )
			ENDIF( ${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug" )
			SET( wxWidgets_USE_UNICODE FALSE )
			SET( wxWidgets_USE_UNIVERSAL FALSE )
			SET( wxWidgets_USE_STATIC TRUE )
			FIND_PACKAGE( wxWidgets QUIET COMPONENTS xrc adv html
				gl xml core base )
		ENDIF( UNIX OR MSYS )
	ENDIF( WIN32 AND NOT CYGWIN AND NOT MSYS )
	IF( wxWidgets_FOUND )
		# Order of components here is important when linking static libraries
		SET( USE_WXWIDGETS TRUE )
		MARK_AS_ADVANCED( wxWidgets_DIR )
		DBG_MSG( "C FLAGS: ${wxWidgets_CXX_FLAGS}" )
		DBG_MSG( "LIBS: ${wxWidgets_LIBRARIES}" )
		SET( WXWIDGETS_LIBRARIES ${wxWidgets_LIBRARIES} )
		DBG_MSG( "DEFS: ${wxWidgets_DEFINITIONS}" )
		DBG_MSG( "INC_DIRS: ${wxWidgets_INCLUDE_DIRS}" )
		SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${wxWidgets_CXX_FLAGS}" )
		SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${wxWidgets_LIBRARIES} )
		SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${wxWidgets_INCLUDE_DIRS} )
		SET( EXTRA_COMPILER_DEFINITIONS ${EXTRA_COMPILER_DEFINITIONS} ${wxWidgets_DEFINITIONS} )
		SET( DEPENDENT_DEFINITIONS ${DEPENDENT_DEFINITIONS} ${wxWidgets_DEFINITIONS} )
		SET( DEPENDENT_CONFIGS "${DEPENDENT_CONFIGS}\nSET( wxWidgets_wxrc_EXECUTABLE ${wxWidgets_wxrc_EXECUTABLE} )" )
		IF( MSVC )
			SET( EXTRA_COMPILER_DEFINITIONS ${EXTRA_COMPILER_DEFINITIONS} wxUSE_NO_MANIFEST=1 )
			SET( DEPENDENT_DEFINITIONS ${DEPENDENT_DEFINITIONS} wxUSE_NO_MANIFEST=1 )
		ENDIF( MSVC )
		IF( UNIX AND NOT APPLE ) # A not very good way of getting wxWidgets to compile with gcc 4.6.2
			SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} /usr/lib/libX11.so )
		ENDIF()
		#INCLUDE( source/cmake/xrc_source_files.cmake )
		DBG_MSG( "XRC SRCS: ${XRC_SRCS}" )
		#WXWIDGETS_HDRS( "${XRC_SRCS}" wxWidgets_GENERATED_HDRS )
	ELSE( wxWidgets_FOUND )
		MESSAGE( FATAL_ERROR "wxWidgets was requested but not found." )
	ENDIF( wxWidgets_FOUND )
ENDIF( WX_USER_INTERFACE )

IF( GTK_USER_INTERFACE )
	IF( GTK2_FOUND )
		SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${GTK2_LIBRARIES} )
		SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${GTK2_INCLUDE_DIRS} )
		IF( GTKGLEXT_FOUND )
			SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${GTKGLEXT_LIBRARIES} )
			SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${GTKGLEXT_INCLUDE_DIRS} )
		ELSE( GTKGLEXT_FOUND )
			MESSAGE( FATAL_ERROR "gtkglext was requested but not found." )
		ENDIF( GTKGLEXT_FOUND )
	ELSE( GTK2_FOUND )
		MESSAGE( FATAL_ERROR "gtk2 was requested but not found." )
	ENDIF( GTK2_FOUND )
ENDIF( GTK_USER_INTERFACE )


# Defines API_INTERFACE_SRCS, CHOOSE_INTERFACE_SRCS, COMFILE_INTERFACE_SRCS,
# COLOUR_INTERFACE_SRCS, COMMAND_INTERFACE_SRCS,
# COMPUTED_FIELD_INTERFACE_SRCS, CURVE_INTERFACE_SRCS, FINITE_ELEMENT_INTERFACE_SRCS,
# GENERAL_INTERFACE_SRCS, GRAPHICS_INTERFACE_SRCS,
# INTERACTION_INTERFACE_SRCS, IO_DEVICES_INTERFACE_SRCS, LINK_INTERFACE_SRCS, MATERIAL_INTERFACE_SRCS,
# NODE_INTERFACE_SRCS, REGION_INTERFACE_SRCS,
# SELECT_INTERFACE_SRCS, TRANSFORMATION_INTERFACE_SRCS,
# THREE_D_DRAWING_INTERFACE_SRCS, TIME_INTERFACE_SRCS,
# USER_INTERFACE_SRCS, USER_INTERFACE_INTERFACE_SRCS, VIEW_INTERFACE_SRCS
INCLUDE( source/cmake/interfaces_source_files.cmake )

# Defines ZINC_INTERFACE_LIB_SRCS, ZINC_INTERFACE_LIB_HDRS
INCLUDE( source/cmake/zinc_interface_lib_source_files.cmake )

# TODO: reinstate
#PRINT_LIST_DUPLICATIONS( ZINC_INTERFACE_LIB_SRCS )
#PRINT_LIST_DUPLICATIONS( ZINC_INTERFACE_LIB_HDRS )

SET( TARGET_ZINC_INTERFACE_LIB "zinc-${ZINC_USER_INTERFACE}" )

IF(MSVC_IDE)
	# I want to group the source files by the directory layout for Visual Studio
	GROUP_SOURCE_FILES(${ZINC_INTERFACE_LIB_SRCS} ${ZINC_INTERFACE_LIB_HDRS})
ENDIF()

IF( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )
	SET_SOURCE_FILES_PROPERTIES( ${ZINC_INTERFACE_LIB_SRCS}
		PROPERTIES COMPILE_FLAGS "-fPIC" )
ENDIF( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )

ADD_LIBRARY( ${TARGET_ZINC_INTERFACE_LIB} ${ZINC_BUILD_LIBRARY_TYPE} ${ZINC_INTERFACE_LIB_SRCS} ${ZINC_INTERFACE_LIB_HDRS} )

SET_TARGET_PROPERTIES( ${TARGET_ZINC_INTERFACE_LIB} PROPERTIES DEBUG_POSTFIX "d"
	VERSION "${ZINC_MAJOR_VERSION}.${ZINC_MINOR_VERSION}.${ZINC_PATCH_VERSION}"
	SOVERSION ${ZINC_MAJOR_VERSION} )

INSTALL( TARGETS ${TARGET_ZINC_INTERFACE_LIB} EXPORT ${ZINC_LIB_GROUP}-targets
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION lib )
