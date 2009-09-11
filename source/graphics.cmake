	
# Defines GRAPHICS_SRCS

SET( GRAPHICS_SRCS
	graphics/auxiliary_graphics_types.c
	graphics/colour.c
	graphics/complex.c
	graphics/decimate_voltex.cpp
	graphics/defined_graphics_objects.c
	graphics/element_group_settings.cpp
	graphics/element_point_ranges.c
	graphics/environment_map.c
	graphics/glyph.cpp
	graphics/import_graphics_object.c
	graphics/iso_field_calculation.c
	graphics/laguer.c
	graphics/material.cpp
	graphics/mcubes.c
	graphics/order_independent_transparency.cpp
	graphics/render_to_finite_elements.cpp
	graphics/renderstl.cpp
	graphics/rendervrml.cpp
	graphics/renderwavefront.cpp
	graphics/render_triangularisation.cpp
	graphics/quaternion.cpp
	graphics/selected_graphic.c
	graphics/spectrum.cpp
	graphics/spectrum_settings.c
	graphics/texture.cpp
	graphics/texture_line.c
	graphics/transform_tool.cpp
	graphics/triangle_mesh.cpp
	graphics/userdef_objects.c
	graphics/volume_texture.c )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( GRAPHICS_SRCS ${GRAPHICS_SRCS}
		graphics/font.cpp
		graphics/graphical_element.cpp
		graphics/graphics_library.c
		graphics/graphics_object.cpp
		graphics/light.c
		graphics/light_model.cpp
		graphics/render.cpp
		graphics/rendergl.cpp
		graphics/scene.cpp
		graphics/scene_viewer.cpp
		graphics/tile_graphics_objects.cpp )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	
IF( WX_USER_INTERFACE )
	SET( GRAPHICS_SRCS ${GRAPHICS_SRCS}
		graphics/scene_editor_wx.cpp
		graphics/spectrum_editor_wx.cpp
		graphics/spectrum_editor_dialog_wx.cpp )
ENDIF( WX_USER_INTERFACE )
