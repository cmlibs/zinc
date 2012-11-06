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

#INCLUDE_DIRECTORIES(../../core/source)

SET(CMAKE_SWIG_FLAGS "")


SET(INTERFACE_SRCS
    ../interface/ZnContext.i
    ../interface/ZnRegion.i
    ../interface/ZnFieldModule.i
    ../interface/ZnGraphicsModule.i
    ../interface/ZnMaterial.i
    ../interface/ZnRendition.i
    ../interface/ZnField.i
    ../interface/ZnStream.i
    ../interface/ZnElement.i
    ../interface/ZnNode.i
    ../interface/ZnDifferentialOperator.i
    ../interface/ZnFieldCache.i
    ../interface/ZnTessellation.i
    ../interface/ZnGraphic.i
    ../interface/ZnSelection.i
    ../interface/ZnGraphicsFilter.i
    ../interface/ZnScene.i
    ../interface/ZnSceneViewer.i
    ../interface/ZnSpectrum.i
    ../interface/ZnOptimisation.i
    ../interface/ZnTime.i
    ../interface/ZnTimeSequence.i)

SET(ZINC_PYTHON_MODULE zinc)
SET(CMAKE_SWIG_OUTDIR ${ZINC_PYTHON_MODULE})
SET_SOURCE_FILES_PROPERTIES(${INTERFACE_SRCS} PROPERTIES CPLUSPLUS ON)
# SET_SOURCE_FILES_PROPERTIES(${INTERFACE_SRCS} PROPERTIES SWIG_FLAGS "-includeall")

FOREACH(SWIG_INTERFACE ${INTERFACE_SRCS})
    #GET_FILENAME_COMPONENT(SWIG_INTERFACE ${SWIG_INTERFACE} ABSOLUTE)
    GET_FILENAME_COMPONENT(MODULE ${SWIG_INTERFACE} NAME_WE)
    STRING(REPLACE "Zn" "" MODULE ${MODULE})
    SWIG_ADD_MODULE(${MODULE} python ${SWIG_INTERFACE})
    SWIG_LINK_LIBRARIES(${MODULE} ${PYTHON_LIBRARIES} ${ZINC_LIBRARIES})
    SET_TARGET_PROPERTIES(${SWIG_MODULE_${MODULE}_REAL_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${ZINC_PYTHON_MODULE}
        RUNTIME_OUTPUT_DIRECTORY ${ZINC_PYTHON_MODULE})
ENDFOREACH()

FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${ZINC_PYTHON_MODULE}/__init__.py" "# Cmiss Python package initialisation file.\n")

