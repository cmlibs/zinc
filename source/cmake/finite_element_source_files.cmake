
# Defines FINITE_ELEMENT_CORE_SRCS, FINITE_ELEMENT_GRAPHICS_SRCS, 
# FINITE_ELEMENT_ADDITIONAL_SRCS, FINITE_ELEMENT_SRCS 
# (group definition which includes the previous three)

SET( FINITE_ELEMENT_CORE_SRCS
	source/finite_element/export_finite_element.cpp
	source/finite_element/finite_element.c
	source/finite_element/finite_element_discretization.cpp
	source/finite_element/finite_element_helper.cpp
	source/finite_element/finite_element_region.c
	source/finite_element/finite_element_time.c
	source/finite_element/import_finite_element.cpp )
SET( FINITE_ELEMENT_CORE_HDRS
	source/finite_element/export_finite_element.h
	source/finite_element/finite_element_discretization.h
	source/finite_element/finite_element_helper.h
	source/finite_element/finite_element_private.h
	source/finite_element/finite_element_region.h
	source/finite_element/finite_element_region_private.h
	source/finite_element/export_finite_element.h
	source/finite_element/finite_element.h
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
	source/finite_element/export_cm_files.c
	source/finite_element/finite_element_adjacent_elements.c
	source/finite_element/finite_element_conversion.cpp
	source/finite_element/finite_element_to_iges.c
	source/finite_element/read_fieldml.c
	source/finite_element/snake.c
	source/finite_element/write_fieldml.c )
SET( FINITE_ELEMENT_ADDITIONAL_HDRS
	source/finite_element/export_cm_files.h
	source/finite_element/finite_element_adjacent_elements.h
	source/finite_element/finite_element_conversion.h
	source/finite_element/finite_element_to_iges.h
	source/finite_element/read_fieldml.h
	source/finite_element/snake.h
	source/finite_element/write_fieldml.h )

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

