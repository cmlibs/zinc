
# Defines COMPUTED_FIELD_CORE_SRCS, COMPUTED_FIELD_GRAPHICS_SRCS, COMPUTED_FIELD_SRCS (contains previous two)

SET( COMPUTED_FIELD_CORE_SRCS
	source/computed_field/computed_field.cpp
	source/computed_field/computed_field_arithmetic_operators.cpp
	source/computed_field/computed_field_composite.cpp
	source/computed_field/computed_field_conditional.cpp
	source/computed_field/computed_field_coordinate.cpp
	source/computed_field/computed_field_fibres.cpp
	source/computed_field/computed_field_format_output.cpp
	source/computed_field/computed_field_function.cpp
	source/computed_field/computed_field_group.cpp
	source/computed_field/computed_field_logical_operators.cpp
	source/computed_field/computed_field_matrix_operations.cpp
	source/computed_field/computed_field_region_operations.cpp
	source/computed_field/computed_field_subobject_group.cpp
	source/computed_field/computed_field_string_constant.cpp
	source/computed_field/computed_field_trigonometry.cpp
	source/computed_field/computed_field_vector_operations.cpp
	source/computed_field/computed_field_wrappers.cpp 
	source/computed_field/field_cache.cpp
	source/computed_field/field_ensemble.cpp
	source/computed_field/field_module.cpp
	source/computed_field/field_parameters.cpp
	source/computed_field/computed_field_find_xi.cpp
	source/computed_field/computed_field_finite_element.cpp
	source/computed_field/computed_field_set.cpp
	source/computed_field/field_location.cpp )
SET( COMPUTED_FIELD_CORE_HDRS
	source/computed_field/computed_field.h
	source/computed_field/computed_field_arithmetic_operators.h
	source/computed_field/computed_field_composite.h
	source/computed_field/computed_field_conditional.h
	source/computed_field/computed_field_coordinate.h
	source/computed_field/computed_field_fibres.h
	source/computed_field/computed_field_format_output.h
	source/computed_field/computed_field_function.h
	source/computed_field/computed_field_group.h
	source/computed_field/computed_field_group_base.hpp
	source/computed_field/computed_field_logical_operators.h
	source/computed_field/computed_field_matrix_operations.h
	source/computed_field/computed_field_region_operations.h
	source/computed_field/computed_field_subobject_group.hpp
	source/computed_field/computed_field_string_constant.h
	source/computed_field/computed_field_trigonometry.h
	source/computed_field/computed_field_vector_operations.h
	source/computed_field/computed_field_wrappers.h
	source/computed_field/field_cache.hpp
	source/computed_field/field_ensemble.hpp
	source/computed_field/field_module.hpp
	source/computed_field/computed_field_find_xi.h
	source/computed_field/computed_field_finite_element.h
	source/computed_field/computed_field_set.h
	source/computed_field/field_location.hpp )

SET( COMPUTED_FIELD_GRAPHICS_SRCS source/computed_field/computed_field_find_xi_graphics.cpp )
SET( COMPUTED_FIELD_GRAPHICS_HDRS
	source/computed_field/computed_field_find_xi_graphics.h
	source/computed_field/computed_field_find_xi_private.hpp )

SET( COMPUTED_FIELD_SRCS
	${COMPUTED_FIELD_CORE_SRCS}
	${COMPUTED_FIELD_GRAPHICS_SRCS}
	source/minimise/minimise.cpp
	source/minimise/cmiss_optimisation_private.cpp
	source/minimise/optimisation.cpp
	source/computed_field/field_module_optimisation_private.cpp
	source/computed_field/computed_field_alias.cpp
	source/computed_field/computed_field_compose.cpp
	source/computed_field/computed_field_curve.cpp
	source/computed_field/computed_field_deformation.cpp
	source/computed_field/computed_field_image.cpp
	source/computed_field/computed_field_integration.cpp
	source/computed_field/computed_field_lookup.cpp
	source/computed_field/computed_field_time.cpp
	source/computed_field/computed_field_update.cpp
	source/computed_field/computed_field_value_index_ranges.cpp
	source/stream/cmiss_field_image_stream.cpp )
	
SET( COMPUTED_FIELD_HDRS
	${COMPUTED_FIELD_CORE_HDRS}
	${COMPUTED_FIELD_GRAPHICS_HDRS}
	source/minimise/minimise.h
	source/minimise/cmiss_optimisation_private.h
	source/minimise/optimisation.hpp
	source/computed_field/computed_field_alias.h
	source/computed_field/computed_field_compose.h
	source/computed_field/computed_field_curve.h
	source/computed_field/computed_field_deformation.h
	source/computed_field/computed_field_external.h
	source/computed_field/computed_field_image.h
	source/computed_field/computed_field_image_processing.h
	source/computed_field/computed_field_integration.h
	source/computed_field/computed_field_lookup.h
	source/computed_field/computed_field_private.hpp
	source/computed_field/computed_field_time.h
	source/computed_field/computed_field_update.h
	source/computed_field/computed_field_value_index_ranges.h
	source/computed_field/computed_field_window_projection.h
	source/stream/cmiss_field_image_stream.hpp )

IF( USE_ITK )
	SET( COMPUTED_FIELD_SRCS ${COMPUTED_FIELD_SRCS} source/computed_field/computed_field_derivatives.cpp )
	SET( COMPUTED_FIELD_HDRS ${COMPUTED_FIELD_HDRS} source/computed_field/computed_field_derivatives.h )
ENDIF( USE_ITK )


