
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

# Set the minimum version of cmake required to 2.8.1
# The FATAL_ERROR is needed if using cmake < 2.4
CMAKE_MINIMUM_REQUIRED( VERSION 2.8.1 FATAL_ERROR ) 

# This is the project name and shows up in ides
PROJECT( ZincLibrary )

# If you want to look at the debug messages uncomment the next line or define
# it on the command line
#SET( CMAKE_DEBUG TRUE )

FILE( TO_CMAKE_PATH "${CMISS_INSTALL_PREFIX}" CMISS_INSTALL_PREFIX )
FILE( TO_CMAKE_PATH "${CMISS_MODULE_DIR}" CMISS_MODULE_DIR )
FILE( TO_CMAKE_PATH "${CMISS_ROOT}" CMISS_ROOT )

# We need a path to our own CMake modules.  This has the MacroDefinitions.cmake file
# that contains common macro defintions that are shared between projects.
SET( FOUND_MACRODEFINITIONS_MODULE NO )
IF( CMISS_INSTALL_PREFIX OR CMISS_MODULE_DIR )
	IF( NOT EXISTS "${CMISS_MODULE_DIR}/MacroDefinitions.cmake" )
		SET( CMISS_MODULE_DIR "${CMISS_INSTALL_PREFIX}/share/cmake/Modules" )
	ENDIF()
	IF( EXISTS "${CMISS_MODULE_DIR}/MacroDefinitions.cmake" )
		SET( FOUND_MACRODEFINITIONS_MODULE YES )
		IF( NOT CMISS_INSTALL_PREFIX )
			# Check the install path on all operating systems
			SET( CMISS_INSTALL_PREFIX "/usr/local" )
		ENDIF()
	ENDIF()
ELSEIF( CMISS_ROOT )
	IF( NOT EXISTS "${CMISS_MODULE_DIR}/MacroDefinitions.cmake" )
		SET( CMISS_MODULE_DIR "${CMISS_ROOT}/cmake/Modules" )
	ENDIF()
	IF( EXISTS "${CMISS_MODULE_DIR}/MacroDefinitions.cmake" )
		SET( FOUND_MACRODEFINITIONS_MODULE YES )
		SET( CMISS_INSTALL_PREFIX "${CMISS_ROOT}/local" )
	ENDIF()
ELSEIF( EXISTS "$ENV{CMISS_ROOT}/cmake/Modules/MacroDefinitions.cmake" )
	SET( FOUND_MACRODEFINITIONS_MODULE YES )
	FILE( TO_CMAKE_PATH "$ENV{CMISS_ROOT}/cmake/Modules" CMISS_MODULE_DIR )
	FILE( TO_CMAKE_PATH "$ENV{CMISS_ROOT}/local" CMISS_INSTALL_PREFIX )
ENDIF()

IF( NOT FOUND_MACRODEFINITIONS_MODULE )
	#UNSET( CMISS_INSTALL_PREFIX CACHE )
	#UNSET( CMISS_MODULE_DIR CACHE )
	SET( CMISS_MODULE_DIR "" CACHE PATH "some text" )
	SET( CMISS_INSTALL_PREFIX "" CACHE PATH "some insightful text" )
	SET( CMAKE_INSTALL_PREFIX "empty" CACHE INTERNAL "Internalise CMAKE_INSTALL_PREFIX, manipulate via ZINC_INSTALL_PREFIX" FORCE )
	SET( CMAKE_BUILD_TYPE "empty" CACHE INTERNAL "Internalise CMAKE_BUILD_TYPE, manipulate via ZINC_BUILD_TYPE" FORCE )
	MESSAGE( FATAL_ERROR "Dude! I have not been able to find a path to the CMISS CMake modules.  Try setting CMISS_INSTALL_PREFIX, CMISS_MODULE_DIR or CMISS_ROOT." )
ENDIF()

SET( CMAKE_MODULE_PATH "${CMISS_MODULE_DIR}" )

INCLUDE( MacroDefinitions )
INCLUDE( FunctionDefinitions )

MACRO( DBG_MSG MSG_CONTENT )
	IF( DEFINED CMAKE_DEBUG )
		MESSAGE( STATUS "${MSG_CONTENT}" )
	ENDIF( DEFINED CMAKE_DEBUG )
ENDMACRO( DBG_MSG MSG_CONTENT )

SET( ZINC_MAJOR_VERSION "3" )
SET( ZINC_MINOR_VERSION "0" )
SET( ZINC_PATCH_VERSION "0" )

SET( PRODUCT_PATH ${CMISS_INSTALL_PREFIX}/zinc )
SET( PRODUCT_SOURCE ${PRODUCT_PATH}/source )

DEFINE_ARCHITECTURE_DIR( )
TEST_FOR_VFSCANF( HAVE_VFSCANF )

# Option setting, the following manipulates the build
OPTION_WITH_DEFAULT( ZINC_USE_ARCHITECTURE_DIR "Set true if cross-compiling or colocating libraries from different systems." OFF )
SET( ZINC_INSTALL_PREFIX_TEMP ${CMISS_INSTALL_PREFIX} )

# Set build type and install location
IF( ZINC_USE_ARCHITECTURE_DIR )
	STRING( FIND "${ZINC_INSTALL_PREFIX_TEMP}"
		"/${ARCHITECTURE_DIR}" CONTAINS )
	IF( ${CONTAINS} LESS 0 )
		SET( ZINC_INSTALL_PREFIX_TEMP
			"${ZINC_INSTALL_PREFIX_TEMP}/${ARCHITECTURE_DIR}" )
	ELSE( ${CONTAINS} LESS 0 )
		SET( ZINC_INSTALL_PREFIX
			"${ZINC_INSTALL_PREFIX_TEMP}" )
	ENDIF( ${CONTAINS} LESS 0 )
ELSE( ZINC_USE_ARCHITECTURE_DIR )
	STRING( REGEX REPLACE "/${ARCHITECTURE_DIR}" ""
		ZINC_INSTALL_PREFIX_TEMP ${ZINC_INSTALL_PREFIX_TEMP} )
ENDIF( ZINC_USE_ARCHITECTURE_DIR )

UNSET( ZINC_INSTALL_PREFIX CACHE )
CACHE_VAR_WITH_DEFAULT( ZINC_INSTALL_PREFIX "${ZINC_INSTALL_PREFIX_TEMP}"
	PATH "Where do you want your Zinc libraries installed?" )
SET( CMAKE_INSTALL_PREFIX ${ZINC_INSTALL_PREFIX} CACHE INTERNAL "Internalise CMAKE_INSTALL_PREFIX, manipulate via ZINC_INSTALL_PREFIX" FORCE )

CACHE_VAR_WITH_DEFAULT( ZINC_BUILD_TYPE "Release"
	STRING "What configuration do you want to build: Debug, Release, MinSizeRel, RelWithDebugInfo" )
SET( CMAKE_BUILD_TYPE ${ZINC_BUILD_TYPE} CACHE INTERNAL "Internalise CMAKE_BUILD_TYPE, manipulate via ZINC_BUILD_TYPE" FORCE )

# Set name option
CACHE_VAR_WITH_DEFAULT( ZINC_TARGET_NAME_FORMAT "Verbose"
	STRING "Do you want a 'Basic' or 'Verbose' target name" )

