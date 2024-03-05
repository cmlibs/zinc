
# Zinc Library
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.10.0)
cmake_policy(VERSION 3.10.0)

if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

if(POLICY CMP0078)
    cmake_policy(SET CMP0078 NEW)
endif()

if(POLICY CMP0086)
    cmake_policy(SET CMP0086 NEW)
endif()

# This is the project name and shows up in IDEs
project(Zinc VERSION 4.1.0 LANGUAGES C CXX)

# Set this to the development release or release candidate version.
set(Zinc_DEVELOPER_VERSION ".dev0")

# The name for the project seen in directory paths
set(ZINC_BASE_NAME zinc)

find_package(CMLIBSDEPENDENCIES REQUIRED)

if (NOT "${CMAKE_CURRENT_SOURCE_DIR}/cmake" IN_LIST CMAKE_MODULE_PATH)
    list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
endif ()

include(MacroDefinitions)
include(FunctionDefinitions)
include(CMLMiscFunctions)

include(EnvironmentChecks)

option(ZINC_BUILD_TESTS "${PROJECT_NAME} - Build tests." ON)
option(ZINC_BUILD_SHARED_LIBRARY "Build a shared zinc library." ON)
option(ZINC_BUILD_STATIC_LIBRARY "Build a static zinc library." OFF)
option(ZINC_PRINT_CONFIG_SUMMARY "Show a summary of the configuration." TRUE)
option(ZINC_USE_OSMESA "Use OSMesa for software rendering." OFF)
if(VALGRIND_TESTING_AVAILABLE)
  option(ZINC_MEMCHECK_WITH_VALGRIND "Enable memory checking with valgrind." OFF)
endif()
if(LLVM_COVERAGE_TESTING_AVAILABLE)
  set(ZINC_LLVM_COVERAGE OFF CACHE BOOL "Enable LLVM coverage testing.")
endif()

# Optional libraries
option(ZINC_USE_IMAGEMAGICK "Use ImageMagick" YES)
if (NOT ZINC_USE_IMAGEMAGICK)
    option(ZINC_USE_PNG "Use png" YES)
endif()
option(ZINC_USE_NETGEN "Use Netgen" YES)
# option(ZINC_USE_ITK "Use ITK" YES) # Not really an option until image fields are worked on
set(ZINC_USE_ITK YES)

set(DEPENDENT_LIBS bz2 fieldml-core fieldml-io ftgl optpp png ITKRegionGrowing ITKSmoothing ITKStatistics ITKIOGDCM)
if (TARGET zlibstatic)
    list(APPEND DEPENDENT_LIBS zlibstatic)
elseif (TARGET zlib)
    list(APPEND DEPENDENT_LIBS zlib)
else()
    list(APPEND DEPENDENT_LIBS ZLIB::ZLIB)
endif()
if (TARGET xml2)
    list(APPEND DEPENDENT_LIBS xml2)
else()
    list(APPEND DEPENDENT_LIBS LibXml2::LibXml2)
endif()
if (ZINC_USE_IMAGEMAGICK)
    list(APPEND DEPENDENT_LIBS MagickCore)
endif()
if (ZINC_USE_NETGEN)
    list(APPEND DEPENDENT_LIBS nglib)
endif()
if (ZINC_USE_OSMESA)
    list(APPEND DEPENDENT_LIBS GLEW::glew_s_osmesa)
else()
    list(APPEND DEPENDENT_LIBS GLEW::GLEW)
endif()

if(TARGET cmlibsdependencies)
    set(CMLIBSDEPENDENCIES_TARGET cmlibsdependencies)
    #set(DEPENDENT_LIBS ${CMLIBSDEPENDENCIES_TARGET})
endif()

