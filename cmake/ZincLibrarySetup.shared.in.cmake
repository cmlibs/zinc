
IF( ZINC_USE_STATIC )
    IF( ZINC_FIND_REQUIRED )
        MESSAGE( FATAL_ERROR "No static build of Zinc library available." )
    ELSE()
        MESSAGE( WARNING "No static build of Zinc library available, falling back to Zinc shared library." )
    ENDIF()
ENDIF()
SET( ZINC_LIBRARY @ZINC_SHARED_TARGET@ CACHE STRING "The shared Zinc library target" )
ADD_DEFINITIONS( @ZINC_SHARED_DEFINITIONS@ )