# Set user interface
CACHE_VAR_WITH_DEFAULT( ZINC_USER_INTERFACE "wx" STRING "What type of user interface is required" )
SET_USER_INTERFACE_DEFINES( )
SET( CAN_USE_USER_INTERFACE FALSE )

# Dependent library location options
IF( WIN32 )
	CACHE_VAR_WITH_DEFAULT( wxWidgets_DIR "${ZINC_INSTALL_PREFIX}/lib/cmake"
		PATH "Where is the wxWidgets config file installed?" )
ELSE( WIN32 )
	CACHE_VAR_WITH_DEFAULT( wxWidgets_DIR "${ZINC_INSTALL_PREFIX}"
		PATH "Where is wxWidgets installed?" )
ENDIF( WIN32 )
CACHE_VAR_WITH_DEFAULT( ImageMagick_DIR "${ZINC_INSTALL_PREFIX}/lib/cmake"
	PATH "Where is Image magick installed?" )
CACHE_VAR_WITH_DEFAULT( OCE_DIR "${ZINC_INSTALL_PREFIX}/lib/cmake"
	PATH "Where is the OCE CMake config file installed?" )
CACHE_VAR_WITH_DEFAULT( OpenCASCADE_DIR "${ZINC_INSTALL_PREFIX}/lib/cmake"
	PATH "Where is the OpenCASCADE CMake config file installed?" )
IF( EXISTS "${ZINC_INSTALL_PREFIX}/lib/InsightToolkit" )
	SET( ITK_DIR "${ZINC_INSTALL_PREFIX}/lib/InsightToolkit" )
ELSE( EXISTS "${ZINC_INSTALL_PREFIX}/lib/InsightToolkit" )
	SET( ITK_DIR "${ZINC_INSTALL_PREFIX}/lib" )
ENDIF( EXISTS "${ZINC_INSTALL_PREFIX}/lib/InsightToolkit" )
CACHE_VAR_WITH_DEFAULT( ITK_DIR "${ITK_DIR}"
	PATH "Where are the ITK libraries installed" )
CACHE_VAR_WITH_DEFAULT( PNG_DIR "${ZINC_INSTALL_PREFIX}/lib/cmake"
	PATH "Where is the png CMake config file installed?" )

# Locate dependent libraries
# I think I can get away with just this set
SET( CMAKE_PREFIX_PATH ${ZINC_INSTALL_PREFIX} )
# This is my back up plan if libxml2 is not found
#SET( CMAKE_INCLUDE_PATH ${ZINC_INSTALL_PREFIX}/include/libxml2 )
#SET( CMAKE_LIBRARY_PATH ${ZINC_INSTALL_PREFIX}/lib )
# Must have dependencies
#FIND_PACKAGE( LibXml2 REQUIRED ) <-- pulled in by fieldml
FIND_PACKAGE( fieldml REQUIRED HINTS ${ZINC_INSTALL_PREFIX}/lib/cmake )
FIND_PACKAGE( Opt++ REQUIRED COMPONENTS opt newton base constraints gss pds utils newmat )

# Should have dependencies
FIND_PACKAGE( ZLIB QUIET )
FIND_PACKAGE( BZip2 QUIET )
FIND_PACKAGE( JPEG QUIET )
FIND_PACKAGE( NETGEN QUIET )
FIND_PACKAGE( GLEW QUIET )

# Could have dependencies
FIND_PACKAGE( OCE QUIET NO_MODULE )
FIND_PACKAGE( OpenCASCADE QUIET NO_MODULE )
FIND_PACKAGE( ImageMagick QUIET COMPONENTS MagickCore )
FIND_PACKAGE( ITK QUIET )
#FIND_PACKAGE( GDCM QUIET )
SET( PNG_NAMES png12_static libpng12_static )
FIND_PACKAGE( PNG QUIET NO_MODULE )
FIND_PACKAGE( TIFF QUIET )
FIND_PACKAGE( perl_interpreter QUIET HINTS ${ZINC_INSTALL_PREFIX}/lib/cmake )
FIND_PACKAGE( PCRE QUIET HINTS ${ZINC_INSTALL_PREFIX}/lib/cmake )
MESSAGE(STATUS  "wxWidgets DIR: ${wxWidgets_DIR}" )
IF( WX_USER_INTERFACE )
	MESSAGE(STATUS  "wxWidgets DIR: ${wxWidgets_DIR}" )
	SET( CMAKE_PREFIX_PATH ${wxWidgets_DIR} )
	FIND_PACKAGE( wxWidgets QUIET COMPONENTS xrc adv html gl xml core base )
	SET( CAN_USE_USER_INTERFACE ${wxWidgets_FOUND} )
ENDIF( WX_USER_INTERFACE )
IF( GTK_USER_INTERFACE )
	FIND_PACKAGE( GTK2 REQUIRED gtk)
	FIND_PACKAGE( GTKGLext REQUIRED )
	SET( CAN_USE_USER_INTERFACE $(GTK2_FOUND) )
ENDIF( GTK_USER_INTERFACE )
IF( WIN32_USER_INTERFACE AND WIN32 )
	SET( CAN_USE_USER_INTERFACE TRUE )
ENDIF( WIN32_USER_INTERFACE AND WIN32 )
SET( CMAKE_PREFIX_PATH )
SET( CMAKE_LIBRARY_PATH )
SET( CMAKE_INCLUDE_PATH )

# Build options
SET( LIBRARY_TYPE_STRING "STATIC" )
IF( DEFINED ZINC_BUILD_LIBRARY_TYPE )
	SET( LIBRARY_TYPE_STRING ${ZINC_BUILD_LIBRARY_TYPE} )
ENDIF( DEFINED ZINC_BUILD_LIBRARY_TYPE )
SET( ZINC_BUILD_LIBRARY_TYPE ${LIBRARY_TYPE_STRING} CACHE STRING "What type of library is required [SHARED | STATIC | MODULE ]" )

# Define variables to false that need to be at least defined for passsing to option_with_default macro.
SET_FALSE_IF_NOT_DEFINED( PERL_INTERPRETER_FOUND )
SET_FALSE_IF_NOT_DEFINED( ITK_FOUND )
SET_FALSE_IF_NOT_DEFINED( ImageMagick_FOUND )
SET_FALSE_IF_NOT_DEFINED( NETGEN_FOUND )
SET_FALSE_IF_NOT_DEFINED( GLEW_FOUND )
SET_FALSE_IF_NOT_DEFINED( ZLIB_FOUND )
SET_FALSE_IF_NOT_DEFINED( BZIP2_FOUND )
SET_FALSE_IF_NOT_DEFINED( JPEG_FOUND )
SET_FALSE_IF_NOT_DEFINED( OCE_FOUND )
SET_FALSE_IF_NOT_DEFINED( OPENCASCADE_FOUND )
SET_FALSE_IF_NOT_DEFINED( PCRE_FOUND )

