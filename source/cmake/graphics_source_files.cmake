	
# Defines GRAPHICS_SRCS

SET( GRAPHICS_SRCS
	source/graphics/auxiliary_graphics_types.c
	source/graphics/cmiss_graphic.cpp
	source/graphics/cmiss_rendition.cpp
	source/graphics/colour.c
	source/graphics/complex.c
	source/graphics/decimate_voltex.cpp
	source/graphics/defined_graphics_objects.c
	source/graphics/element_group_settings.cpp
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
	source/graphics/renderstl.cpp
	source/graphics/rendervrml.cpp
	source/graphics/renderwavefront.cpp
	source/graphics/render_triangularisation.cpp
	source/graphics/quaternion.cpp
	source/graphics/selected_graphic.c
	source/graphics/spectrum.cpp
	source/graphics/spectrum_settings.c
	source/graphics/texture.cpp
	source/graphics/texture_line.c
	source/graphics/transform_tool.cpp
	source/graphics/triangle_mesh.cpp
	source/graphics/userdef_objects.c
	source/graphics/volume_texture.c )
SET( GRAPHICS_HDRS
	source/graphics/auxiliary_graphics_types.h
	source/graphics/cmiss_graphic.h
	source/graphics/cmiss_rendition.hpp
	source/graphics/colour.h
	source/graphics/complex.h
	source/graphics/decimate_voltex.h
	source/graphics/defined_graphics_objects.h
	source/graphics/element_group_settings.h
	source/graphics/element_point_ranges.h
	source/graphics/environment_map.h
	source/graphics/glyph.h
	source/graphics/graphical_element.hpp
	source/graphics/graphical_element_editor.h
	source/graphics/graphics_object.h
	source/graphics/graphics_object.hpp
	source/graphics/graphics_object_private.hpp
	source/graphics/graphics_object_highlight.hpp
	source/graphics/graphics_window.h
	source/graphics/graphics_window_private.hpp
	source/graphics/image.h
	source/graphics/import_graphics_object.h
	source/graphics/iso_field_calculation.h
	source/graphics/laguer.h
	source/graphics/material.h
	source/graphics/material.hpp
	source/graphics/mcubes.h
	source/graphics/movie_graphics.h
	source/graphics/order_independent_transparency.h
	source/graphics/quaternion.hpp
	source/graphics/renderalias.h
	source/graphics/renderbinarywavefront.h
	source/graphics/renderstl.h
	source/graphics/rendervrml.h
	source/graphics/renderwavefront.h
	source/graphics/render_to_finite_elements.h
	source/graphics/render_triangularisation.hpp
	source/graphics/robo_window.h
	source/graphics/scene_editor.h
	source/graphics/scene_filters.hpp
	source/graphics/selected_graphic.h
	source/graphics/settings_editor.h
	source/graphics/spectrum.h
	source/graphics/spectrum.hpp
	source/graphics/spectrum_editor.h
	source/graphics/spectrum_editor_dialog.h
	source/graphics/spectrum_editor_settings.h
	source/graphics/spectrum_settings.h
	source/graphics/texture.h
	source/graphics/texture.hpp
	source/graphics/texturemap.h
	source/graphics/texture_line.h
	source/graphics/transform_tool.h
	source/graphics/triangle_mesh.hpp
	source/graphics/userdef_objects.h
	source/graphics/volume_texture.h
	source/graphics/volume_texture_editor_dialog.h )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( GRAPHICS_SRCS ${GRAPHICS_SRCS}
		source/graphics/font.cpp
		source/graphics/graphical_element.cpp
		source/graphics/graphics_library.c
		source/graphics/graphics_object.cpp
		source/graphics/light.c
		source/graphics/light_model.cpp
		source/graphics/render.cpp
		source/graphics/rendergl.cpp
		source/graphics/scene.cpp
		source/graphics/scene_viewer.cpp
		source/graphics/tile_graphics_objects.cpp )
	SET( GRAPHICS_HDRS ${GRAPHICS_HDRS}
		source/graphics/font.h
		source/graphics/graphical_element.h
		source/graphics/graphics_library.h
		source/graphics/light.h
		source/graphics/light_model.h
		source/graphics/render.hpp
		source/graphics/rendergl.hpp
		source/graphics/scene.h
		source/graphics/scene.hpp
		source/graphics/scene_viewer.h
		source/graphics/scene_viewer.hpp
		source/graphics/tile_graphics_objects.h )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	
IF( WX_USER_INTERFACE )
	SET( GRAPHICS_SRCS ${GRAPHICS_SRCS}
		source/graphics/region_tree_viewer_wx.cpp
		source/graphics/spectrum_editor_wx.cpp
		source/graphics/spectrum_editor_dialog_wx.cpp )
	SET( GRAPHICS_HDRS ${GRAPHICS_HDRS}
		source/graphics/region_tree_viewer_wx.h
		source/graphics/spectrum_editor_wx.h
		source/graphics/spectrum_editor_dialog_wx.h )
ENDIF( WX_USER_INTERFACE )
