
# Defines API_SRCS

SET( API_SRCS 
	source/api/cmiss_command_data.c
	source/api/cmiss_field.c
	source/api/cmiss_field_composite.c
	source/api/cmiss_core.c
	source/api/cmiss_element.c
	source/api/cmiss_node.c
	source/api/cmiss_region.c
	source/api/cmiss_time.c
	source/api/cmiss_time_keeper.c
	source/api/cmiss_time_sequence.c )
SET( API_HDRS
	source/api/cmiss_command_data.h
	source/api/cmiss_core.h
	source/api/cmiss_element.h
	source/api/cmiss_fdio.h
	source/api/cmiss_field.h
	source/api/cmiss_field_alias.h
	source/api/cmiss_field_arithmetic_operators.h
	source/api/cmiss_field_composite.h
	source/api/cmiss_field_conditional.h
	source/api/cmiss_field_image.h
	source/api/cmiss_field_image_processing.h
	source/api/cmiss_field_logical_operators.h
	source/api/cmiss_field_trigonometry.h
	source/api/cmiss_graphics_window.h
	source/api/cmiss_idle.h
	source/api/cmiss_node.h
	source/api/cmiss_region.h
	source/api/cmiss_time.h
	source/api/cmiss_timer.h
	source/api/cmiss_time_keeper.h
	source/api/cmiss_time_sequence.h
	source/api/cmiss_value.h
	source/api/cmiss_value_derivative_matrix.h
	source/api/cmiss_value_element_xi.h
	source/api/cmiss_value_fe_value.h
	source/api/cmiss_value_matrix.h )
	
IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( API_SRCS ${API_SRCS}
		source/api/cmiss_scene_viewer.cpp
		source/api/cmiss_texture.c )
	SET( API_HDRS ${API_HDRS}
		source/api/cmiss_scene_viewer.h
		source/api/cmiss_scene_viewer_private.h
		source/api/cmiss_texture.h )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