# Use options
IF( ${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug" )
	OPTION_WITH_DEFAULT( ZINC_BUILD_MEMORYCHECK "Do you want to build a memory check version" OFF )
ENDIF( ${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug" )
OPTION_WITH_DEFAULT( ZINC_BUILD_ABI "Do you want to build an ABI version" FALSE )
OPTION_WITH_DEFAULT( ZINC_FE_VALUE_IS_DOUBLE "Double or floats for FE_value?" TRUE )
# Dependent library options
# Must have dependent library options, so not really options just here for visiblity
# OPTION_WITH_DEFAULT( ZINC_USE_FIELDML "DO you want to use fieldml?" ON )
# OPTION_WITH_DEFAULT( ZINC_USE_LIBXML2 "Do you want to use xml2?" ${LIBXML2_FOUND} )
# Should have dependent library options
OPTION_WITH_DEFAULT( ZINC_USE_PERL_INTERPRETER "Do you want to use the perl interpreter?"
	${PERL_INTERPRETER_FOUND} )
OPTION_WITH_DEFAULT( ZINC_USE_NETGEN "Do you want to use netgen?" ${NETGEN_FOUND} )
OPTION_WITH_DEFAULT( ZINC_USE_GLEW "Do you want to use glew?" ${GLEW_FOUND} )
OPTION_WITH_DEFAULT( ZINC_USE_ZLIB "Do you want to use zlib?" ${ZLIB_FOUND} )
OPTION_WITH_DEFAULT( ZINC_USE_BZip2 "Do you want to use bzip2?" ${BZIP2_FOUND} )
OPTION_WITH_DEFAULT( ZINC_USE_JPEG "Do you want to use jpeg?" ${JPEG_FOUND} )
# Could have dependent library options
OPTION_WITH_DEFAULT( ZINC_USE_IMAGEMAGICK "Do you want to use Image Magick?"
	${ImageMagick_FOUND} )
OPTION_WITH_DEFAULT( ZINC_USE_ITK "Do you want to use ITK?" ${ITK_FOUND} )
# GDCM is not used directly by Zinc so there is no need to define it here.  It should be
# dragged in by the dependencies of those libraries that direclty depend upon it.
#OPTION_WITH_DEFAULT( ZINC_USE_GDCM "Do you want to use libGDCM?" OFF )
OPTION_WITH_DEFAULT( ZINC_USE_OCE "Do you want to use OCE?" ${OCE_FOUND} )
OPTION_WITH_DEFAULT( ZINC_USE_OPENCASCADE "Do you want to use OpenCASCADE?" FALSE )
OPTION_WITH_DEFAULT( ZINC_USE_PCRE "Do you want to use pcre?" ${PCRE_FOUND} )
IF( ITK_FOUND OR ImageMagick_FOUND )
	SET( PNG_AND_TIFF_REQUIRED TRUE )
ELSE( ITK_FOUND OR ImageMagick_FOUND )
	SET( PNG_AND_TIFF_REQUIRED FALSE )
ENDIF( ITK_FOUND OR ImageMagick_FOUND )
OPTION_WITH_DEFAULT( ZINC_USE_PNG "Do you want to use png?" ${PNG_AND_TIFF_REQUIRED} )
OPTION_WITH_DEFAULT( ZINC_USE_TIFF "Do you want to use tiff?" ${PNG_AND_TIFF_REQUIRED} )
OPTION_WITH_DEFAULT( ZINC_PRINT_CONFIG_SUMMARY "Do you want a configuration summary printed?" TRUE )

# Mark some options as advanced to declutter the CMake GUI interface
MARK_AS_ADVANCED( ZINC_USE_LINK_CMISS )
MARK_AS_ADVANCED( ZINC_USE_GTKMAIN )
MARK_AS_ADVANCED( ZINC_FE_VALUE_IS_DOUBLE )
# Set this for the configure header file
SET( FE_VALUE_IS_DOUBLE ${ZINC_FE_VALUE_IS_DOUBLE} )

# Set general variables that manipulate the build
# Set graphics
IF( NOT DEFINED GRAPHICS_API )
	SET( GRAPHICS_API OPENGL_GRAPHICS )
ENDIF( NOT DEFINED GRAPHICS_API )

# Check user interface is of known type
IF( NOT CAN_USE_USER_INTERFACE )
	GET_AVAILABLE_USER_INTERFACES( )
	MESSAGE( FATAL_ERROR "Can not use/find desired user interface."
		"  Use one of: ${AVAILABLE_USER_INTERFACES}" )
ENDIF( NOT CAN_USE_USER_INTERFACE )
DBG_MSG( "User interface: ${ZINC_USER_INTERFACE}" )

# Set build type dependent #Defines
IF( ${CMAKE_GENERATOR} MATCHES "Visual Studio" )
	DBG_MSG( "=== Visual Studio Project Creation ===" )
	SET( CREATING_FOR_MULTI_BUILD_TYPE TRUE )
ENDIF( ${CMAKE_GENERATOR} MATCHES "Visual Studio" )
IF( ${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug" )
	SET( USE_PARAMETER_ON TRUE )
	IF( NOT ${COMPILER_NAME} MATCHES "msvc" )
		SET( REPORT_GL_ERRORS )
	ENDIF( NOT ${COMPILER_NAME} MATCHES "msvc" )
ELSE( ${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug" )
	IF( CREATING_FOR_MULTI_BUILD_TYPE )
		SET( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DOPTIMISED" )
		SET( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DOPTIMISED" )
	ELSE( CREATING_FOR_MULTI_BUILD_TYPE )
		SET( OPTIMISED TRUE )
	ENDIF( CREATING_FOR_MULTI_BUILD_TYPE )
ENDIF( ${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug" )

IF( ZINC_BUILD_MEMORYCHECK )
	SET( MEMORY_CHECKING TRUE )
ENDIF( ZINC_BUILD_MEMORYCHECK )

IF( APPLE )
	FIND_LIBRARY( ICONV_LIBRARY NAMES iconv )
	MARK_AS_ADVANCED( ICONV_LIBRARY )
	IF( ICONV_LIBRARY )
		SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${ICONV_LIBRARY} )
	ENDIF( ICONV_LIBRARY )
ENDIF( APPLE )

# Set Target name and global defines
DEFINE_TARGET_NAME( )
SET_PLATFORM_DEFINES( )
SET_OPERATING_SYSTEM_DEFINES( ) # <-- mostly set automatically by cmake just adding WIN32_SYSTEM to be used instead of WIN32
SET_GRAPHICS_DEFINES( )

# Start loading in the dependent libraries
# fieldml, opt++
SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${FIELDML_LIBRARIES} ${OPTPP_LIBRARIES} )
SET( INCLUDE_DIRS ${FIELDML_INCLUDE_DIRS} ${OPTPP_INCLUDE_DIRS} )
SET( EXTRA_COMPILER_DEFINITIONS ${OPTPP_DEFINITIONS} ${FIELDML_DEFINITIONS})
SET( USE_FIELDML TRUE )
SET( USE_OPTPP TRUE )
SET( DEPENDENT_CONFIGS
	"${DEPENDENT_CONFIGS}\nINCLUDE(${fieldml_DIR}/fieldml-config.cmake)" )
DBG_MSG( "FieldML include dirs ${FIELDML_INCLUDE_DIRS}" )
DBG_MSG( "FieldML libraries ${FIELDML_LIBRARIES}" )

# xml2
SET( USE_LIBXML2 TRUE )
SET( DEPENDENT_DEFINITIONS ${DEPENDENT_DEFINITIONS} LIBXML_STATIC )
# Libxml2 libraries are pulled in by fieldml, check the fieldml interface
# libraries for the libxml2 location.
GET_TARGET_PROPERTY( LIBXML2_LIBRARIES fieldml IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE )

# zlib 
IF( ZINC_USE_ZLIB )
	FIND_PACKAGE( ZLIB REQUIRED )
	SET( USE_ZLIB TRUE )
	SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${ZLIB_LIBRARIES} )
	DBG_MSG( STATUS "Z LIBS: ${ZLIB_LIBRARIES}" )
	SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} )
ENDIF( ZINC_USE_ZLIB )

# bzip2 
IF( ZINC_USE_BZip2 )
	FIND_PACKAGE( BZip2 REQUIRED )
	SET( USE_BZIP2 TRUE )
	SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${BZIP2_LIBRARIES} )
	DBG_MSG( STATUS "BZ2 LIBS: ${BZIP2_LIBRARIES}" )
	SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${BZIP2_INCLUDE_DIRS} )
ENDIF( ZINC_USE_BZip2 )

# perl interpreter - is a config file therefore I cannot find twice
IF( ZINC_USE_PERL_INTERPRETER )
	IF( PERL_INTERPRETER_FOUND )
		SET( USE_PERL_INTERPRETER TRUE )
		SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${PERL_INTERPRETER_INCLUDE_DIRS} )
		SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${PERL_INTERPRETER_LIBRARIES} )
		SET( DEPENDENT_CONFIGS
			"${DEPENDENT_CONFIGS}\nINCLUDE(${perl_interpreter_DIR}/perl_interpreter-config.cmake)" )
		DBG_MSG( STATUS "PERL INT LIBS: ${PERL_INTERPRETER_LIBRARIES}" )
		DBG_MSG( "Perl Interpreter LIBS: ${PERL_INTERPRETER_LIBRARIES}" )
	ELSE( PERL_INTERPRETER_FOUND )
		MESSAGE( FATAL_ERROR "The perl interpreter library was requested but not found."
			" Looked in this directory '${perl_interpreter_DIR}' for a perl_interpreter-config.cmake file" )
	ENDIF( PERL_INTERPRETER_FOUND )
ENDIF( ZINC_USE_PERL_INTERPRETER )

IF( ZINC_USE_NETGEN )
	SET( CMAKE_PREFIX_PATH ${ZINC_INSTALL_PREFIX} )
	FIND_PACKAGE( NETGEN REQUIRED )
	SET( USE_NETGEN TRUE )
	SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${NETGEN_LIBRARIES} )
	DBG_MSG( STATUS " LIBS: ${}" )
	SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${NETGEN_INCLUDE_DIRS} )
	SET( CMAKE_PREFIX_PATH )
ENDIF( ZINC_USE_NETGEN )

IF( ZINC_USE_GLEW )
	SET( CMAKE_PREFIX_PATH ${ZINC_INSTALL_PREFIX} )
	FIND_PACKAGE( GLEW REQUIRED )
	SET( USE_GLEW TRUE )
	SET( USE_MSAA TRUE )
	IF( WIN32 )
		SET( GLEW_STATIC TRUE )
	ENDIF( WIN32 )
	SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${GLEW_LIBRARIES} )
	DBG_MSG( STATUS "GLEW LIBS: ${GLEW_LIBRARIES}" )
	SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} )
	SET( CMAKE_PREFIX_PATH )
