# - Try to find OpenCascade
# Once done, this will define
#
# OpenCascade_FOUND - OpenCascade is installed
# OpenCascade_INCLUDE_DIRS
# OpenCascade_LIB_DIR
# OpenCascade_LIBRARIES

# include(LibFindMacros)

if (OpenCascade_INCLUDE_DIR AND OpenCascade_LIB_DIR)
  SET (OpenCascade_INCLUDE_DIR OpenCascade_INCLUDE_DIRS-NOTFOUND)
  SET (OpenCascade_LIB_DIR OpenCascade_LIB-NOTFOUND)
endif (OpenCascade_INCLUDE_DIR AND OpenCascade_LIB_DIR)

find_path(X11_LIB_DIR
	NAMES libX11.so
	PATHS
	/lib
	/usr/lib
	/usr/local/lib
	/usr/X11/lib
	/usr/X11R5/lib
	/usr/X11R6/lib
	)

if (X11_LIB_DIR)
	MESSAGE(STATUS "X11 LIB Dir found: ${X11_LIB_DIR}")
endif (X11_LIB_DIR)



MESSAGE(STATUS "Looking for OpenCascade...")

#SET(CASROOT /home/stockman/OpenCascade/OpenCascade6.2/ros)
MESSAGE(STATUS "CASROOT: ${CASROOT}")

find_path(OpenCascade_INCLUDE_DIR
	NAMES STEPCAFControl_Reader.hxx
	PATHS ${CASROOT}/inc
	NO_DEFAULT_PATH
)

if (OpenCascade_INCLUDE_DIR)
	MESSAGE(STATUS "Include Dir found: ${OpenCascade_INCLUDE_DIR}")
endif (OpenCascade_INCLUDE_DIR)

find_path(OpenCascade_LIB_DIR
	NAMES libTKSTEP.so
	PATHS ${CASROOT}/Linux/lib
	NO_DEFAULT_PATH
)

if (OpenCascade_LIB_DIR)
	MESSAGE(STATUS "LIB Dir found: ${OpenCascade_LIB_DIR}")
	SET(OpenCascade_LIBRARIES -L${OpenCascade_LIB_DIR}
		-lTKernel
		-lTKMath
		-lTKG2d
		-lTKG3d
		-lTKGeomBase
		-lTKBRep
		-lTKGeomAlgo
		-lTKTopAlgo
		-lTKPrim
		-lTKBool
		-lTKFeat
		-lTKFillet
		-lTKOffset
		-lTKHLR
		-lTKService
		-lTKV2d
		-lTKV3d
		-lTKMesh
		-lTKPCAF
		-lTKLCAF
		-lTKPLCAF
		-lTKCDF
		-lTKCAF
		-lPTKernel
		-lTKIGES
		-lTKSTEP
		-lTKSTEPBase
		-lTKSTEPAttr
		-lTKSTEP209
		-lTKXCAF
		-lTKXDESTEP
		-lTKXDEIGES
		-lTKSTL
		-lTKVRML
		-lTKShHealing
		-lTKXSBase
		-lTKPShape	
		-lTKBO
		-L${X11_LIB_DIR}
		-lX11
		-lXext
		-lXmu
		-lGL
		-lGLU
	)
endif (OpenCascade_LIB_DIR)

if (OpenCascade_INCLUDE_DIR AND OpenCascade_LIB_DIR)
	set(OpenCascade_FOUND TRUE)
endif (OpenCascade_INCLUDE_DIR AND OpenCascade_LIB_DIR)

if (OpenCascade_FOUND)
	MESSAGE(STATUS "Looking for OpenCascade... - found ${OpenCascade_LIB_DIR}")
	SET(LD_LIBRARY_PATH ${LD_LIBRARY_PATH} ${OpenCascade_LIB_DIR})
else (OpenCascade_FOUND)
    if (OpenCascade_FIND_REQUIRED)
	message(FATAL_ERROR "Looking for OpenCascade... - Not found")
    endif (OpenCascade_FIND_REQUIRED)
endif (OpenCascade_FOUND)
