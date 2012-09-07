
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

# Defines ZINC_INTERFACE_LIB_SRCS, ZINC_INTERFACE_LIB_HDRS
INCLUDE( source/cmake/zinc_interface_lib_source_files.cmake )

PRINT_LIST_DUPLICATIONS( ZINC_INTERFACE_LIB_SRCS )
PRINT_LIST_DUPLICATIONS( ZINC_INTERFACE_LIB_HDRS )

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

