/*******************************************************************************
FILE : computed_field_update.c

LAST MODIFIED : 24 August 2006

DESCRIPTION :
Functions for updating values of one computed field from those of another.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "zinc/fieldcache.h"
#include "zinc/fieldmodule.h"
#include "zinc/status.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_update.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_discretization.h"
#include "general/debug.h"
#include "general/message.h"
#include "mesh/cmiss_element_private.hpp"
#include "mesh/cmiss_node_private.hpp"

int cmzn_nodeset_assign_field_from_source(
	cmzn_nodeset_id nodeset, cmzn_field_id destination_field,
	cmzn_field_id source_field, cmzn_field_id conditional_field,
	FE_value time)
{
	int return_code = 1;
	if (nodeset && destination_field && source_field)
	{
		const int number_of_components =
			Computed_field_get_number_of_components(destination_field);
		cmzn_field_value_type value_type = cmzn_field_get_value_type(destination_field);
		// can always evaluate to a string value
		if ((value_type == CMZN_FIELD_VALUE_TYPE_STRING) ||
			((Computed_field_get_number_of_components(source_field) == number_of_components) &&
				(cmzn_field_get_value_type(source_field) == value_type)))
		{
			cmzn_fieldmodule_id field_module = cmzn_field_get_fieldmodule(destination_field);
			cmzn_fieldmodule_begin_change(field_module);
			cmzn_fieldcache_id field_cache = cmzn_fieldmodule_create_fieldcache(field_module);
			FE_value *values = new FE_value[number_of_components];
			// all fields evaluated at same time so set once
			cmzn_fieldcache_set_time(field_cache, time);
			cmzn_nodeiterator_id iterator = cmzn_nodeset_create_nodeiterator(nodeset);
			cmzn_node_id node = 0;
			int selected_count = 0;
			int success_count = 0;
			while (return_code && (0 != (node = cmzn_nodeiterator_next(iterator))))
			{
				cmzn_fieldcache_set_node(field_cache, node);
				if ((!conditional_field) || cmzn_field_evaluate_boolean(conditional_field, field_cache))
				{
					if ((CMZN_OK == cmzn_field_is_defined_at_location(destination_field, field_cache)))
					{
						switch (value_type)
						{
						case CMZN_FIELD_VALUE_TYPE_MESH_LOCATION:
							{
								FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
								cmzn_element_id element = cmzn_field_evaluate_mesh_location(
									source_field, field_cache, MAXIMUM_ELEMENT_XI_DIMENSIONS, xi);
								if (element)
								{
									if ((CMZN_OK == cmzn_field_assign_mesh_location(destination_field, field_cache,
										element, MAXIMUM_ELEMENT_XI_DIMENSIONS, xi)))
									{
										++success_count;
									}
									cmzn_element_destroy(&element);
								}
							} break;
						case CMZN_FIELD_VALUE_TYPE_REAL:
							{
								if ((CMZN_OK == cmzn_field_evaluate_real(source_field, field_cache, number_of_components, values)) &&
									(CMZN_OK == cmzn_field_assign_real(destination_field, field_cache, number_of_components, values)))
								{
									++success_count;
								}
							} break;
						case CMZN_FIELD_VALUE_TYPE_STRING:
							{
								char *string_value = cmzn_field_evaluate_string(source_field, field_cache);
								if (string_value)
								{
									if ((CMZN_OK == cmzn_field_assign_string(destination_field, field_cache, string_value)))
									{
										++success_count;
									}
									DEALLOCATE(string_value);
								}
							} break;
						default:
							{
								display_message(ERROR_MESSAGE,
									"cmzn_nodeset_assign_field_from_source.  Unsupported value type.");
								return_code = 0;
							} break;
						}
					}
					++selected_count;
				}
				cmzn_node_destroy(&node);
			}
			cmzn_nodeiterator_destroy(&iterator);
			if (success_count != selected_count)
			{
				display_message(WARNING_MESSAGE,
					"cmzn_nodeset_assign_field_from_source.  "
					"Only able to set values for %d nodes out of %d\n"
					"  Either source field isn't defined at node "
					"or destination field could not be set.",
					success_count, selected_count);
			}
			delete[] values;
			cmzn_fieldcache_destroy(&field_cache);
			cmzn_fieldmodule_end_change(field_module);
			cmzn_fieldmodule_destroy(&field_module);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmzn_nodeset_assign_field_from_source.  "
				"Value type and number of components in source and destination fields must match.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_nodeset_assign_field_from_source.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

struct cmzn_element_assign_grid_field_from_source_data
{
	cmzn_fieldcache_id field_cache;
	int selected_count, success_count, xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct Computed_field *source_field;
	struct Computed_field *destination_field;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct Computed_field *group_field;
};

int cmzn_element_assign_grid_field_from_source_sub(
	cmzn_element_id element, cmzn_element_assign_grid_field_from_source_data *data)
{
	FE_value *values, xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int can_select_individual_points, destination_field_is_grid_based,
		element_selected, grid_point_number, i, maximum_element_point_number,
		number_of_ranges, return_code, start, stop;
	struct Element_point_ranges *element_point_ranges;
	struct Element_point_ranges_identifier element_point_ranges_identifier;
	struct Multi_range *selected_ranges;

	ENTER(cmzn_element_assign_grid_field_from_source_sub);
	if (element && data)
	{
		int number_of_components = cmzn_field_get_number_of_components(data->source_field);
		element_point_ranges = (struct Element_point_ranges *)NULL;
		return_code = 1;
		/* trivial rejection to see if element has storage for any grid based field 
			- no faces or lines */
		if (FE_element_has_values_storage(element))
		{
			if (Computed_field_get_native_discretization_in_element(
				data->destination_field, element,
				element_point_ranges_identifier.number_in_xi))
			{
				destination_field_is_grid_based = 1;
			}
			else
			{
				destination_field_is_grid_based = 0;
			}
			if (data->group_field)
			{
				if ((CMZN_OK == cmzn_fieldcache_set_element(data->field_cache, element)) &&
					cmzn_field_evaluate_boolean(data->group_field, data->field_cache))
				{
					element_selected = 1;
					can_select_individual_points = 0;
				}
				else
				{
					element_selected = 0;
					can_select_individual_points = 1;
				}
			}
			else
			{
				element_selected = 1;
				can_select_individual_points = 1;
			}
			if (destination_field_is_grid_based)
			{
				if (can_select_individual_points &&
					data->element_point_ranges_selection)
				{
					element_point_ranges_identifier.element = element;
					element_point_ranges_identifier.top_level_element = element;
					element_point_ranges_identifier.sampling_mode = CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_CORNERS;
					/* already set the number_in_xi, above */
					if (0 != (element_point_ranges = ACCESS(Element_point_ranges)(
							 FIND_BY_IDENTIFIER_IN_LIST(
							 Element_point_ranges, identifier)(&element_point_ranges_identifier,
								 Element_point_ranges_selection_get_element_point_ranges_list(
									 data->element_point_ranges_selection)))))
					{
						element_selected = 1;
					}
					else
					{
						element_selected = 0;
					}
				}
				else
				{
					element_point_ranges_identifier.element = element;
					element_point_ranges_identifier.top_level_element = element;
					element_point_ranges_identifier.sampling_mode = CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_CORNERS;
					/* already set the number_in_xi, above */
					element_point_ranges = ACCESS(Element_point_ranges)(
						CREATE(Element_point_ranges)(
							&element_point_ranges_identifier));
					FE_element_get_xi_points(element, 
						element_point_ranges_identifier.sampling_mode,
						element_point_ranges_identifier.number_in_xi,
						element_point_ranges_identifier.exact_xi,
						(cmzn_fieldcache_id)0,
						/*coordinate_field*/(struct Computed_field *)NULL,
						/*density_field*/(struct Computed_field *)NULL,
						&maximum_element_point_number,
						/*xi_points_address*/(FE_value_triple **)NULL);
					Element_point_ranges_add_range(element_point_ranges,
						0, maximum_element_point_number - 1);
				}
			}
			if (element_selected)
			{
				data->selected_count++;
				if (destination_field_is_grid_based &&
					(CMZN_OK == cmzn_fieldcache_set_element(data->field_cache, element)) &&
					cmzn_field_is_defined_at_location(data->source_field, data->field_cache) &&
					ALLOCATE(values, FE_value, number_of_components))
				{
					if (element_point_ranges)
					{
						selected_ranges = 
							Element_point_ranges_get_ranges(element_point_ranges);
						
						number_of_ranges =
							Multi_range_get_number_of_ranges(selected_ranges);
						for (i = 0; i < number_of_ranges; i++)
						{
							if (Multi_range_get_range(selected_ranges, i, &start, &stop))
							{
								for (grid_point_number = start ; grid_point_number <= stop ; grid_point_number++)
								{
									if (FE_element_get_numbered_xi_point(
											 element, element_point_ranges_identifier.sampling_mode,
											 element_point_ranges_identifier.number_in_xi, element_point_ranges_identifier.exact_xi,
											 (cmzn_fieldcache_id)0,
											 /*coordinate_field*/(struct Computed_field *)NULL,
											 /*density_field*/(struct Computed_field *)NULL,
											 grid_point_number, xi))
									{
										if ((CMZN_OK == cmzn_fieldcache_set_mesh_location(data->field_cache,
												element, MAXIMUM_ELEMENT_XI_DIMENSIONS, xi)) &&
											(CMZN_OK == cmzn_field_evaluate_real(data->source_field,
												data->field_cache, number_of_components, values)))
										{
											cmzn_field_assign_real(data->destination_field,
												data->field_cache, number_of_components, values);
										}
									}
								}
							}
						}
					}
					data->success_count++;
					DEALLOCATE(values);
				}
			}
			if (element_point_ranges)
			{
				DEACCESS(Element_point_ranges)(&element_point_ranges);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_element_assign_grid_field_from_source_sub.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_element_assign_grid_field_from_source_sub */

int cmzn_mesh_assign_grid_field_from_source(
	cmzn_mesh_id mesh, cmzn_field_id destination_field,
	cmzn_field_id source_field, cmzn_field_id conditional_field,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	FE_value time)
{
	int return_code = 1;
	if (mesh && destination_field && source_field)
	{
		if (Computed_field_get_number_of_components(source_field) ==
			 Computed_field_get_number_of_components(destination_field))
		{
			cmzn_region_id region = cmzn_mesh_get_region_internal(mesh);
			cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
			cmzn_fieldmodule_begin_change(field_module);
			cmzn_fieldcache_id field_cache = cmzn_fieldmodule_create_fieldcache(field_module);
			cmzn_fieldcache_set_time(field_cache, time);
			cmzn_element_assign_grid_field_from_source_data data;
			data.field_cache = field_cache;
			data.source_field = source_field;
			data.destination_field = destination_field;
			data.element_point_ranges_selection = element_point_ranges_selection;
			data.group_field = conditional_field;
			data.selected_count = 0;
			data.success_count = 0;
			cmzn_elementiterator_id iter = cmzn_mesh_create_elementiterator(mesh);
			cmzn_element_id element = 0;
			while (0 != (element = cmzn_elementiterator_next_non_access(iter)))
			{
				if (!cmzn_element_assign_grid_field_from_source_sub(element, &data))
				{
					return_code = 0;
					break;
				}
			}
			cmzn_elementiterator_destroy(&iter);
			if (data.success_count != data.selected_count)
			{
				display_message(ERROR_MESSAGE,
					"cmzn_mesh_assign_grid_field_from_source."
					"  Only able to set values for %d elements out of %d\n"
					"  Either source field isn't defined in element "
					"or destination field could not be set.",
					data.success_count, data.selected_count);
				return_code = 0;
			}
			cmzn_fieldcache_destroy(&field_cache);
			cmzn_fieldmodule_end_change(field_module);
			cmzn_fieldmodule_destroy(&field_module);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmzn_mesh_assign_grid_field_from_source.  "
				"Number of components in source and destination fields must match.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_mesh_assign_grid_field_from_source.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}
