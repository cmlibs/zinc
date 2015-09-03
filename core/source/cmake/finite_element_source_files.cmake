
# Defines FINITE_ELEMENT_CORE_SRCS, FINITE_ELEMENT_GRAPHICS_SRCS, 
# FINITE_ELEMENT_ADDITIONAL_SRCS, FINITE_ELEMENT_SRCS 
# (group definition which includes the previous three)

# OpenCMISS-Zinc Library
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SET( FINITE_ELEMENT_CORE_SRCS
	source/finite_element/export_finite_element.cpp
	source/finite_element/finite_element.cpp
	source/finite_element/finite_element_basis.cpp
	source/finite_element/finite_element_discretization.cpp
	source/finite_element/finite_element_helper.cpp
	source/finite_element/finite_element_mesh.cpp
	source/finite_element/finite_element_region.cpp
	source/finite_element/finite_element_time.cpp
	source/finite_element/import_finite_element.cpp )
SET( FINITE_ELEMENT_CORE_HDRS
	source/finite_element/export_finite_element.h
	source/finite_element/finite_element_discretization.h
	source/finite_element/finite_element_helper.h
	source/finite_element/finite_element_mesh.hpp
	source/finite_element/finite_element_private.h
	source/finite_element/finite_element_region.h
	source/finite_element/finite_element_region_private.h
	source/finite_element/finite_element.h
	source/finite_element/finite_element_basis.h
	source/finite_element/finite_element_time.h
	source/finite_element/import_finite_element.h )

SET( FINITE_ELEMENT_GRAPHICS_SRCS
	source/finite_element/finite_element_to_graphics_object.cpp
	source/finite_element/finite_element_to_iso_lines.cpp
	source/finite_element/finite_element_to_iso_surfaces.cpp
	source/finite_element/finite_element_to_streamlines.cpp )
SET( FINITE_ELEMENT_GRAPHICS_HDRS
	source/finite_element/finite_element_to_graphics_object.h
	source/finite_element/finite_element_to_iso_lines.h
	source/finite_element/finite_element_to_iso_surfaces.h
	source/finite_element/finite_element_to_streamlines.h )

SET( FINITE_ELEMENT_ADDITIONAL_SRCS
	source/finite_element/export_cm_files.cpp
	source/finite_element/finite_element_adjacent_elements.cpp
	source/finite_element/finite_element_conversion.cpp
	source/finite_element/finite_element_to_iges.cpp
	source/finite_element/snake.cpp )
SET( FINITE_ELEMENT_ADDITIONAL_HDRS
	source/finite_element/export_cm_files.h
	source/finite_element/finite_element_adjacent_elements.h
	source/finite_element/finite_element_conversion.h
	source/finite_element/finite_element_to_iges.h
	source/finite_element/snake.h )

IF( USE_NETGEN )
	SET( FINITE_ELEMENT_CORE_SRCS
		${FINITE_ELEMENT_CORE_SRCS}
		source/finite_element/generate_mesh_netgen.cpp )
	SET( FINITE_ELEMENT_CORE_HDRS
		${FINITE_ELEMENT_CORE_HDRS}
		source/finite_element/generate_mesh_netgen.h )
ENDIF( USE_NETGEN )

SET( FINITE_ELEMENT_SRCS
	${FINITE_ELEMENT_CORE_SRCS}
	${FINITE_ELEMENT_GRAPHICS_SRCS}
	${FINITE_ELEMENT_ADDITIONAL_SRCS} )
SET(  FINITE_ELEMENT_HDRS
	${FINITE_ELEMENT_CORE_HDRS}
	${FINITE_ELEMENT_GRAPHICS_HDRS}
	${FINITE_ELEMENT_ADDITIONAL_HDRS} )

