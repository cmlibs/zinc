# This example shows how to use python

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(ZincPython)

FIND_PACKAGE(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})

FIND_PACKAGE(PythonLibs)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

SET( PATH_SUFFIXES cmake )
FIND_PACKAGE(Zinc)
INCLUDE_DIRECTORIES(${ZINC_INCLUDE_DIRS})

SET(CMAKE_SWIG_FLAGS "-w362")


SET(INTERFACE_SRCS
    ../interface/context.i
    ../interface/region.i
    ../interface/fieldmodule.i
    ../interface/graphicsmodule.i
    ../interface/material.i
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
    ../interface/time.i
    ../interface/timesequence.i)

SET(ZINC_PYTHON_MODULE zinc)
SET(CMAKE_SWIG_OUTDIR ${ZINC_PYTHON_MODULE})
SET_SOURCE_FILES_PROPERTIES(${INTERFACE_SRCS} PROPERTIES CPLUSPLUS ON)
# SET_SOURCE_FILES_PROPERTIES(${INTERFACE_SRCS} PROPERTIES SWIG_FLAGS "-includeall")

FOREACH(SWIG_INTERFACE ${INTERFACE_SRCS})
    #GET_FILENAME_COMPONENT(SWIG_INTERFACE ${SWIG_INTERFACE} ABSOLUTE)
    GET_FILENAME_COMPONENT(MODULE ${SWIG_INTERFACE} NAME_WE)
    SWIG_ADD_MODULE(${MODULE} python ${SWIG_INTERFACE})
    SWIG_LINK_LIBRARIES(${MODULE} ${PYTHON_LIBRARIES} ${ZINC_LIBRARIES})
    SET_TARGET_PROPERTIES(${SWIG_MODULE_${MODULE}_REAL_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${ZINC_PYTHON_MODULE}
        RUNTIME_OUTPUT_DIRECTORY ${ZINC_PYTHON_MODULE})
ENDFOREACH()

FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${ZINC_PYTHON_MODULE}/__init__.py" "# Cmiss Python package initialisation file.\n")

