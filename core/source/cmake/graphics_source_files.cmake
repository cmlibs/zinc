	
# Defines GRAPHICS_SRCS

SET( GRAPHICS_SRCS
	source/graphics/auxiliary_graphics_types.c
	source/graphics/graphic.cpp
	source/graphics/graphics_coordinate_system.cpp
	source/graphics/graphics_module.cpp
	source/graphics/rendition.cpp
	source/graphics/colour.c
	source/graphics/complex.c
	source/graphics/decimate_voltex.cpp
	source/graphics/defined_graphics_objects.c
	source/graphics/element_point_ranges.c
	source/graphics/environment_map.c
	source/graphics/glyph.cpp
	source/graphics/import_graphics_object.c
	source/graphics/iso_field_calculation.c
	source/graphics/laguer.c
	source/graphics/material.cpp
	source/graphics/mcubes.c
	source/graphics/order_independent_transparency.cpp
	source/graphics/render_to_finite_elements.cpp
	source/graphics/render_stl.cpp
	source/graphics/render_vrml.cpp
	source/graphics/render_wavefront.cpp
	source/graphics/render_triangularisation.cpp
	source/graphics/quaternion.cpp
	source/graphics/selected_graphic.c
	source/graphics/selection.cpp
	source/graphics/spectrum.cpp
	source/graphics/spectrum_settings.cpp
	source/graphics/tessellation.cpp
	source/graphics/texture.cpp
	source/graphics/texture_line.c
	source/graphics/triangle_mesh.cpp
	source/graphics/userdef_objects.c
	source/graphics/volume_texture.c )
SET( GRAPHICS_HDRS
	source/graphics/auxiliary_graphics_types.h
	source/graphics/graphic.h
	source/graphics/graphics_coordinate_system.hpp
	source/graphics/graphics_module.h
	source/graphics/rendition.h
	source/graphics/rendition.hpp
	source/graphics/colour.h
	source/graphics/complex.h
	source/graphics/decimate_voltex.h
	source/graphics/defined_graphics_objects.h
	source/graphics/element_point_ranges.h
	source/graphics/environment_map.h
	source/graphics/glyph.h
	source/graphics/graphics_filter.hpp
	source/graphics/graphics_object.h
	source/graphics/graphics_object.hpp
	source/graphics/graphics_object_private.hpp
	source/graphics/graphics_object_highlight.hpp
	source/graphics/image.h
	source/graphics/import_graphics_object.h
	source/graphics/iso_field_calculation.h
	source/graphics/laguer.h
	source/graphics/material.h
	source/graphics/material.hpp
	source/graphics/mcubes.h
	source/graphics/order_independent_transparency.h
	source/graphics/quaternion.hpp
	source/graphics/render_alias.h
	source/graphics/render_binary_wavefront.h
	source/graphics/render_stl.h
	source/graphics/render_vrml.h
	source/graphics/render_wavefront.h
	source/graphics/render_to_finite_elements.h
	source/graphics/render_triangularisation.hpp
	source/graphics/selected_graphic.h
	source/graphics/selection.hpp
	source/graphics/spectrum.h
	source/graphics/spectrum.hpp
	source/graphics/spectrum_settings.h
	source/graphics/tessellation.hpp
	source/graphics/texture.h
	source/graphics/texture.hpp
	source/graphics/texture_line.h
	source/graphics/triangle_mesh.hpp
	source/graphics/userdef_objects.h
	source/graphics/volume_texture.h )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( GRAPHICS_SRCS ${GRAPHICS_SRCS}
		source/graphics/font.cpp
		source/graphics/graphics_filter.cpp
		source/graphics/graphics_library.c
		source/graphics/graphics_object.cpp
		source/graphics/light.c
		source/graphics/light_model.cpp
		source/graphics/render.cpp
		source/graphics/render_gl.cpp
		source/graphics/scene.cpp
		source/graphics/scene_viewer.cpp
		source/graphics/tile_graphics_objects.cpp )
	SET( GRAPHICS_HDRS ${GRAPHICS_HDRS}
		source/graphics/font.h
		source/graphics/graphics_library.h
		source/graphics/light.h
		source/graphics/light_model.h
		source/graphics/render.hpp
		source/graphics/render_gl.h
		source/graphics/scene.h
		source/graphics/scene.hpp
		source/graphics/scene_viewer.h
		source/graphics/scene_viewer.hpp
		source/graphics/tile_graphics_objects.h )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
