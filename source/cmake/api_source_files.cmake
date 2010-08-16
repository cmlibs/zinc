
# Defines API_SRCS

SET( API_SRCS 
	source/api/cmiss_context.c
	source/api/cmiss_element.c
	source/api/cmiss_graphics_window.c
	source/api/cmiss_field.c
	source/api/cmiss_graphic.c
	source/api/cmiss_graphics_window.c
	source/api/cmiss_idle.c
	source/api/cmiss_material.c
	source/api/cmiss_node.c
	source/api/cmiss_region.c
	source/api/cmiss_rendition.c
	source/api/cmiss_time.c
	source/api/cmiss_time_keeper.c
	source/api/cmiss_time_sequence.c
	source/api/cmiss_timer.c )
SET( API_HDRS
	source/api/cmiss_context.h
	source/api/cmiss_core.h
	source/api/cmiss_element.h
	source/api/cmiss_fdio.h
	source/api/cmiss_field.h
	source/api/cmiss_field_alias.h
	source/api/cmiss_field_arithmetic_operators.h
	source/api/cmiss_field_composite.h
	source/api/cmiss_field_conditional.h
	source/api/cmiss_field_group.h
	source/api/cmiss_field_image.h
	source/api/cmiss_field_image_processing.h
	source/api/cmiss_field_logical_operators.h
	source/api/cmiss_field_module.h
	source/api/cmiss_field_parameters.h
	source/api/cmiss_field_sub_group_template.h
	source/api/cmiss_field_trigonometry.h
	source/api/cmiss_graphic.h
	source/api/cmiss_graphics_window.h
	source/api/cmiss_idle.h
	source/api/cmiss_material.h
	source/api/cmiss_node.h
	source/api/cmiss_region.h
	source/api/cmiss_rendition.h
	source/api/cmiss_scene.h
	source/api/cmiss_scene_filter.h
	source/api/cmiss_time.h
	source/api/cmiss_time_keeper.h
	source/api/cmiss_time_sequence.h
	source/api/cmiss_timer.h )
	
IF( USE_OPENCASCADE )
	SET( API_HDRS ${API_HDRS}
		source/api/cmiss_field_cad.h )
ENDIF( USE_OPENCASCADE )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( API_SRCS ${API_SRCS}
		source/api/cmiss_scene_viewer.cpp
		source/api/cmiss_texture.c )
	SET( API_HDRS ${API_HDRS}
		source/api/cmiss_scene_viewer.h
		source/api/cmiss_scene_viewer_private.h
		source/api/cmiss_texture.h )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