if(IS_MULTI_CONFIG_ENV)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
else()
    set(_PARAM_ANNOTATION "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
    set(ZINC_BUILD_TYPE "Release" CACHE STRING ${_PARAM_ANNOTATION})
    if(DEFINED BUILD_TYPE)
        set(ZINC_BUILD_TYPE ${BUILD_TYPE} CACHE STRING ${_PARAM_ANNOTATION} FORCE)
    endif()
    set(CMAKE_BUILD_TYPE ${ZINC_BUILD_TYPE} CACHE INTERNAL "Internalise CMAKE_BUILD_TYPE, manipulate via ZINC_BUILD_TYPE" FORCE)
    unset(BUILD_TYPE CACHE)
endif()

if(NOT ZINC_BUILD_SHARED_LIBRARY AND NOT ZINC_BUILD_STATIC_LIBRARY )
    message(FATAL_ERROR "You have to build at least one of the types of libraries (shared or static)." )
endif()

if (NOT DEFINED ZINC_PACKAGE_CONFIG_DIR)
    set(ZINC_PACKAGE_CONFIG_DIR "share/cmake/ZINC")
endif ()
set(ZINC_PACKAGE_CONFIG_DIR "${ZINC_PACKAGE_CONFIG_DIR}" CACHE STRING "Path for CMake package configuration files (relative to CMAKE_INSTALL_PREFIX or an absolute path).")

if(NOT ZINC_BUILD_SHARED_LIBRARY AND NOT ZINC_BUILD_STATIC_LIBRARY )
    message(FATAL_ERROR "You have to build at least one of the types of libraries (shared or static)." )
endif()

find_package(Git QUIET)
find_package(SWIG QUIET)

set(ZINC_REVISION "no-rev")
if (GIT_FOUND)
    git_get_revision(ZINC_REVISION_LONG)
    string(SUBSTRING ${ZINC_REVISION_LONG} 0 6 ZINC_REVISION)
    git_is_dirty(_IS_DIRTY)
    if (_IS_DIRTY)
        set(ZINC_REVISION "${ZINC_REVISION}*")
    endif ()
endif()

set(USE_MSAA TRUE)

set(USE_GLEW TRUE)
#if(WIN32)
#    set(GLEW_STATIC TRUE)
#endif()

# Set graphics
if(NOT DEFINED GRAPHICS_API )
    set(GRAPHICS_API OPENGL_GRAPHICS )
endif()

# Set build type dependent #Defines
if(IS_MULTI_CONFIG_ENV)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DOPTIMISED" )
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DOPTIMISED" )
else()
    if(${CMAKE_BUILD_TYPE} MATCHES "[Dd]ebug")
        option(ZINC_BUILD_MEMORYCHECK "Do you want to build a memory check version" OFF )
        set(USE_PARAMETER_ON TRUE )
        if(NOT ${COMPILER_NAME} MATCHES "msvc")
            set(REPORT_GL_ERRORS )
        endif()
    elseif(${CMAKE_BUILD_TYPE} MATCHES "[Rr]elease")
        set(OPTIMISED TRUE )
    endif()
endif()

if(ZINC_BUILD_MEMORYCHECK )
    set(MEMORY_CHECKING TRUE )
endif()

# Set the configurable typedefs
set(ZnReal double )
set(FE_value ${ZnReal} )
set(FE_VALUE_INPUT_STRING \"%lf\" )
set(FE_VALUE_STRING \"22.15le\" )
set(COLOUR_PRECISION ${ZnReal} )
set(MATERIAL_PRECISION ${ZnReal} )

if(APPLE)
    find_library(ICONV_LIBRARY NAMES iconv)
    if(ICONV_LIBRARY )
        list(APPEND DEPENDENT_LIBS ${ICONV_LIBRARY})
    endif()
endif()

# Set Target name and global defines
SET_PLATFORM_DEFINES( )
SET_OPERATING_SYSTEM_DEFINES( ) # <-- mostly set automatically by cmake just adding WIN32_SYSTEM to be used instead of WIN32
SET_GRAPHICS_DEFINES( )

if(${GRAPHICS_API} MATCHES OPENGL_GRAPHICS AND NOT ZINC_USE_OSMESA)
    find_package(OpenGL REQUIRED )
    list(APPEND DEPENDENT_LIBS ${OPENGL_LIBRARIES})
elseif(${GRAPHICS_API} MATCHES OPENGL_GRAPHICS AND ZINC_USE_OSMESA)
    find_package(OSMesa REQUIRED)
    list(APPEND DEPENDENT_LIBS ${OSMesa_LIBRARIES} ${OSMesa_GLU_LIBRARIES})
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra -Wall -fvisibility=hidden -Wl,--as-needed" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wall -fvisibility=hidden -Wl,--as-needed" )
    set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} --as-needed" )
    if(${CMAKE_BUILD_TYPE} MATCHES [Rr]elease )
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter" )
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter" )
    endif(${CMAKE_BUILD_TYPE} MATCHES [Rr]elease )
elseif(MSVC)
    set(PLATFORM_COMPILER_DEFINITIONS ${PLATFORM_COMPILER_DEFINITIONS} _CRT_SECURE_NO_WARNINGS _CRTDBG_MAP_ALLOC)
endif()

TEST_FOR_VFSCANF( HAVE_VFSCANF )
include( CheckFunctionExists )
CHECK_FUNCTION_EXISTS( heapsort HAVE_HEAPSORT )

foreach(DEF ${DEPENDENT_DEFINITIONS} ${PLATFORM_COMPILER_DEFINITIONS})
    add_definitions(-D${DEF})
endforeach()
set(ZINC_STATIC_DEFINITIONS )
foreach( DEF ${DEPENDENT_DEFINITIONS} )
    LIST( APPEND ZINC_STATIC_DEFINITIONS -D${DEF} )
endforeach()
set(ZINC_SHARED_DEFINITIONS )

set(ZINC_API_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/src/api;${PROJECT_BINARY_DIR}/src/api")

# Set those before we define libraries in subfolders
set(ZINC_INSTALL_LIB_DIR lib)# "Installation directory for libraries"
set(ZINC_INSTALL_BIN_DIR bin)# "Installation directory for executables"
set(ZINC_INSTALL_INC_DIR include)# "Installation directory for header files"
set(ZINC_EXPORT_SET_NAME zinc-targets)

add_subdirectory(src) # Defines ZINC_LIBRARY_TARGETS

if(ZINC_BUILD_TESTS)
    enable_testing()

    # Set up the correct environment for the tests
    # See e.g. https://cmake.org/pipermail/cmake/2009-May/029464.html
    if (WIN32)
       set(LD_VARNAME "PATH")
    else()
       set(LD_VARNAME "LD_LIBRARY_PATH")
    endif()

    add_subdirectory(tests)
endif()

string(REGEX REPLACE ";" " " INCLUDE_DIRS "${INCLUDE_DIRS}" )
string(REGEX REPLACE ";" " " DEPENDENT_DEFINITIONS "${DEPENDENT_DEFINITIONS}")

# Register the Zinc project with the CMake database
export(PACKAGE ${ZINC_BASE_NAME})

# Install ZincLibraryDepends.cmake to the install destination
install(EXPORT ${ZINC_EXPORT_SET_NAME}
    DESTINATION ${ZINC_PACKAGE_CONFIG_DIR}
    COMPONENT Development
)
include(CMakePackageConfigHelpers)
WRITE_BASIC_PACKAGE_VERSION_FILE(${CMAKE_CURRENT_BINARY_DIR}/${ZINC_EXPORT_SET_NAME}-version.cmake
    COMPATIBILITY AnyNewerVersion)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${ZINC_EXPORT_SET_NAME}-version.cmake
    DESTINATION ${ZINC_PACKAGE_CONFIG_DIR}
    COMPONENT Development
)
set(DEPFILE ${CMAKE_CURRENT_BINARY_DIR}/${ZINC_EXPORT_SET_NAME}-dependencies.cmake)
file(WRITE ${DEPFILE}
    "include(CMakeFindDependencyMacro)\n"
    "set(${PROJECT_NAME}_IMPORT_PREFIX \${_IMPORT_PREFIX})\n"
)

file(APPEND ${DEPFILE} "if (ZINC_USE_STATIC)\n")
file(APPEND ${DEPFILE} "    set(USE_CMLIBSDEPENDENCIES TRUE)\n")
file(APPEND ${DEPFILE} "    set(CMLIBSDEPENDENCIES_DIR \"${CMLIBSDEPENDENCIES_DIR}\")\n")
file(APPEND ${DEPFILE} "    find_dependency(CMLIBSDEPENDENCIES)\n")
file(APPEND ${DEPFILE} "endif ()\n")
file(APPEND ${DEPFILE} "set(_IMPORT_PREFIX \${${PROJECT_NAME}_IMPORT_PREFIX})\n")

install(FILES ${DEPFILE}
    DESTINATION ${ZINC_PACKAGE_CONFIG_DIR}
    COMPONENT DevelopmentSDK
)

# Create the build tree imported targets
set(ZINC_CONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/zinc-config.cmake)
file(WRITE ${ZINC_CONFIG_FILE}
    "set(Zinc_VERSION ${Zinc_VERSION})\n"
    "get_filename_component(_HERE \"\${CMAKE_CURRENT_LIST_FILE}\" PATH)\n"
    "include(\"\${_HERE}/${ZINC_EXPORT_SET_NAME}-dependencies.cmake\")\n"
    "include(\"\${_HERE}/${ZINC_EXPORT_SET_NAME}.cmake\")\n"
)
file(APPEND ${ZINC_CONFIG_FILE} "
if (REQUIRE_INTERNAL_ZINC_INCLUDE_DIR)
    if (TARGET ${ZINC_STATIC_TARGET_NAME})
        set_property(TARGET ${ZINC_STATIC_TARGET_NAME} PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/src APPEND)
    endif ()
endif ()
"
)

install(FILES
    ${ZINC_CONFIG_FILE}
    DESTINATION ${ZINC_PACKAGE_CONFIG_DIR}
    COMPONENT DevelopmentSDK
)


# Packaging for the Zinc library
include(cmake/ZincPackaging.cmake)

# Output configuration information if requested
if(ZINC_BUILD_SHARED_LIBRARY )
    set(ZINC_BUILD_STRING "shared")
endif()
if(ZINC_BUILD_STATIC_LIBRARY )
    set(ZINC_BUILD_STRING ${ZINC_BUILD_STRING} static)
endif()
string(REPLACE ";" " and " ZINC_BUILD_STRING "${ZINC_BUILD_STRING}")

if(ZINC_PRINT_CONFIG_SUMMARY)
    message(STATUS "")
    message(STATUS "=====================================================")
    message(STATUS "Zinc has been configured with the following settings:")
    message(STATUS "")
    message(STATUS "             Version: '${Zinc_VERSION}${Zinc_DEVELOPER_VERSION}'")
    message(STATUS "     Source revision: '${ZINC_REVISION}'")
    message(STATUS "         Target type: '${ZINC_BUILD_STRING}'")
    message(STATUS "          Build type: '${CMAKE_BUILD_TYPE}'")
    message(STATUS "         Install dir: '${CMAKE_INSTALL_PREFIX}'")
    foreach(ZINCDEP ${ZINC_DEPS})
        if (${ZINCDEP}_FOUND)
            string(SUBSTRING "${ZINCDEP}              " 0 11 ZINCDEP_FIXED_SIZE)
            message(STATUS "         ${ZINCDEP_FIXED_SIZE}: YES")
        endif()
    endforeach()
    foreach(DEPLIB ${DEPENDENT_LIBS})
        if (TARGET ${DEPLIB})
            get_target_property(TARGET_TYPE ${DEPLIB} TYPE)
            if (TARGET_TYPE STREQUAL INTERFACE_LIBRARY)
                get_target_property(TARGET_ILR ${DEPLIB} INTERFACE_LINK_LIBRARIES)
                set(TARGET_ILR "Interface to ${TARGET_ILR}")
            else()
                get_target_property(TARGET_ILR ${DEPLIB} IMPORTED_LOCATION_RELEASE)
                if (NOT TARGET_ILR)
                    get_target_property(TARGET_ILR ${DEPLIB} IMPORTED_LOCATION_DEBUG)
                endif()
            endif()
            string(SUBSTRING "${DEPLIB}              " 0 11 DEPLIB_FIXED_SIZE)
            message(STATUS "         ${DEPLIB_FIXED_SIZE}: '${TARGET_ILR}'")
        endif()
    endforeach()
    message(STATUS "=====================================================")
    message(STATUS "")
endif()

