# OpenCMISS-Zinc Library
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

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