ENDIF( ZINC_USE_GLEW )

IF( ZINC_USE_JPEG )
	SET( CMAKE_PREFIX_PATH ${ZINC_INSTALL_PREFIX} )
	FIND_PACKAGE( JPEG REQUIRED )
	SET( USE_JPEG TRUE )
	SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${JPEG_LIBRARIES} )
	DBG_MSG( STATUS " LIBS: ${}" )
	SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${JPEG_INCLUDE_DIRS} )
	SET( CMAKE_PREFIX_PATH )
ENDIF( ZINC_USE_JPEG )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	FIND_PACKAGE( OpenGL REQUIRED )
	#DBG_MSG( "OpenGL libs: " ${OPENGL_LIBRARIES} )
	#DBG_MSG( "OpenGL includes: " ${OPENGL_INCLUDE_DIRS} )
	SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${OPENGL_LIBRARIES} )
	DBG_MSG( STATUS " LIBS: ${}" )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

IF( ZINC_USE_ITK )
	FIND_PACKAGE( ITK REQUIRED )
	MARK_AS_ADVANCED( ITK_DIR )
	#DBG_MSG( "ITK libs: " ${ITK_LIBRARIES} )
	#DBG_MSG( "ITK includes: " ${ITK_INCLUDE_DIRS} )
	SET( USE_ITK TRUE )
	SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${ITK_LIBRARIES} )
	DBG_MSG( STATUS " LIBS: ${}" )
	SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${ITK_INCLUDE_DIRS} )
	SET( DEPENDENT_CONFIGS "${DEPENDENT_CONFIGS}\nINCLUDE(${ITK_DIR}/ITKConfig.cmake)" )
ENDIF( ZINC_USE_ITK )

# OCE - is a config file therefore I cannot find twice
IF( ZINC_USE_OCE )
	IF( OCE_FOUND )
		MARK_AS_ADVANCED( OCE_DIR )
		SET( USE_OPENCASCADE TRUE )
		SET( EXTRA_COMPILER_DEFINITIONS ${EXTRA_COMPILER_DEFINITIONS} USE_CAD_FIELDS HAVE_CONFIG_H )
		SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${OCE_LIBRARIES} )
		SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${OCE_INCLUDE_DIRS} )
		DBG_MSG( "=================" )
		DBG_MSG( ${OCE_LIBRARIES} )
		DBG_MSG( "=================" )
	ELSE( OCE_FOUND )
		MESSAGE( FATAL_ERROR "OCE was requested but not found." )
	ENDIF( OCE_FOUND )
ENDIF( ZINC_USE_OCE )

