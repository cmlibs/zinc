# Zinc Library
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

IF( ZINC_USE_STATIC )
    IF( ZINC_FIND_REQUIRED )
        MESSAGE( FATAL_ERROR "No static build of Zinc library available." )
    ELSE()
        MESSAGE( WARNING "No static build of Zinc library available, falling back to Zinc shared library." )
    ENDIF()
ENDIF()
SET( ZINC_LIBRARY @ZINC_SHARED_TARGET@ CACHE STRING "The shared Zinc library target" )
ADD_DEFINITIONS( @ZINC_SHARED_DEFINITIONS@ )

