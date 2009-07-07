
# Defines COMPUTED_FIELD_CORE_SRCS, COMPUTED_FIELD_GRAPHICS_SRCS, COMPUTED_FIELD_SRCS (contains previous two)

SET( COMPUTED_FIELD_CORE_SRCS
  computed_field/computed_field.cpp
	computed_field/computed_field_arithmetic_operators.cpp
	computed_field/computed_field_composite.cpp
	computed_field/computed_field_conditional.cpp
	computed_field/computed_field_find_xi.cpp
	computed_field/computed_field_finite_element.cpp
	computed_field/computed_field_set.cpp
	computed_field/field_location.cpp )

SET( COMPUTED_FIELD_GRAPHICS_SRCS computed_field/computed_field_find_xi_graphics.cpp )
SET( COMPUTED_FIELD_SRCS
	${COMPUTED_FIELD_CORE_SRCS}
	${COMPUTED_FIELD_GRAPHICS_SRCS}
	minimise/minimise.cpp
	computed_field/computed_field_alias.cpp
	computed_field/computed_field_compose.cpp
	computed_field/computed_field_curve.cpp
	computed_field/computed_field_coordinate.cpp
	computed_field/computed_field_deformation.cpp
	computed_field/computed_field_fibres.cpp
	computed_field/computed_field_function.cpp
	computed_field/computed_field_image.cpp
	computed_field/computed_field_integration.cpp
	computed_field/computed_field_logical_operators.cpp
	computed_field/computed_field_lookup.cpp
	computed_field/computed_field_matrix_operations.cpp
	computed_field/computed_field_region_operations.cpp
	computed_field/computed_field_string_constant.cpp
	computed_field/computed_field_time.cpp
	computed_field/computed_field_trigonometry.cpp
	computed_field/computed_field_update.cpp
	computed_field/computed_field_value_index_ranges.cpp
	computed_field/computed_field_vector_operations.cpp
	computed_field/computed_field_wrappers.cpp )

IF( USE_ITK )
	SET( COMPUTED_FIELD_SRCS ${COMPUTED_FIELD_SRCS} computed_field/computed_field_derivatives.cpp )
ENDIF( USE_ITK )