# OpenCASCADE - is a config file therefore I cannot find twice
IF( ZINC_USE_OPENCASCADE )
	IF( OPENCASCADE_FOUND )
		DBG_MSG( "OCC FOUND ${OPENCASCADE_FOUND}" )
		# Subset of OpenCASCADE libraries Zinc is dependent on
		# TKernel, TKMath, TKAdvTools, TKGeomBase, TKGeomAlgo, TKBRep, TKPrim,
		# TKBO, TKBool, TKG2d, TKG3d, PTKernel, TKOffset, TKFillet, TKMESH, TKIGES,
		# TKXCAF, TKCAF, TKV3d, TKLCAF, TKTopAlgo, TKCDF, TKV2d, TKHLR, TKService,
		# TKShHealing, TKXSBase, TKSTEP, TKSTEPBase, TKSTEPAttr, TKSTEP209,
		# TKXDEIGES, TKXDESTEP
		MARK_AS_ADVANCED( OpenCASCADE_DIR )
		SET( USE_OPENCASCADE TRUE )
		SET( OPENCASCADE_DEFINITIONS USE_CAD_FIELDS )
		IF( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64" )
			# 64-bit architecture
			SET( OPENCASCADE_DEFINITIONS ${OPENCASCADE_DEFINITIONS} _OCC64 )
			SET( OPENCASCADE_CXX_FLAGS -m64 )
		ENDIF( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64" )
		IF( WIN32 )
			SET( OPENCASCADE_DEFINITIONS ${OPENCASCADE_DEFINITIONS} WNT HAVE_NO_DLL )
		ELSE( WIN32 )
			SET( OPENCASCADE_DEFINITIONS ${OPENCASCADE_DEFINITIONS} HAVE_OCC_CONFIG_H )
		ENDIF( WIN32 )
		#SET( OPENCASCADE_INCLUDE_DIRS ${OPENCASCADE_PATH}/include/OpenCASCADE )
		DBG_MSG( "OCC INC: ${OPENCASCADE_INCLUDE_DIRS}" )
		#SET( OPENCASCADE_LIBRARY_DIRS ${OPENCASCADE_PATH}/lib/OpenCASCADE )
		DBG_MSG( "OCC LIB: ${OPENCASCADE_LIBRARIES}" )
		SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPENCASCADE_CXX_FLAGS}" )
		SET( EXTRA_COMPILER_DEFINITIONS ${EXTRA_COMPILER_DEFINITIONS} ${OPENCASCADE_DEFINITIONS} )
		#IF( NOT WIN32 )
		#	SET( OPENCASCADE_X_LIBRARIES /usr/lib64/libXmu.so )
		#ENDIF( NOT WIN32 )
		SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${OPENCASCADE_LIBRARIES} )
		SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${OPENCASCADE_INCLUDE_DIRS} )
		SET( DEPENDENT_CONFIGS "${DEPENDENT_CONFIGS}\nINCLUDE(${OpenCASCADE_DIR}/opencascade-config.cmake)" )
		SET( DEPENDENT_DEFINITIONS ${DEPENDENT_DEFINITIONS} ${OPENCASCADE_DEFINITIONS} )
	ELSE( OPENCASCADE_FOUND )
		MESSAGE( FATAL_ERROR "OpenCASCADE was requested but not found." )
	ENDIF( OPENCASCADE_FOUND )
ENDIF( ZINC_USE_OPENCASCADE )

# ImageMagick - is a config file therefore I cannot find twice
IF( ZINC_USE_IMAGEMAGICK )
	IF( ImageMagick_FOUND )
		MARK_AS_ADVANCED( ImageMagick_DIR )
		DBG_MSG( "Image magick found " ${ImageMagick_INCLUDE_DIRS} )
		#DBG_MSG( "Image magick inc ${ImageMagick_INCLUDE_DIRS}" )
		SET( USE_IMAGEMAGICK TRUE )
		# For config summary print
		SET( IMAGEMAGICK_LIBRARIES ${ImageMagick_LIBRARIES} )
		SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} MagickCore )
		#DBG_MSG( STATUS " LIBS: ${}" )
		SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${ImageMagick_INCLUDE_DIRS} )

		SET( CMAKE_INCLUDE_PATH ${ZINC_INSTALL_PREFIX}/include )
		SET( CMAKE_LIBRARY_PATH ${ZINC_INSTALL_PREFIX}/lib )
		IF( NOT USE_BZIP2 )
			FIND_PACKAGE( BZip2 REQUIRED )
			SET( USE_BZIP2 TRUE )
			#DBG_MSG( "BZip2 has been found " ${BZIP2_LIBRARIES} )
			SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${BZIP2_LIBRARIES} )
			DBG_MSG( STATUS " LIBS: ${}" )
			SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${BZIP2_INCLUDE_DIR} )
		ENDIF( NOT USE_BZIP2 )
		IF( NOT USE_JPEG )
			FIND_PACKAGE( JPEG REQUIRED )
			SET( USE_JPEG TRUE )
			SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${JPEG_LIBRARIES} )
			DBG_MSG( STATUS " LIBS: ${}" )
			SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${JPEG_INCLUDE_DIRS} )
		ENDIF( NOT USE_JPEG )
		SET( CMAKE_LIBRARY_PATH )
		SET( CMAKE_INCLUDE_PATH )
		SET( DEPENDENT_CONFIGS
			"${DEPENDENT_CONFIGS}\nINCLUDE(${ImageMagick_DIR}/ImageMagick-config.cmake)" )
	ELSE( ImageMagick_FOUND )
		MESSAGE( FATAL_ERROR "ImageMagick was requested but not found." )
	ENDIF( ImageMagick_FOUND )
ENDIF( ZINC_USE_IMAGEMAGICK )

IF( ZINC_USE_TIFF )
	FIND_PACKAGE( TIFF REQUIRED )
	DBG_MSG( "TIFF has been found " ${TIFF_LIBRARIES} )
	SET( USE_TIFF TRUE )
	SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${TIFF_LIBRARIES} )
	DBG_MSG( STATUS "TIFF LIBS: ${TIFF_LIBRARIES}" )
	SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${TIFF_INCLUDE_DIRS} )
ENDIF( ZINC_USE_TIFF )

IF( ZINC_USE_PNG )
	FIND_PACKAGE( PNG REQUIRED NO_MODULE )
	SET( USE_PNG TRUE )
	SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${PNG_LIBRARIES} )
	SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${PNG_INCLUDE_DIRS} )
	SET( DEPENDENT_CONFIGS
		"${DEPENDENT_CONFIGS}\nINCLUDE(${PNG_DIR}/png-config.cmake)" )
ENDIF( ZINC_USE_PNG )

IF( ZINC_USE_PCRE )
	FIND_PACKAGE( PCRE REQUIRED )
	#DBG_MSG( "PCRE has been found " ${PCRE_INCLUDE_DIRS} " " ${PCRE_LIBRARIES} )
	SET( USE_PCRE TRUE )
	SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${PCRE_LIBRARIES} )
	DBG_MSG( STATUS "PCRE LIBS: ${PCRE_LIBRARIES}" )
	SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${PCRE_INCLUDE_DIRS} )
	SET( DEPENDENT_CONFIGS
		"${DEPENDENT_CONFIGS}\nINCLUDE(${PCRE_DIR}/pcre-config.cmake)" )
ENDIF( ZINC_USE_PCRE )

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
		INCLUDE( source/cmake/xrc_source_files.cmake )
		DBG_MSG( "XRC SRCS: ${XRC_SRCS}" )
		WXWIDGETS_HDRS( "${XRC_SRCS}" wxWidgets_GENERATED_HDRS )
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

#IF( ZINC_USE_GDCM )
#	IF( GDCM_FOUND )
#		DBG_MSG( "WARNING: GDCM Issues see below" )
#		DBG_MSG( "GDCM use file: ${GDCM_USE_FILE}" )
#		MESSAGE( "GDCM_LIBRARIES currently: ${GDCM_LIBRARIES}" )
#		DBG_MSG( "Hard coding to gdcmCWRAPPER gdcmDSED gdcmMSFF gdcmDIST" )
#		#SET( GDCM_LIBRARIES gdcmCWRAPPER gdcmDSED gdcmMSFF gdcmDICT )
#		SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${LIBGDCM_LIBRARIES} )
#		SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${LIBGDCM_INCLUDE_DIRS} )
#		SET( USE_GDCM TRUE )
#	ELSE( GDCM_FOUND )
#		MESSAGE( FATAL_ERROR "GDCM was requested but not found." )
#	ENDIF( GDCM_FOUND )
#ENDIF( ZINC_USE_GDCM )

# Load in source files, each source file list may contain several
# CMake variables see comments for a list of _SRCS included from 
# a particular file (similarly that file defines a _HDRS variable).
# Defines API_SRCS
INCLUDE( source/cmake/api_source_files.cmake )

