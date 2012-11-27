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

# Suppress the assignment operator overload warning for Python.
SET(CMAKE_SWIG_FLAGS -w362 -py3)

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

FOREACH(SWIG_INTERFACE ${INTERFACE_SRCS})
    GET_FILENAME_COMPONENT(MODULE ${SWIG_INTERFACE} NAME_WE)
    SWIG_ADD_MODULE(${MODULE} python ${SWIG_INTERFACE})
    SWIG_LINK_LIBRARIES(${MODULE} ${PYTHON_LIBRARIES} ${ZINC_LIBRARIES})
ENDFOREACH()

