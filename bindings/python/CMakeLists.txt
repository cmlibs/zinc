# OpenCMISS-Zinc Library
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Create Python bindings for the Zinc library.

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(PyZinc)

INCLUDE( Functions.cmake )

SET(PYTHON_VERSIONS "2.6;2.7;3.3" CACHE STRING "Identify your preferred version or versions of Python to locate. [2.6, 2.7, 3.0, 3.3]")

IF( PYZINC_MODULE_PATH AND NOT IS_ABSOLUTE ${PYZINC_MODULE_PATH} )
	SET(_TMP_PATH "${PYZINC_MODULE_PATH}")
	UNSET(PYZINC_MODULE_PATH CACHE)
	GET_FILENAME_COMPONENT(PYZINC_MODULE_PATH "${_TMP_PATH}" REALPATH CACHE)
ENDIF()

SET(PYZINC_MODULE_PATH "${PYZINC_MODULE_PATH}" CACHE PATH "Location to the PyZinc modules, specifically we need a modified FindSWIG.cmake module.")
SET(CMAKE_MODULE_PATH ${PYZINC_MODULE_PATH})

SET(PYZINC_BUILD_TYPE "Release" CACHE STRING "The type of configuration to build: Debug, Release, MinSizeRel, RelWithDebugInfo, default: Release")
SET(CMAKE_BUILD_TYPE ${PYZINC_BUILD_TYPE} CACHE INTERNAL "Internalise CMAKE_BUILD_TYPE, manipulate via ZINC_BUILD_TYPE" FORCE)

FIND_PACKAGE(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})

SET(Python_ADDITIONAL_VERSIONS ${PYTHON_VERSIONS})
# Force the finding of the python library always incase the desired PYTHON_VERSIONS has changed.
UNSET(PYTHONLIBS_FOUND CACHE)
UNSET(PYTHON_LIBRARY CACHE)
UNSET(PYTHON_INCLUDE_DIR CACHE)
UNSET(PYTHONINTERP_FOUND CACHE)
UNSET(PYTHON_EXECUTABLE CACHE)
FIND_PACKAGE(PythonLibs REQUIRED)
FIND_PACKAGE(PythonInterp REQUIRED)

INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})

IF(${CMAKE_VERSION} VERSION_LESS "2.8.8")
	IF(PYTHON_INCLUDE_DIRS AND EXISTS "${PYTHON_INCLUDE_DIR}/patchlevel.h")
		FILE(STRINGS "${PYTHON_INCLUDE_DIR}/patchlevel.h" python_version_str
			REGEX "^#define[ \t]+PY_VERSION[ \t]+\"[^\"]+\"")
		STRING(REGEX REPLACE "^#define[ \t]+PY_VERSION[ \t]+\"([^\"]+)\".*" "\\1"
			PYTHONLIBS_VERSION_STRING "${python_version_str}")
		UNSET(python_version_str)
	ENDIF()
ENDIF()

STRING(SUBSTRING ${PYTHONLIBS_VERSION_STRING} 0 1 PYTHONLIBS_MAJOR_VERSION)
IF(${PYTHONLIBS_MAJOR_VERSION} STREQUAL "3")
	SET(PYTHONLIBS_SWIG_FLAGS -py3)
ENDIF()

SET( PATH_SUFFIXES cmake )
FIND_PACKAGE(Zinc REQUIRED)
INCLUDE_DIRECTORIES(${ZINC_INCLUDE_DIRS})

# Suppress the overload operator warning for Python.
SET(CMAKE_SWIG_FLAGS -w381 -w382 -w361 -w362 ${PYTHONLIBS_SWIG_FLAGS})

SET(INTERFACE_SRCS
	../interface/context.i
	../interface/differentialoperator.i
	../interface/element.i
	../interface/field.i
	../interface/fieldmodule.i
	../interface/fieldcache.i
	../interface/fieldsmoothing.i
	../interface/font.i
	../interface/glyph.i
	../interface/graphics.i
	../interface/light.i
	../interface/material.i
	../interface/optimisation.i
	../interface/node.i
	../interface/scene.i
	../interface/scenecoordinatesystem.i
	../interface/scenefilter.i
	../interface/scenepicker.i
	../interface/sceneviewer.i
	../interface/sceneviewerinput.i
	../interface/selection.i
	../interface/spectrum.i
	../interface/region.i
	../interface/status.i
	../interface/stream.i
	../interface/streamimage.i
	../interface/streamregion.i
	../interface/streamscene.i
	../interface/tessellation.i
	../interface/timekeeper.i
	../interface/timenotifier.i
	../interface/timesequence.i)

SET_SOURCE_FILES_PROPERTIES(${INTERFACE_SRCS} PROPERTIES CPLUSPLUS ON)

SET(OPENCMISS_PYTHON_PACKAGE opencmiss)
SET(ZINC_PYTHON_PACKAGE zinc)

SET(CMAKE_SWIG_OUTDIR ${OPENCMISS_PYTHON_PACKAGE}/${ZINC_PYTHON_PACKAGE})