# Defines COMMAND_SRCS, CONTEXT_SRCS
INCLUDE( source/cmake/command_source_files.cmake )

# Defines COMPUTED_FIELD_CORE_SRCS, COMPUTED_FIELD_GRAPHICS_SRCS,
# COMPUTED_FIELD_SRCS (contains previous two)
INCLUDE( source/cmake/computed_field_source_files.cmake )

# Defines FINITE_ELEMENT_CORE_SRCS, FINITE_ELEMENT_GRAPHICS_SRCS, 
# FINITE_ELEMENT_ADDITIONAL_SRCS, FINITE_ELEMENT_SRCS 
# (group definition which includes the previous three)
INCLUDE( source/cmake/finite_element_source_files.cmake )

# Defines GENERAL_SRCS
INCLUDE( source/cmake/general_source_files.cmake )

# Defines GRAPHICS_SRCS
INCLUDE( source/cmake/graphics_source_files.cmake )

# Defines IMAGE_PROCESSING_SRCS
INCLUDE( source/cmake/image_processing_source_files.cmake )

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

# Defines COMFILE_SRCS, CURVE_SRCS, ELEMENT_SRCS, EMOTER_SRCS
# FIELD_IO_SRCS, INTERACTION_SRCS, IO_DEVICES_SRCS, MESH_SRCS, NODE_SRCS,
# REGION_SRCS, SELECTION_SRCS, THREE_D_DRAWING_SRCS, TIME_SRCS
INCLUDE( source/cmake/misc_source_files.cmake )

# Defines OPENCASCADE_SRCS
INCLUDE( source/cmake/opencascade_source_files.cmake )

