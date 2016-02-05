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

set(BASE_PYTHON_PACKAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
set(OPENCMISS_PYTHON_PACKAGE_DIR "${BASE_PYTHON_PACKAGE_DIR}/opencmiss")
set(ZINC_PYTHON_PACKAGE_DIR "${OPENCMISS_PYTHON_PACKAGE_DIR}/zinc")
set(CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/modules_python_files")

set(SWIG_MODULE_TARGETS )
set(MODULE_PY_FILES )
set(SETUP_PY_PACKAGE_FILES )
foreach(SWIG_INTERFACE ${INTERFACE_SRCS})
	get_filename_component(MODULE_NAME ${SWIG_INTERFACE} NAME_WE)
	list(APPEND MODULE_PY_FILES ${MODULE_NAME}.py)
	SWIG_ADD_MODULE(${MODULE_NAME} python ${SWIG_INTERFACE})
	SWIG_LINK_LIBRARIES(${MODULE_NAME} ${PYTHON_LIBRARIES} zinc)
	set(MODULE_TARGET ${SWIG_MODULE_${MODULE_NAME}_REAL_NAME})
	set_target_properties(${MODULE_TARGET} PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY ${ZINC_PYTHON_PACKAGE_DIR}
		RUNTIME_OUTPUT_DIRECTORY ${ZINC_PYTHON_PACKAGE_DIR}
    )
	list(APPEND SWIG_MODULE_TARGETS ${MODULE_TARGET})
	list(APPEND SETUP_PY_PACKAGE_FILES "'$<TARGET_FILE_NAME:${MODULE_TARGET}>'")
endforeach()
add_custom_target(collect_python_binding_files ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SWIG_OUTDIR} ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/opencmiss/zinc
)

set(PYZINC_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
if(WIN32)
	# Sometimes we cross-compile on Windows so we'll set
	# the PYZINC_ARCHITECTURE manually
	if(CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(PYZINC_ARCHITECTURE x86)
	else()
		set(PYZINC_ARCHITECTURE amd64)
	endif()
endif()
if(APPLE)
	set(PYZINC_ARCHITECTURE universal)
endif()

string(SUBSTRING ${PYTHONLIBS_VERSION_STRING} 0 3 PYTHONLIBS_MAJOR_MINOR_VERSION)

# Get a pretty system name.
GET_SYSTEM_NAME(SYSTEM_NAME)

string(TOLOWER ${PROJECT_NAME} PYPI_PACKAGE_NAME)
set(PYPI_SOURCE_TARGZ_VERSION "${Zinc_VERSION}${ZINC_DEVELOPER_VERSION}-${PYZINC_ARCHITECTURE}-${SYSTEM_NAME}-Python-${PYTHONLIBS_MAJOR_MINOR_VERSION}")


################################################################
# Generate the required package files
set(ADDITIONAL_PACKAGE_FILES
	CHANGES.txt
	LICENSE.txt
	README.txt
	docs/main.rst)

# 1. Configure stage
set(SETUP_PY_PACKAGE_FILES_STR "${SETUP_PY_PACKAGE_FILES}")
string(REPLACE ";" "," SETUP_PY_PACKAGE_FILES_STR "${SETUP_PY_PACKAGE_FILES_STR}")
configure_file(setup.in.py "${CMAKE_CURRENT_BINARY_DIR}/setup.py" @ONLY)

file(TO_NATIVE_PATH "${CMAKE_INSTALL_PREFIX}/lib" NATIVE_LIBRARY_PATH)
file(TO_NATIVE_PATH "${CMAKE_INSTALL_PREFIX}/bin" NATIVE_BINARY_PATH)
configure_file(zinc__init__.in.py "${CMAKE_CURRENT_BINARY_DIR}/zinc__init__.py" @ONLY)

foreach(_file ${ADDITIONAL_PACKAGE_FILES})
    configure_file(${_file} "${CMAKE_CURRENT_BINARY_DIR}/${_file}")
endforeach()
#set(MANIFEST_IN_FILE "${CMAKE_CURRENT_BINARY_DIR}/MANIFEST.in")
#configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/MANIFEST.in" "${MANIFEST_IN_FILE}")

# 2. Generate stage - takes care of all the build types (think of multiconf environment like VS)
file(GENERATE OUTPUT "${BASE_PYTHON_PACKAGE_DIR}/setup.py"
    INPUT "${CMAKE_CURRENT_BINARY_DIR}/setup.py"
)
# opencmiss __init__.py does not need to be configured (yet)
file(GENERATE OUTPUT "${OPENCMISS_PYTHON_PACKAGE_DIR}/__init__.py"
    INPUT "${CMAKE_CURRENT_SOURCE_DIR}/opencmiss__init__.py"
)
file(GENERATE OUTPUT "${ZINC_PYTHON_PACKAGE_DIR}/__init__.py"
    INPUT "${CMAKE_CURRENT_BINARY_DIR}/zinc__init__.py"
)
foreach(_file ${ADDITIONAL_PACKAGE_FILES})
	file(GENERATE OUTPUT "${BASE_PYTHON_PACKAGE_DIR}/${_file}"
	    INPUT "${CMAKE_CURRENT_BINARY_DIR}/${_file}"
	)
endforeach()

#################################
# Create tarball
set(PYPI_SOURCE_TARGZ "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${PYPI_PACKAGE_NAME}-${PYPI_SOURCE_TARGZ_VERSION}.tar.gz")
add_custom_target(python-bindings-tarball
    COMMAND ${CMAKE_COMMAND} -E tar czf ${PYPI_SOURCE_TARGZ} -- $<CONFIG>/opencmiss setup.py ${ADDITIONAL_PACKAGE_FILES}
)

include(${CMAKE_CURRENT_LIST_DIR}/InstallConfig.cmake)
