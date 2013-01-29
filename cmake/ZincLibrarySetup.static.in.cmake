
IF( NOT ZINC_USE_STATIC )
    IF( ZINC_FIND_REQUIRED )
        MESSAGE( FATAL_ERROR "No shared build of Zinc library available." )
    ELSE()
        MESSAGE( WARNING "No shared build of Zinc library available, falling back to Zinc static library." )
    ENDIF()
ENDIF()

@DEPENDENT_CONFIGS@

SET( ZINC_LIBRARY @ZINC_STATIC_TARGET@ CACHE STRING "The static Zinc library target" )
ADD_DEFINITIONS( @ZINC_STATIC_DEFINITIONS@ )