IF( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
	#SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Werror" )
	#SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Werror" )
	SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra -Wall -Wl,--as-needed" )
	SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wall -Wl,--as-needed" )
	SET( CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} --as-needed" )
	IF( ${CMAKE_BUILD_TYPE} MATCHES [Rr]elease )
		SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter" )
		SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter" )
	ENDIF( ${CMAKE_BUILD_TYPE} MATCHES [Rr]elease )
ELSEIF( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
	MESSAGE( STATUS "Set windows compiler specific flags..." )
ENDIF( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

INCLUDE_DIRECTORIES( source source/api ${CMAKE_CURRENT_BINARY_DIR}/source
	${INCLUDE_DIRS} )

# TODO: Is this still required????
LINK_DIRECTORIES( ${ITK_LIBRARY_DIRS} ${GDCM_LIBRARY_DIRS} )

IF( DEFINED MSVC )
	SET( EXTRA_COMPILER_DEFINITIONS ${EXTRA_COMPILER_DEFINITIONS} _CRT_SECURE_NO_WARNINGS _CRTDBG_MAP_ALLOC )
	#DBG_MSG( "extra compiler definitions: ${EXTRA_COMPIILER_DEFINITIONS}" )
ENDIF( DEFINED MSVC )

FOREACH( DEF ${EXTRA_COMPILER_DEFINITIONS} BUILD_WITH_CMAKE )
	ADD_DEFINITIONS( -D${DEF} )
ENDFOREACH( DEF ${EXTRA_COMPILER_DEFINITIONS} BUILD_WITH_CMAKE )

CONFIGURE_FILE( ${PROJECT_SOURCE_DIR}/source/configure/zinc_configure.h.cmake
	${PROJECT_BINARY_DIR}/source/configure/zinc_configure.h )

# Used in the generate version string cmake script at compile time.
SET( ZINC_SVN_REPOSITORY_DIR "${CMAKE_SOURCE_DIR}" )
Subversion_GET_REVISION( "${ZINC_SVN_REPOSITORY_DIR}" ZINC_REVISION )
DBG_MSG(STATUS "Revision is ${ZINC_REVISION}")

# Define PASS_THROUGH_LIB_SRCS, PASS_THROUGH_LIB_HDRS
SET( PASS_THROUGH_LIB_SRCS "source/command/pass_through_interpreter.c" )
SET( PASS_THROUGH_LIB_HDRS "source/command/pass_through_interpreter.h;source/general/debug.h" )

# Defines ZINC_LIB_SRCS, ZINC_LIB_HDRS
INCLUDE( source/cmake/zinc_lib_source_files.cmake )

# Defines ZINC_INTERFACE_LIB_SRCS, ZINC_INTERFACE_LIB_HDRS
INCLUDE( source/cmake/zinc_interface_lib_source_files.cmake )

PRINT_LIST_DUPLICATIONS( ZINC_LIB_SRCS )
PRINT_LIST_DUPLICATIONS( ZINC_LIB_HDRS )

PRINT_LIST_DUPLICATIONS( ZINC_INTERFACE_LIB_SRCS )
PRINT_LIST_DUPLICATIONS( ZINC_INTERFACE_LIB_HDRS )

SET( CONFIGURE_HDRS
	${PROJECT_BINARY_DIR}/source/configure/zinc_configure.h )
SOURCE_GROUP( "Header Files\\configure" FILES ${CONFIGURE_HDRS} )

SET( TARGET_ZINC_LIB "zinc" )
SET( TARGET_ZINC_INTERFACE_LIB "zinc-${ZINC_USER_INTERFACE}" )
SET( TARGET_ZINC_PASS_THROUGH_LIB "zinc_pass_through" )
SET( INSTALL_LIBRARY_TYPE "ARCHIVE" )
IF( ${ZINC_BUILD_LIBRARY_TYPE} MATCHES "SHARED" )
	SET( INSTALL_LIBRARY_TYPE "LIBRARY" )
ELSEIF( ${ZINC_BUILD_LIBRARY_TYPE} MATCHES "MODULE" )
	SET( INSTALL_LIBRARY_TYPE "LIBRARY" )
ENDIF( ${ZINC_BUILD_LIBRARY_TYPE} MATCHES "SHARED" )

IF(MSVC_IDE)
	# I want to group the source files by the directory layout for Visual Studio
	GROUP_SOURCE_FILES(${ZINC_LIB_SRCS} ${ZINC_LIB_HDRS} ${CONFIGURE_HDRS})
	GROUP_SOURCE_FILES(${ZINC_INTERFACE_LIB_SRCS} ${ZINC_INTERFACE_LIB_HDRS})
	GROUP_SOURCE_FILES(${PASS_THROUGH_LIB_SRCS} ${PASS_THROUGH_LIB_HDRS} ${CONFIGURE_HDRS})
ENDIF()

ADD_LIBRARY( ${TARGET_ZINC_LIB} ${ZINC_BUILD_LIBRARY_TYPE} ${ZINC_LIB_SRCS} ${ZINC_LIB_HDRS} ${CONFIGURE_HDRS} )
ADD_LIBRARY( ${TARGET_ZINC_INTERFACE_LIB} ${ZINC_BUILD_LIBRARY_TYPE} ${ZINC_INTERFACE_LIB_SRCS} ${ZINC_INTERFACE_LIB_HDRS} )
# ADD_LIBRARY( ${TARGET_ZINC_PASS_THROUGH_LIB} ${ZINC_BUILD_LIBRARY_TYPE} ${PASS_THROUGH_LIB_SRCS} ${PASS_THROUGH_LIB_HDRS} ${CONFIGURE_HDRS} )

TARGET_LINK_LIBRARIES( ${TARGET_ZINC_LIB} ${DEPENDENT_LIBS} )
#TARGET_LINK_LIBRARIES( ${TARGET_ZINC_INTERFACE_LIB} ${??} )
# TARGET_LINK_LIBRARIES( ${TARGET_ZINC_PASS_THROUGH_LIB} ${PCRE_LIBRARIES} )

SET_TARGET_PROPERTIES( ${TARGET_ZINC_LIB} PROPERTIES DEBUG_POSTFIX "d"
	VERSION "${ZINC_MAJOR_VERSION}.${ZINC_MINOR_VERSION}.${ZINC_PATCH_VERSION}"
	SOVERSION ${ZINC_MAJOR_VERSION} )
SET_TARGET_PROPERTIES( ${TARGET_ZINC_INTERFACE_LIB} PROPERTIES DEBUG_POSTFIX "d"
	VERSION "${ZINC_MAJOR_VERSION}.${ZINC_MINOR_VERSION}.${ZINC_PATCH_VERSION}"
	SOVERSION ${ZINC_MAJOR_VERSION} )
# SET_TARGET_PROPERTIES( ${TARGET_ZINC_PASS_THROUGH_LIB} PROPERTIES DEBUG_POSTFIX "d"
	# VERSION "${ZINC_MAJOR_VERSION}.${ZINC_MINOR_VERSION}.${ZINC_PATCH_VERSION}"
	# SOVERSION ${ZINC_MAJOR_VERSION} )

SET( ZINC_LIB_GROUP zinc )
LIST( REMOVE_DUPLICATES INCLUDE_DIRS )
STRING( REGEX REPLACE ";" " " INCLUDE_DIRS "${INCLUDE_DIRS}" )
STRING( REGEX REPLACE ";" " " DEPENDENT_DEFINITIONS "${DEPENDENT_DEFINITIONS}" )
SET( CONFIG_FILE_CONTENTS
	"\n#"
	"\n# - Zinc Config File."
	"\n# Defines:"
	"\n#"
	"\n#  ZINC_INCLUDE_DIRS - Where to find the Zinc header files"
	"\n#  ZINC_LIBRARIES    - List of libraries for using Zinc"
	"\n#  ZINC_DEFINITIONS  - List of definitions that *must* be set"
	"\n#  ZINC_FOUND        - Set to TRUE to conform like other FIND_PACKAGE calls"
	"\n#"
	"\n# The ZINC_DEFINITIONS need to be added to your project like so:"
	"\n#"
	"\n#  FOREACH(DEF \${ZINC_DEFINITIONS}"
	"\n#      ADD_DEFINTIONS( -D\${DEF} )"
	"\n#  ENDFORECH(DEF \${ZINC_DEFINITIONS}"
	"\n#"
	"\nIF( NOT DEFINED _ZINC_CONFIG_CMAKE )"
	"\nSET( _ZINC_CONFIG_CMAKE TRUE )"
	"${DEPENDENT_CONFIGS}"
	"\nGET_FILENAME_COMPONENT( SELF_DIR \"\${CMAKE_CURRENT_LIST_FILE}\" PATH )"
	"\nINCLUDE( \${SELF_DIR}/${ZINC_LIB_GROUP}-targets.cmake )"
	"\nGET_FILENAME_COMPONENT( ZINC_INCLUDE_DIRS \"\${SELF_DIR}/../../include\" ABSOLUTE )"
	"\nSET( ZINC_INCLUDE_DIRS \${ZINC_INCLUDE_DIRS} ${INCLUDE_DIRS} )"
	"\nSET( ZINC_LIBRARIES ${TARGET_ZINC_LIB} ${TARGET_ZINC_INTERFACE_LIB} )"
	"\nSET( ZINC_DEFINITIONS ${DEPENDENT_DEFINITIONS} )"
	"\nSET( ZINC_FOUND TRUE )"
	"\nENDIF( NOT DEFINED _ZINC_CONFIG_CMAKE )"
	"\n\n" )
SET( CONFIG_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/${ZINC_LIB_GROUP}-config.cmake )
FILE( WRITE ${CONFIG_FILE_NAME} ${CONFIG_FILE_CONTENTS} )

INSTALL( TARGETS ${TARGET_ZINC_LIB} ${TARGET_ZINC_INTERFACE_LIB} EXPORT ${ZINC_LIB_GROUP}-targets
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION lib )
# INSTALL( TARGETS ${TARGET_ZINC_PASS_THROUGH_LIB} EXPORT ${ZINC_LIB_GROUP}-targets
	# ARCHIVE DESTINATION lib
	# LIBRARY DESTINATION lib
	# RUNTIME DESTINATION lib )

INSTALL( EXPORT ${ZINC_LIB_GROUP}-targets DESTINATION lib/cmake )

INSTALL( FILES ${CONFIG_FILE_NAME} DESTINATION lib/cmake )

# Install headers for Zinc
SET( ZINC_LIB_API_INSTALL_HDRS
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_context.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_core.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_differential_operator.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_element.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_fdio.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field_alias.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field_arithmetic_operators.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field_composite.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field_conditional.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field_finite_element.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field_group.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field_image.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field_image_processing.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field_logical_operators.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field_matrix_operators.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field_module.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field_nodeset_operators.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field_scene_viewer_projection.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field_subobject_group.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field_time.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field_trigonometry.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field_vector_operators.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_graphic.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_graphics_filter.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_graphics_material.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_graphics_module.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_idle.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_interactive_tool.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_node.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_optimisation.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_region.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_rendition.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_scene.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_scene_viewer.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_selection.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_spectrum.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_status.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_stream.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_tessellation.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_time.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_time_keeper.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_time_sequence.h )
IF( USE_OPENCASCADE )
	SET( ZINC_LIB_API_INSTALL_HDRS ${ZINC_LIB_API_INSTALL_HDRS}
		${CMAKE_CURRENT_SOURCE_DIR}/source/api/cmiss_field_cad.h )
ENDIF( USE_OPENCASCADE )

SET( ZINC_LIB_API_TYPES_INSTALL_HDRS
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_c_inline.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_context_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_differential_operator_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_element_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_field_alias_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_field_finite_element_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_field_group_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_field_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_field_image_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_field_image_processing_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_field_module_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_field_subobject_group_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_graphic_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_graphics_coordinate_system.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_graphics_filter_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_graphics_material_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_graphics_module_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_graphics_render_type.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_interactive_tool_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_node_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_optimisation_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_region_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_rendition_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_scene_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_scene_viewer_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_selection_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_spectrum_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_stream_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_tessellation_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_time_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_time_keeper_id.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_time_sequence_id.h )
IF( USE_OPENCASCADE )
	SET( ZINC_LIB_API_TYPES_INSTALL_HDRS ${ZINC_LIB_API_TYPES_INSTALL_HDRS}
		${CMAKE_CURRENT_SOURCE_DIR}/source/api/types/cmiss_field_cad_id.h )
ENDIF( USE_OPENCASCADE )

# Install headers for pass_through_interpreter
SET( PASS_THROUGH_LIB_COMMAND_INSTALL_HDRS
	${CMAKE_CURRENT_SOURCE_DIR}/source/command/pass_through_interpreter.h )

# Files that always need to be installed
INSTALL( FILES ${PROJECT_BINARY_DIR}/source/configure/zinc_configure.h
	DESTINATION include/zn/ )

# Api directory installs
INSTALL( FILES ${ZINC_LIB_API_TYPES_INSTALL_HDRS}
	DESTINATION include/zn/types )

INSTALL( FILES ${ZINC_LIB_API_INSTALL_HDRS}
	${GENERAL_LIB_API_INSTALL_HDRS}
	${CORE_FIELDS_LIB_API_INSTALL_HDRS}
	${PASS_THROUGH_LIB_COMMAND_INSTALL_HDRS}
	DESTINATION include/zn/ )

# Output configuration information if requested
IF( ZINC_PRINT_CONFIG_SUMMARY )
	MACRO( LIBRARY_SUMMARY_STRING LIBRARY_NAME OUTPUT_STRING )
		IF( USE_${LIBRARY_NAME} )
			SET( ${OUTPUT_STRING} "system" )
			FOREACH( LIB ${${LIBRARY_NAME}_LIBRARIES} )
				GET_TARGET_PROPERTY( TARGET_ILR ${LIB} IMPORTED_LOCATION_RELEASE )
				SET( SEARCH_LIB_STRING ${LIB} )
				# If we can get a target imported location use
				# that for the test string.
				IF( TARGET_ILR )
					SET( SEARCH_LIB_STRING ${TARGET_ILR} )
				ENDIF( TARGET_ILR )
				STRING( REGEX MATCH "${ZINC_INSTALL_PREFIX}" REGEX_RESULT ${SEARCH_LIB_STRING} )
				IF( "x${REGEX_RESULT}" STREQUAL "x${ZINC_INSTALL_PREFIX}" )
					SET( ${OUTPUT_STRING} "third_party" )
				ENDIF( "x${REGEX_RESULT}" STREQUAL "x${ZINC_INSTALL_PREFIX}" )
			ENDFOREACH( LIB ${${LIBRARY_NAME}_LIBRARIES} )
		ELSE( USE_${LIBRARY_NAME} )
			SET( ${OUTPUT_STRING} "n/a" )
		ENDIF( USE_${LIBRARY_NAME} )
	ENDMACRO( LIBRARY_SUMMARY_STRING LIBRARY_NAME OUTPUT_STRING )
	LIBRARY_SUMMARY_STRING( BZIP2 BZIP2_LIB_STRING )
	LIBRARY_SUMMARY_STRING( FIELDML FIELDML_LIB_STRING )
#	LIBRARY_SUMMARY_STRING( GDCM GDCM_LIB_STRING )
	LIBRARY_SUMMARY_STRING( GLEW GLEW_LIB_STRING )
	LIBRARY_SUMMARY_STRING( IMAGEMAGICK IMAGEMAGICK_LIB_STRING )
	LIBRARY_SUMMARY_STRING( ITK INSIGHTTOOLKIT_LIB_STRING )
	LIBRARY_SUMMARY_STRING( JPEG JPEG_LIB_STRING )
	LIBRARY_SUMMARY_STRING( NETGEN NETGEN_LIB_STRING )
	LIBRARY_SUMMARY_STRING( OPENCASCADE OPENCASCADE_LIB_STRING )
	LIBRARY_SUMMARY_STRING( OPTPP OPT++_LIB_STRING )
	LIBRARY_SUMMARY_STRING( PCRE PCRE_LIB_STRING )
	LIBRARY_SUMMARY_STRING( PERL_INTERPRETER PERL_INTERPRETER_LIB_STRING )
	LIBRARY_SUMMARY_STRING( PNG PNG_LIB_STRING )
	LIBRARY_SUMMARY_STRING( TIFF TIFF_LIB_STRING )
	LIBRARY_SUMMARY_STRING( WXWIDGETS WXWIDGETS_LIB_STRING )
	LIBRARY_SUMMARY_STRING( LIBXML2 LIBXML2_LIB_STRING )
	LIBRARY_SUMMARY_STRING( ZLIB ZLIB_LIB_STRING )
	IF( ZINC_BUILD_EXECUTABLE )
		SET( ZINC_BUILD_STRING "binary executable" )
	ELSE( ZINC_BUILD_EXECUTABLE )
		IF( ${ZINC_BUILD_LIBRARY_TYPE} STREQUAL "STATIC" )
			SET( ZINC_BUILD_STRING "static library" )
		ELSEIF( ${ZINC_BUILD_LIBRARY_TYPE} STREQUAL "SHARED" )
			SET( ZINC_BUILD_STRING "shared library" )
		ELSE( ${ZINC_BUILD_LIBRARY_TYPE} STREQUAL "SHARED" )
			SET( ZINC_BUILD_STRING "module library" )
		ENDIF( ${ZINC_BUILD_LIBRARY_TYPE} STREQUAL "STATIC" )
	ENDIF( ZINC_BUILD_EXECUTABLE )
	MESSAGE( STATUS "")
	MESSAGE( STATUS "======================================================")
	MESSAGE( STATUS "Zinc has been configured with the following settings:")
	MESSAGE( STATUS "      User interface: '${ZINC_USER_INTERFACE}'")
	MESSAGE( STATUS "    Architecture dir: '${ARCHITECTURE_DIR}'")
	MESSAGE( STATUS "     Source revision: '${ZINC_REVISION}'")
	MESSAGE( STATUS "         Target type: '${ZINC_BUILD_STRING}'")
	MESSAGE( STATUS "          Build type: '${CMAKE_BUILD_TYPE}'")
	MESSAGE( STATUS "")
	MESSAGE( STATUS "Dependent libraries:")
	MESSAGE( STATUS "               BZIP2: '${BZIP2_LIB_STRING}'")
	MESSAGE( STATUS "             FIELDML: '${FIELDML_LIB_STRING}'")
#	MESSAGE( STATUS "                GDCM: '${GDCM_LIB_STRING}'")
	MESSAGE( STATUS "                GLEW: '${GLEW_LIB_STRING}'")
	MESSAGE( STATUS "         IMAGEMAGICK: '${IMAGEMAGICK_LIB_STRING}'")
	MESSAGE( STATUS "      INSIGHTTOOLKIT: '${INSIGHTTOOLKIT_LIB_STRING}'")
	MESSAGE( STATUS "                JPEG: '${JPEG_LIB_STRING}'")
	MESSAGE( STATUS "              NETGEN: '${NETGEN_LIB_STRING}'")
	MESSAGE( STATUS "         OPENCASCADE: '${OPENCASCADE_LIB_STRING}'")
	MESSAGE( STATUS "               OPT++: '${OPT++_LIB_STRING}'")
	MESSAGE( STATUS "                PCRE: '${PCRE_LIB_STRING}'")
	MESSAGE( STATUS "    PERL INTERPRETER: '${PERL_INTERPRETER_LIB_STRING}'")
	MESSAGE( STATUS "                 PNG: '${PNG_LIB_STRING}'")
	MESSAGE( STATUS "                TIFF: '${TIFF_LIB_STRING}'")
	MESSAGE( STATUS "           WXWIDGETS: '${WXWIDGETS_LIB_STRING}'")
	MESSAGE( STATUS "             LIBXML2: '${LIBXML2_LIB_STRING}'")
	MESSAGE( STATUS "                ZLIB: '${ZLIB_LIB_STRING}'")
	MESSAGE( STATUS "======================================================")
	MESSAGE( STATUS "")
ENDIF( ZINC_PRINT_CONFIG_SUMMARY )

