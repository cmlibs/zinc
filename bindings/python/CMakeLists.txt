# OpenCMISS-Zinc Library
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Create Python bindings for the Zinc library.

set(PYTHON_VERSIONS "2.6;2.7;3.3" CACHE STRING "Identify your preferred version or versions of Python to locate. [2.6, 2.7, 3.0, 3.3]")

# SWIG has already been found at this point.
include(${SWIG_USE_FILE})

set(Python_ADDITIONAL_VERSIONS ${PYTHON_VERSIONS})
# Force the finding of the python library always incase the desired PYTHON_VERSIONS has changed.
unset(PYTHONLIBS_FOUND CACHE)
unset(PYTHON_LIBRARY CACHE)
unset(PYTHON_INCLUDE_DIR CACHE)
unset(PYTHONINTERP_FOUND CACHE)
unset(PYTHON_EXECUTABLE CACHE)
find_package(PythonLibs QUIET)
find_package(PythonInterp QUIET)

if(${CMAKE_VERSION} VERSION_LESS "2.8.8")
	if(PYTHON_INCLUDE_DIRS AND EXISTS "${PYTHON_INCLUDE_DIR}/patchlevel.h")
		file(STRINGS "${PYTHON_INCLUDE_DIR}/patchlevel.h" python_version_str
			REGEX "^#define[ \t]+PY_VERSION[ \t]+\"[^\"]+\"")
		string(REGEX REPLACE "^#define[ \t]+PY_VERSION[ \t]+\"([^\"]+)\".*" "\\1"
			PYTHONLIBS_VERSION_STRING "${python_version_str}")
		unset(python_version_str)
	endif()
endif()

string(SUBSTRING ${PYTHONLIBS_VERSION_STRING} 0 1 PYTHONLIBS_MAJOR_VERSION)
if(${PYTHONLIBS_MAJOR_VERSION} STREQUAL "3")
	set(PYTHONLIBS_SWIG_FLAGS -py3)
endif()

# We have to set the include directories onto the directory because the swig
# wrapping process requires the include directories.
include_directories(${ZINC_API_INCLUDE_DIR} ${PYTHON_INCLUDE_DIRS})

# Suppress the overload operator warning for Python.
set(CMAKE_SWIG_FLAGS -w381 -w382 -w361 -w362 ${PYTHONLIBS_SWIG_FLAGS})

set(INTERFACE_SRCS
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
	../interface/logger.i
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

set_source_files_properties(${INTERFACE_SRCS} PROPERTIES CPLUSPLUS ON)

set(OPENCMISS_PYTHON_PACKAGE opencmiss)
set(ZINC_PYTHON_PACKAGE zinc)

set(CMAKE_SWIG_OUTDIR ${OPENCMISS_PYTHON_PACKAGE}/${ZINC_PYTHON_PACKAGE})
set(SWIG_MODULE_TARGETS)
foreach(SWIG_INTERFACE ${INTERFACE_SRCS})
	get_filename_component(MODULE ${SWIG_INTERFACE} NAME_WE)
	SWIG_ADD_MODULE(${MODULE} python ${SWIG_INTERFACE})
	SWIG_LINK_LIBRARIES(${MODULE} ${PYTHON_LIBRARIES} zinc)
	set_target_properties(${SWIG_MODULE_${MODULE}_REAL_NAME} PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SWIG_OUTDIR}
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SWIG_OUTDIR})
	list(APPEND SWIG_MODULE_TARGETS ${SWIG_MODULE_${MODULE}_REAL_NAME})
endforeach()

set(PYPI_SRCS
	CHANGES.txt
	LICENSE.txt
	README.txt
	docs/main.rst)

set(PYZINC_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
set(ZINC_SHARED_OBJECT_GLOB "*.so")
if(WIN32)
	# Sometimes we cross-compile on Windows so we'll set
	# the PYZINC_ARCHITECTURE manually
	if(CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(PYZINC_ARCHITECTURE x86)
	else()
		set(PYZINC_ARCHITECTURE amd64)
	endif()
	set(ZINC_SHARED_OBJECT_GLOB "*.pyd")
endif()
if(APPLE)
	set(PYZINC_ARCHITECTURE universal)
endif()

string(SUBSTRING ${PYTHONLIBS_VERSION_STRING} 0 3 PYTHONLIBS_MAJOR_MINOR_VERSION)

# Create the Python package initialisation files.
configure_file(createpackagefiles.in.cmake createpackagefiles.cmake)
add_custom_command(TARGET ${SWIG_MODULE_TARGETS} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/createpackagefiles.cmake"
	COMMENT "Create Python package initialisation files")

# Get a pretty system name.
GET_SYSTEM_NAME(SYSTEM_NAME)

string(TOLOWER ${PROJECT_NAME} PYPI_PACKAGE_NAME)
set(PYPI_SOURCE_TARGZ_VERSION "${Zinc_VERSION}${ZINC_DEVELOPER_VERSION}-${PYZINC_ARCHITECTURE}-${SYSTEM_NAME}-Python-${PYTHONLIBS_MAJOR_MINOR_VERSION}")

set(CONFIGURED_SETUP_FILE "${CMAKE_CURRENT_BINARY_DIR}/setup.py")
configure_file(setup.in.py ${CONFIGURED_SETUP_FILE} @ONLY)
set(MANIFEST_IN_FILE "${CMAKE_CURRENT_BINARY_DIR}/MANIFEST.in")
configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/MANIFEST.in" "${MANIFEST_IN_FILE}")

set(PYPI_GENERATED_SRCS ${CONFIGURED_SETUP_FILE} ${MANIFEST_IN_FILE})
foreach(_file ${PYPI_SRCS})
	configure_file(${_file} "${CMAKE_CURRENT_BINARY_DIR}/${_file}" @ONLY)
	list(APPEND PYPI_GENERATED_SRCS "${CMAKE_CURRENT_BINARY_DIR}/${_file}")
endforeach()

set(PYPI_SOURCE_TARGZ "${CMAKE_CURRENT_BINARY_DIR}/dist/${PYPI_PACKAGE_NAME}-${PYPI_SOURCE_TARGZ_VERSION}.tar.gz")
add_custom_command(OUTPUT "${PYPI_SOURCE_TARGZ}"
	COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_CURRENT_BINARY_DIR}" ${PYTHON_EXECUTABLE} setup.py clean
	COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_CURRENT_BINARY_DIR}" ${PYTHON_EXECUTABLE} setup.py sdist
	#COMMAND ${CMAKE_COMMAND} -E chdir "${CMAKE_CURRENT_BINARY_DIR}" ${PYTHON_EXECUTABLE} setup.py bdist_egg
	DEPENDS ${PYPI_GENERATED_SRCS})
add_custom_target(pypi-package DEPENDS "${PYPI_SOURCE_TARGZ}")

# Add custom command to add additional files to the clean target
add_custom_command(OUTPUT ${OPENCMISS_PYTHON_PACKAGE} ${PYPI_GENERATED_SRCS}#"${MANIFEST_IN_FILE}" "${CONFIGURED_SETUP_FILE}"
	COMMAND ${CMAKE_COMMAND} -E echo "Dummy command for cleaning generated files.")
add_custom_target(dontuse DEPENDS ${OPENCMISS_PYTHON_PACKAGE})

# Install step for virtual environment (if given)
if (VIRTUALENV_LOCATION)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/install_virtualenv.in.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/install_virtualenv.cmake @ONLY)
    install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/install_virtualenv.cmake)
endif()

