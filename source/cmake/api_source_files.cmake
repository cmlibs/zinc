
# Defines API_SRCS

SET( API_HDRS
	source/api/zn_context.h
	source/api/zn_core.h
	source/api/zn_element.h
	source/api/zn_fdio.h
	source/api/zn_field.h
	source/api/zn_field_alias.h
	source/api/zn_field_arithmetic_operators.h
	source/api/zn_field_composite.h
	source/api/zn_field_conditional.h
	source/api/zn_field_finite_element.h
	source/api/zn_field_group.h
	source/api/zn_field_image.h
	source/api/zn_field_image_processing.h
	source/api/zn_field_logical_operators.h
	source/api/zn_field_matrix_operators.h
	source/api/zn_field_module.h
	source/api/zn_field_nodeset_operators.h
	source/api/zn_field_scene_viewer_projection.h
	source/api/zn_field_subobject_group.h
	source/api/zn_field_trigonometry.h
	source/api/zn_field_vector_operators.h
	source/api/zn_graphic.h
	source/api/zn_graphics_filter.h
	source/api/zn_graphics_material.h
	source/api/zn_idle.h
	source/api/zn_interactive_tool.h
	source/api/zn_optimisation.h
	source/api/zn_node.h
	source/api/zn_region.h
	source/api/zn_rendition.h
	source/api/zn_scene.h
	source/api/zn_scene_viewer.h
	source/api/zn_selection.h
	source/api/zn_spectrum.h
	source/api/zn_stream.h
	source/api/zn_tessellation.h
	source/api/zn_time.h
	source/api/zn_time_keeper.h
	source/api/zn_time_sequence.h 
	source/api/types/zn_c_inline.h
	source/api/types/zn_context_id.h
	source/api/types/zn_element_id.h
	source/api/types/zn_field_alias_id.h
	source/api/types/zn_field_finite_element_id.h
	source/api/types/zn_field_group_id.h
	source/api/types/zn_field_id.h
	source/api/types/zn_field_image_id.h
	source/api/types/zn_field_image_processing_id.h
	source/api/types/zn_field_module_id.h
	source/api/types/zn_field_subobject_group_id.h
	source/api/types/zn_graphic_id.h
	source/api/types/zn_graphics_coordinate_system.h
	source/api/types/zn_graphics_filter_id.h
	source/api/types/zn_graphics_material_id.h
	source/api/types/zn_graphics_module_id.h
	source/api/types/zn_graphics_render_type.h
	source/api/types/zn_interactive_tool_id.h
	source/api/types/zn_node_id.h
	source/api/types/zn_optimisation_id.h
	source/api/types/zn_region_id.h
	source/api/types/zn_rendition_id.h
	source/api/types/zn_scene_id.h
	source/api/types/zn_scene_viewer_id.h
	source/api/types/zn_selection_id.h
	source/api/types/zn_spectrum_id.h
	source/api/types/zn_stream_id.h
	source/api/types/zn_tessellation_id.h
	source/api/types/zn_time_id.h
	source/api/types/zn_time_keeper_id.h
	source/api/types/zn_time_sequence_id.h )
	
IF( USE_OPENCASCADE )
	SET( API_HDRS ${API_HDRS}
		source/api/zn_field_cad.h 
		source/api/types/zn_field_cad_id.h )
ENDIF( USE_OPENCASCADE )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( API_HDRS ${API_HDRS}
		source/api/zn_scene_viewer.h )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