SET(SWIG_MODULE_TARGETS)
FOREACH(SWIG_INTERFACE ${INTERFACE_SRCS})
	GET_FILENAME_COMPONENT(MODULE ${SWIG_INTERFACE} NAME_WE)
	SWIG_ADD_MODULE(${MODULE} python ${SWIG_INTERFACE})
	SWIG_LINK_LIBRARIES(${MODULE} ${PYTHON_LIBRARIES} ${ZINC_LIBRARIES})
	SET_TARGET_PROPERTIES(${SWIG_MODULE_${MODULE}_REAL_NAME} PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SWIG_OUTDIR}
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SWIG_OUTDIR})
	LIST(APPEND SWIG_MODULE_TARGETS ${SWIG_MODULE_${MODULE}_REAL_NAME})
ENDFOREACH()

SET(PYPI_SRCS
	CHANGES.txt
	LICENSE.txt
	README.txt
	docs/main.rst)

SET(PYZINC_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
SET(ZINC_SHARED_OBJECT_GLOB "*.so")
IF(WIN32)
	# Sometimes we cross-compile on Windows so we'll set
	# the PYZINC_ARCHITECTURE manually
	IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
		SET(PYZINC_ARCHITECTURE x86)
	ELSE()
		SET(PYZINC_ARCHITECTURE amd64)
	ENDIF()
	SET(ZINC_SHARED_OBJECT_GLOB "*.pyd")
ENDIF()
IF(APPLE)
	SET(PYZINC_ARCHITECTURE universal)
ENDIF()

STRING(SUBSTRING ${PYTHONLIBS_VERSION_STRING} 0 3 PYTHONLIBS_MAJOR_MINOR_VERSION)
GET_TARGET_PROPERTY(ZINC_LOCATION ${ZINC_LIBRARIES} LOCATION_RELEASE)
STRING(REGEX MATCH "\\.r[0-9]+[M]?" ZINC_DEVELOPER_VERSION "${ZINC_LOCATION}")
STRING(REGEX MATCH "\\.([0-9]+\\.[0-9]+\\.[0-9]+)" ZINC_VERSION "${ZINC_LOCATION}")
SET( ZINC_VERSION ${CMAKE_MATCH_1} )

# Create the Python package initialisation files.
CONFIGURE_FILE(createpackagefiles.in.cmake createpackagefiles.cmake)
ADD_CUSTOM_COMMAND(TARGET ${SWIG_MODULE_TARGETS} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/createpackagefiles.cmake"
	COMMENT "Create Python package initialisation files")

# Copy over the zinc library linked to
ADD_CUSTOM_COMMAND(TARGET ${SWIG_MODULE_TARGETS} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ZINC_LOCATION}" "${CMAKE_SWIG_OUTDIR}")


# Get a pretty system name.
GET_SYSTEM_NAME(SYSTEM_NAME)

STRING(TOLOWER ${PROJECT_NAME} PYPI_PACKAGE_NAME)
SET(PYPI_SOURCE_TARGZ_VERSION "${ZINC_VERSION}${ZINC_DEVELOPER_VERSION}-${PYZINC_ARCHITECTURE}-${SYSTEM_NAME}-Python-${PYTHONLIBS_MAJOR_MINOR_VERSION}")

SET(CONFIGURED_SETUP_FILE "${CMAKE_CURRENT_BINARY_DIR}/setup.py")
CONFIGURE_FILE(setup.in.py ${CONFIGURED_SETUP_FILE} @ONLY)
SET(MANIFEST_IN_FILE "${CMAKE_CURRENT_BINARY_DIR}/MANIFEST.in")
CONFIGURE_FILE( "${CMAKE_CURRENT_SOURCE_DIR}/MANIFEST.in" "${MANIFEST_IN_FILE}")

SET(PYPI_GENERATED_SRCS ${CONFIGURED_SETUP_FILE} ${MANIFEST_IN_FILE})
FOREACH(_file ${PYPI_SRCS})
	CONFIGURE_FILE(${_file} "${CMAKE_CURRENT_BINARY_DIR}/${_file}" @ONLY)
	LIST(APPEND PYPI_GENERATED_SRCS "${CMAKE_CURRENT_BINARY_DIR}/${_file}")
ENDFOREACH()

SET(PYPI_SOURCE_TARGZ "${CMAKE_CURRENT_BINARY_DIR}/dist/${PYPI_PACKAGE_NAME}-${PYPI_SOURCE_TARGZ_VERSION}.tar.gz")
ADD_CUSTOM_COMMAND(OUTPUT "${PYPI_SOURCE_TARGZ}"
	COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_CURRENT_BINARY_DIR}" ${PYTHON_EXECUTABLE} setup.py clean
	COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_CURRENT_BINARY_DIR}" ${PYTHON_EXECUTABLE} setup.py sdist
	#COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_CURRENT_BINARY_DIR}" ${PYTHON_EXECUTABLE} setup.py bdist_egg
	DEPENDS ${PYPI_GENERATED_SRCS})
ADD_CUSTOM_TARGET(pypi-package DEPENDS "${PYPI_SOURCE_TARGZ}")

# Add custom command to add additional files to the clean target
ADD_CUSTOM_COMMAND(OUTPUT ${OPENCMISS_PYTHON_PACKAGE} ${PYPI_GENERATED_SRCS}#"${MANIFEST_IN_FILE}" "${CONFIGURED_SETUP_FILE}"
	COMMAND ${CMAKE_COMMAND} -E echo "Dummy command for cleaning generated files.")
ADD_CUSTOM_TARGET(dontuse DEPENDS ${OPENCMISS_PYTHON_PACKAGE})

