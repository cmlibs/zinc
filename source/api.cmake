
# Defines API_SRCS

SET( API_SRCS api/cmiss_command_data.c
	api/cmiss_field.c
	api/cmiss_core.c
	api/cmiss_element.c
	api/cmiss_node.c
	api/cmiss_region.c
	api/cmiss_time_sequence.c )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( API_SRCS ${API_SRCS} api/cmiss_scene_viewer.cpp api/cmiss_texture.c )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

