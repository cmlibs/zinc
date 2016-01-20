
# Defines GRAPHICS_SRCS

# OpenCMISS-Zinc Library
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SET( GRAPHICS_SRCS
	source/description_io/graphics_json_export.cpp
	source/description_io/graphics_json_import.cpp
	source/description_io/graphics_json_io.cpp
	source/description_io/scene_json_export.cpp
	source/description_io/scene_json_import.cpp
	source/description_io/sceneviewer_json_io.cpp
	source/description_io/spectrum_json_io.cpp
	source/description_io/tessellation_json_io.cpp
	source/graphics/auxiliary_graphics_types.cpp
	source/graphics/graphics.cpp
	source/graphics/graphics_module.cpp
	source/graphics/scene.cpp
	source/stream/scene_stream.cpp
	source/graphics/colour.cpp
	source/graphics/complex.cpp
	source/graphics/element_point_ranges.cpp
	source/graphics/environment_map.cpp
	source/graphics/glyph.cpp
	source/graphics/glyph_axes.cpp
	source/graphics/glyph_circular.cpp
	source/graphics/glyph_colour_bar.cpp
	source/graphics/graphics_vertex_array.cpp
	source/graphics/import_graphics_object.cpp
	source/graphics/iso_field_calculation.cpp
	source/graphics/laguer.cpp
	source/graphics/material.cpp
	source/graphics/mcubes.cpp
	source/graphics/order_independent_transparency.cpp
	source/graphics/render_to_finite_elements.cpp
	source/graphics/render_stl.cpp
	source/graphics/render_vrml.cpp
	source/graphics/render_wavefront.cpp
	source/graphics/render_triangularisation.cpp
	source/graphics/quaternion.cpp
	source/graphics/scene_coordinate_system.cpp
	source/graphics/scene_viewer.cpp
	source/graphics/selection.cpp
	source/graphics/spectrum.cpp
	source/graphics/spectrum_component.cpp
	source/graphics/tessellation.cpp
	source/graphics/texture.cpp
	source/graphics/texture_line.cpp
	source/graphics/threejs_export.cpp
	source/graphics/triangle_mesh.cpp
	source/graphics/volume_texture.cpp
	source/graphics/webgl_export.cpp )
SET( GRAPHICS_HDRS
	source/description_io/graphics_json_export.hpp
	source/description_io/graphics_json_import.hpp
	source/description_io/graphics_json_io.hpp
	source/description_io/scene_json_export.hpp
	source/description_io/scene_json_import.hpp
	source/description_io/sceneviewer_json_io.hpp
	source/description_io/spectrum_json_io.hpp
	source/description_io/tessellation_json_io.hpp
	source/graphics/auxiliary_graphics_types.h
	source/graphics/graphics.h
	source/graphics/graphics_module.h
	source/graphics/scene.h
	source/graphics/scene.hpp
	source/stream/scene_stream.hpp
	source/graphics/colour.h
	source/graphics/complex.h
	source/graphics/element_point_ranges.h
	source/graphics/environment_map.h
	source/graphics/glyph.hpp
	source/graphics/glyph_axes.hpp
	source/graphics/glyph_circular.hpp
	source/graphics/glyph_colour_bar.hpp
	source/graphics/graphics_object.h
	source/graphics/graphics_object.hpp
	source/graphics/graphics_object_private.hpp
	source/graphics/graphics_object_highlight.hpp
	source/graphics/graphics_vertex_array.hpp
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
	source/graphics/scene_coordinate_system.hpp
	source/graphics/scenefilter.hpp
	source/graphics/scene_viewer.h
	source/graphics/selection.hpp
	source/graphics/spectrum.h
	source/graphics/spectrum.hpp
	source/graphics/spectrum_component.h
	source/graphics/tessellation.hpp
	source/graphics/texture.h
	source/graphics/texture.hpp
	source/graphics/texture_line.h
	source/graphics/threejs_export.hpp
	source/graphics/triangle_mesh.hpp
	source/graphics/volume_texture.h
	source/graphics/webgl_export.hpp )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( GRAPHICS_SRCS ${GRAPHICS_SRCS}
		source/graphics/font.cpp
		source/graphics/graphics_library.cpp
		source/graphics/graphics_object.cpp
		source/graphics/light.cpp
		source/graphics/render.cpp
		source/graphics/render_gl.cpp
		source/graphics/scenefilter.cpp
		source/graphics/scene_picker.cpp
		source/graphics/tile_graphics_objects.cpp )
	SET( GRAPHICS_HDRS ${GRAPHICS_HDRS}
		source/graphics/font.h
		source/graphics/graphics_library.h
		source/graphics/light.hpp
		source/graphics/render.hpp
		source/graphics/render_gl.h
		source/graphics/scene_picker.hpp
		source/graphics/tile_graphics_objects.h )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
