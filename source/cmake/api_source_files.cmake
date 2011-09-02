
# Defines API_SRCS

SET( API_SRCS 
	source/api/cmiss_core.c
	source/api/cmiss_idle.c
	source/api/cmiss_region.c
	source/api/cmiss_scene_viewer.cpp
	source/api/cmiss_time_sequence.c )
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
	source/api/cmiss_field_ensemble.h
	source/api/cmiss_field_finite_element.h
	source/api/cmiss_field_group.h
	source/api/cmiss_field_image.h
	source/api/cmiss_field_image_processing.h
	source/api/cmiss_field_logical_operators.h
	source/api/cmiss_field_matrix_operators.h
	source/api/cmiss_field_module.h
	source/api/cmiss_field_nodeset_operators.h
	source/api/cmiss_field_parameters.h
	source/api/cmiss_field_scene_viewer_projection.h
	source/api/cmiss_field_subobject_group.h
	source/api/cmiss_field_trigonometry.h
	source/api/cmiss_field_vector_operators.h
	source/api/cmiss_graphic.h
	source/api/cmiss_graphics_filter.h
	source/api/cmiss_graphics_material.h
	source/api/cmiss_idle.h
	source/api/cmiss_interactive_tool.h
	source/api/cmiss_optimisation.h
	source/api/cmiss_node.h
	source/api/cmiss_region.h
	source/api/cmiss_rendition.h
	source/api/cmiss_scene.h
	source/api/cmiss_scene_viewer.h
	source/api/cmiss_selection.h
	source/api/cmiss_spectrum.h
	source/api/cmiss_stream.h
	source/api/cmiss_tessellation.h
	source/api/cmiss_time.h
	source/api/cmiss_time_keeper.h
	source/api/cmiss_time_sequence.h 
	source/api/types/cmiss_c_inline.h
	source/api/types/cmiss_context_id.h
	source/api/types/cmiss_element_id.h
	source/api/types/cmiss_field_alias_id.h
	source/api/types/cmiss_field_ensemble_id.h
	source/api/types/cmiss_field_finite_element_id.h
	source/api/types/cmiss_field_group_id.h
	source/api/types/cmiss_field_id.h
	source/api/types/cmiss_field_image_id.h
	source/api/types/cmiss_field_image_processing_id.h
	source/api/types/cmiss_field_module_id.h
	source/api/types/cmiss_field_parameters_id.h
	source/api/types/cmiss_field_subobject_group_id.h
	source/api/types/cmiss_graphic_id.h
	source/api/types/cmiss_graphics_coordinate_system.h
	source/api/types/cmiss_graphics_filter_id.h
	source/api/types/cmiss_graphics_material_id.h
	source/api/types/cmiss_graphics_module_id.h
	source/api/types/cmiss_graphics_render_type.h
	source/api/types/cmiss_interactive_tool_id.h
	source/api/types/cmiss_node_id.h
	source/api/types/cmiss_optimisation_id.h
	source/api/types/cmiss_region_id.h
	source/api/types/cmiss_rendition_id.h
	source/api/types/cmiss_scene_id.h
	source/api/types/cmiss_scene_viewer_id.h
	source/api/types/cmiss_selection_id.h
	source/api/types/cmiss_spectrum_id.h
	source/api/types/cmiss_stream_id.h
	source/api/types/cmiss_tessellation_id.h
	source/api/types/cmiss_time_id.h
	source/api/types/cmiss_time_keeper_id.h
	source/api/types/cmiss_time_sequence_id.h )
	
IF( USE_OPENCASCADE )
	SET( API_HDRS ${API_HDRS}
		source/api/cmiss_field_cad.h 
		source/api/types/cmiss_field_cad_id.h )
ENDIF( USE_OPENCASCADE )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( API_SRCS ${API_SRCS}
		source/api/cmiss_scene_viewer.cpp )
	SET( API_HDRS ${API_HDRS}
		source/api/cmiss_scene_viewer.h )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

