# This example shows how to use python

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(ZincPython)

SET(PYTHON_VERSIONS "2.6;2.7" CACHE STRING "Identify your preferred version or versions of Python to locate. [2.6, 2.7, 3.0, 3.3]")

FIND_PACKAGE(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})

SET(Python_ADDITIONAL_VERSIONS ${PYTHON_VERSIONS})
# Force the finding of the python library always incase the desired PYTHON_VERSIONS has changed.
UNSET(PYTHONLIBS_FOUND CACHE)
UNSET(PYTHON_LIBRARY CACHE)
UNSET(PYTHON_INCLUDE_DIR CACHE)
FIND_PACKAGE(PythonLibs REQUIRED)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})
STRING(SUBSTRING ${PYTHONLIBS_VERSION_STRING} 0 1 PYTHONLIBS_MAJOR_VERSION)

IF(${CMAKE_VERSION} VERSION_LESS "2.8.8")
    IF(PYTHON_INCLUDE_DIRS AND EXISTS "${PYTHON_INCLUDE_DIR}/patchlevel.h")
        FILE(STRINGS "${PYTHON_INCLUDE_DIR}/patchlevel.h" python_version_str
            REGEX "^#define[ \t]+PY_VERSION[ \t]+\"[^\"]+\"")
        STRING(REGEX REPLACE "^#define[ \t]+PY_VERSION[ \t]+\"([^\"]+)\".*" "\\1"
            PYTHONLIBS_VERSION_STRING "${python_version_str}")
        UNSET(python_version_str)
    ENDIF()
ENDIF()

IF(${PYTHONLIBS_MAJOR_VERSION} STREQUAL "3")
    SET(PYTHONLIBS_SWIG_FLAGS -py3)
ENDIF()

SET( PATH_SUFFIXES cmake )
FIND_PACKAGE(Zinc)
INCLUDE_DIRECTORIES(${ZINC_INCLUDE_DIRS})

# Suppress the assignment operator overload warning for Python.
SET(CMAKE_SWIG_FLAGS -w362 ${PYTHONLIBS_SWIG_FLAGS})

SET(INTERFACE_SRCS
    ../interface/context.i
    ../interface/region.i
    ../interface/fieldmodule.i
    ../interface/graphicsmodule.i
    ../interface/graphicsmaterial.i
    ../interface/rendition.i
    ../interface/field.i
    ../interface/stream.i
    ../interface/element.i
    ../interface/node.i
    ../interface/differentialoperator.i
    ../interface/fieldcache.i
    ../interface/tessellation.i
    ../interface/graphic.i
    ../interface/selection.i
    ../interface/graphicsfilter.i
    ../interface/scene.i
    ../interface/sceneviewer.i
    ../interface/spectrum.i
    ../interface/optimisation.i
    ../interface/timenotifier.i
    ../interface/timekeeper.i
    ../interface/timesequence.i)

SET_SOURCE_FILES_PROPERTIES(${INTERFACE_SRCS} PROPERTIES CPLUSPLUS ON)

SET(ZINC_PYTHON_MODULE zinc)
SET(CMAKE_SWIG_OUTDIR ${ZINC_PYTHON_MODULE})

FOREACH(SWIG_INTERFACE ${INTERFACE_SRCS})
    GET_FILENAME_COMPONENT(MODULE ${SWIG_INTERFACE} NAME_WE)
    SWIG_ADD_MODULE(${MODULE} python ${SWIG_INTERFACE})
    SWIG_LINK_LIBRARIES(${MODULE} ${PYTHON_LIBRARIES} ${ZINC_LIBRARIES})
    SET_TARGET_PROPERTIES(${SWIG_MODULE_${MODULE}_REAL_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${ZINC_PYTHON_MODULE}
        RUNTIME_OUTPUT_DIRECTORY ${ZINC_PYTHON_MODULE})
ENDFOREACH()

FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${ZINC_PYTHON_MODULE}/__init__.py" "# Zinc Python package initialisation file.\n")

