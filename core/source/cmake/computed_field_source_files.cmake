
# Defines COMPUTED_FIELD_CORE_SRCS, COMPUTED_FIELD_GRAPHICS_SRCS, COMPUTED_FIELD_SRCS (contains previous two)

# OpenCMISS-Zinc Library
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
	source/computed_field/computed_field_matrix_operators.cpp
	source/computed_field/computed_field_mesh_operators.cpp
	source/computed_field/computed_field_nodeset_operators.cpp
	source/computed_field/computed_field_scene_viewer_projection.cpp
	source/computed_field/computed_field_subobject_group.cpp
	source/computed_field/computed_field_string_constant.cpp
	source/computed_field/computed_field_trigonometry.cpp
	source/computed_field/computed_field_vector_operators.cpp
	source/computed_field/computed_field_wrappers.cpp
	source/computed_field/differential_operator.cpp
	source/computed_field/field_cache.cpp
	source/computed_field/field_module.cpp
	source/computed_field/fieldassignmentprivate.cpp
	source/computed_field/fieldsmoothingprivate.cpp
	source/computed_field/computed_field_find_xi.cpp
	source/computed_field/computed_field_finite_element.cpp
	source/computed_field/computed_field_set.cpp
	source/computed_field/field_location.cpp
	source/description_io/field_json_io.cpp
	source/description_io/fieldmodule_json_io.cpp )
SET( COMPUTED_FIELD_CORE_HDRS
	source/computed_field/computed_field.h
	source/computed_field/computed_field_arithmetic_operators.h
	source/computed_field/computed_field_composite.h
	source/computed_field/computed_field_conditional.h
	source/computed_field/computed_field_coordinate.h
	source/computed_field/computed_field_format_output.h
	source/computed_field/computed_field_function.h
	source/computed_field/computed_field_group.hpp
	source/computed_field/computed_field_group_base.hpp
	source/computed_field/computed_field_logical_operators.h
	source/computed_field/computed_field_matrix_operators.hpp
	source/computed_field/computed_field_mesh_operators.hpp
	source/computed_field/computed_field_nodeset_operators.hpp
	source/computed_field/computed_field_scene_viewer_projection.h
	source/computed_field/computed_field_subobject_group.hpp
	source/computed_field/computed_field_string_constant.h
	source/computed_field/computed_field_trigonometry.h
	source/computed_field/computed_field_vector_operators.hpp
	source/computed_field/computed_field_wrappers.h
	source/computed_field/differential_operator.hpp
	source/computed_field/field_cache.hpp
	source/computed_field/field_module.hpp
	source/computed_field/fieldassignmentprivate.hpp
	source/computed_field/fieldsmoothingprivate.hpp
	source/computed_field/computed_field_find_xi.h
	source/computed_field/computed_field_finite_element.h
	source/computed_field/computed_field_set.h
	source/computed_field/field_location.hpp
	source/description_io/field_json_io.hpp
	source/description_io/fieldmodule_json_io.hpp )

SET( COMPUTED_FIELD_GRAPHICS_SRCS
	source/computed_field/computed_field_find_xi_graphics.cpp )
SET( COMPUTED_FIELD_GRAPHICS_HDRS
	source/computed_field/computed_field_find_xi_graphics.h
	source/computed_field/computed_field_find_xi_private.hpp )

SET( COMPUTED_FIELD_SRCS
	${COMPUTED_FIELD_CORE_SRCS}
	${COMPUTED_FIELD_GRAPHICS_SRCS}
	source/minimise/minimise.cpp
	source/minimise/cmiss_optimisation_private.cpp
	source/minimise/optimisation.cpp
	source/computed_field/computed_field_alias.cpp
	source/computed_field/computed_field_compose.cpp
	source/computed_field/computed_field_deformation.cpp
	source/computed_field/computed_field_image.cpp
	source/computed_field/computed_field_integration.cpp
	source/computed_field/computed_field_lookup.cpp
	source/computed_field/computed_field_time.cpp
	source/computed_field/computed_field_update.cpp
	source/computed_field/computed_field_value_index_ranges.cpp
	source/stream/field_image_stream.cpp )

SET( COMPUTED_FIELD_HDRS
	${COMPUTED_FIELD_CORE_HDRS}
	${COMPUTED_FIELD_GRAPHICS_HDRS}
	source/minimise/minimise.h
	source/minimise/cmiss_optimisation_private.hpp
	source/minimise/optimisation.hpp
	source/computed_field/computed_field_alias.h
	source/computed_field/computed_field_compose.h
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
	source/stream/field_image_stream.hpp )

IF( ZINC_USE_ITK )
	SET( COMPUTED_FIELD_SRCS ${COMPUTED_FIELD_SRCS} source/computed_field/computed_field_derivatives.cpp )
	SET( COMPUTED_FIELD_HDRS ${COMPUTED_FIELD_HDRS} source/computed_field/computed_field_derivatives.h )
ENDIF( ZINC_USE_ITK )


