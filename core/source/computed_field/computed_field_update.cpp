/*******************************************************************************
FILE : computed_field_update.c

LAST MODIFIED : 24 August 2006

DESCRIPTION :
Functions for updating values of one computed field from those of another.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#include "zinc/fieldmodule.h"
#include "computed_field/computed_field.h"
#include "zinc/status.h"
#include "computed_field/computed_field_update.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_discretization.h"
#include "general/debug.h"
#include "general/message.h"
#include "mesh/cmiss_element_private.hpp"
#include "mesh/cmiss_node_private.hpp"

int Cmiss_nodeset_assign_field_from_source(
	Cmiss_nodeset_id nodeset, Cmiss_field_id destination_field,
	Cmiss_field_id source_field, Cmiss_field_id conditional_field,
	FE_value time)
{
	int return_code = 1;
	if (nodeset && destination_field && source_field)
	{
		const int number_of_components =
			Computed_field_get_number_of_components(destination_field);
		Cmiss_field_value_type value_type = Cmiss_field_get_value_type(destination_field);
		// can always evaluate to a string value
		if ((value_type == CMISS_FIELD_VALUE_TYPE_STRING) ||
			((Computed_field_get_number_of_components(source_field) == number_of_components) &&
				(Cmiss_field_get_value_type(source_field) == value_type)))
		{
			Cmiss_field_module_id field_module = Cmiss_field_get_field_module(destination_field);
			Cmiss_field_module_begin_change(field_module);
			Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
			FE_value *values = new FE_value[number_of_components];
			// all fields evaluated at same time so set once
			Cmiss_field_cache_set_time(field_cache, time);
			Cmiss_node_iterator_id iterator = Cmiss_nodeset_create_node_iterator(nodeset);
			Cmiss_node_id node = 0;
			int selected_count = 0;
			int success_count = 0;
			while (return_code && (0 != (node = Cmiss_node_iterator_next(iterator))))
			{
				Cmiss_field_cache_set_node(field_cache, node);
				if ((!conditional_field) || (CMISS_OK == Cmiss_field_evaluate_boolean(conditional_field, field_cache)))
				{
					if ((CMISS_OK == Cmiss_field_is_defined_at_location(destination_field, field_cache)))
					{
						switch (value_type)
						{
						case CMISS_FIELD_VALUE_TYPE_MESH_LOCATION:
							{
								FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
								Cmiss_element_id element = Cmiss_field_evaluate_mesh_location(
									source_field, field_cache, MAXIMUM_ELEMENT_XI_DIMENSIONS, xi);
								if (element)
								{
									if ((CMISS_OK == Cmiss_field_assign_mesh_location(destination_field, field_cache,
										element, MAXIMUM_ELEMENT_XI_DIMENSIONS, xi)))
									{
										++success_count;
									}
									Cmiss_element_destroy(&element);
								}
							} break;
						case CMISS_FIELD_VALUE_TYPE_REAL:
							{
								if ((CMISS_OK == Cmiss_field_evaluate_real(source_field, field_cache, number_of_components, values)) &&
									(CMISS_OK == Cmiss_field_assign_real(destination_field, field_cache, number_of_components, values)))
								{
									++success_count;
								}
							} break;
						case CMISS_FIELD_VALUE_TYPE_STRING:
							{
								char *string_value = Cmiss_field_evaluate_string(source_field, field_cache);
								if (string_value)
								{
									if ((CMISS_OK == Cmiss_field_assign_string(destination_field, field_cache, string_value)))
									{
										++success_count;
									}
									DEALLOCATE(string_value);
								}
							} break;
						default:
							{
								display_message(ERROR_MESSAGE,
									"Cmiss_nodeset_assign_field_from_source.  Unsupported value type.");
								return_code = 0;
							} break;
						}
					}
					++selected_count;
				}
				Cmiss_node_destroy(&node);
			}
			Cmiss_node_iterator_destroy(&iterator);
			if (success_count != selected_count)
			{
				display_message(WARNING_MESSAGE,
					"Cmiss_nodeset_assign_field_from_source.  "
					"Only able to set values for %d nodes out of %d\n"
					"  Either source field isn't defined at node "
					"or destination field could not be set.",
					success_count, selected_count);
			}
			delete[] values;
			Cmiss_field_cache_destroy(&field_cache);
			Cmiss_field_module_end_change(field_module);
			Cmiss_field_module_destroy(&field_module);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_nodeset_assign_field_from_source.  "
				"Value type and number of components in source and destination fields must match.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_nodeset_assign_field_from_source.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

struct Cmiss_element_assign_grid_field_from_source_data
{
	Cmiss_field_cache_id field_cache;
	int selected_count, success_count, xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct Computed_field *source_field;
	struct Computed_field *destination_field;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct Computed_field *group_field;
};

int Cmiss_element_assign_grid_field_from_source_sub(
	Cmiss_element_id element, Cmiss_element_assign_grid_field_from_source_data *data)
{
	FE_value *values, xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int can_select_individual_points, destination_field_is_grid_based,
		element_selected, grid_point_number, i, maximum_element_point_number,
		number_of_ranges, return_code, start, stop;
	struct Element_point_ranges *element_point_ranges;
	struct Element_point_ranges_identifier element_point_ranges_identifier;
	struct Multi_range *selected_ranges;

	ENTER(Cmiss_element_assign_grid_field_from_source_sub);
	if (element && data)
	{
		int number_of_components = Cmiss_field_get_number_of_components(data->source_field);
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
				if (Cmiss_field_cache_set_element(data->field_cache, element) &&
					Cmiss_field_evaluate_boolean(data->group_field, data->field_cache))
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
					element_point_ranges_identifier.sample_mode = CMISS_ELEMENT_POINT_SAMPLE_CELL_CORNERS;
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
					element_point_ranges_identifier.sample_mode = CMISS_ELEMENT_POINT_SAMPLE_CELL_CORNERS;
					/* already set the number_in_xi, above */
					element_point_ranges = ACCESS(Element_point_ranges)(
						CREATE(Element_point_ranges)(
							&element_point_ranges_identifier));
					FE_element_get_xi_points(element, 
						element_point_ranges_identifier.sample_mode,
						element_point_ranges_identifier.number_in_xi,
						element_point_ranges_identifier.exact_xi,
						(Cmiss_field_cache_id)0,
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
					Cmiss_field_cache_set_element(data->field_cache, element) &&
					Cmiss_field_is_defined_at_location(data->source_field, data->field_cache) &&
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
											 element, element_point_ranges_identifier.sample_mode,
											 element_point_ranges_identifier.number_in_xi, element_point_ranges_identifier.exact_xi,
											 (Cmiss_field_cache_id)0,
											 /*coordinate_field*/(struct Computed_field *)NULL,
											 /*density_field*/(struct Computed_field *)NULL,
											 grid_point_number, xi))
									{
										if (Cmiss_field_cache_set_mesh_location(data->field_cache,
												element, MAXIMUM_ELEMENT_XI_DIMENSIONS, xi) &&
											Cmiss_field_evaluate_real(data->source_field,
												data->field_cache, number_of_components, values))
										{
											Cmiss_field_assign_real(data->destination_field,
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
			"Cmiss_element_assign_grid_field_from_source_sub.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_element_assign_grid_field_from_source_sub */

int Cmiss_mesh_assign_grid_field_from_source(
	Cmiss_mesh_id mesh, Cmiss_field_id destination_field,
	Cmiss_field_id source_field, Cmiss_field_id conditional_field,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	FE_value time)
{
	int return_code = 1;
	if (mesh && destination_field && source_field)
	{
		if (Computed_field_get_number_of_components(source_field) ==
			 Computed_field_get_number_of_components(destination_field))
		{
			Cmiss_region_id region = Cmiss_mesh_get_region_internal(mesh);
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			Cmiss_field_module_begin_change(field_module);
			Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
			Cmiss_field_cache_set_time(field_cache, time);
			Cmiss_element_assign_grid_field_from_source_data data;
			data.field_cache = field_cache;
			data.source_field = source_field;
			data.destination_field = destination_field;
			data.element_point_ranges_selection = element_point_ranges_selection;
			data.group_field = conditional_field;
			data.selected_count = 0;
			data.success_count = 0;
			Cmiss_element_iterator_id iter = Cmiss_mesh_create_element_iterator(mesh);
			Cmiss_element_id element = 0;
			while (0 != (element = Cmiss_element_iterator_next_non_access(iter)))
			{
				if (!Cmiss_element_assign_grid_field_from_source_sub(element, &data))
				{
					return_code = 0;
					break;
				}
			}
			Cmiss_element_iterator_destroy(&iter);
			if (data.success_count != data.selected_count)
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_mesh_assign_grid_field_from_source."
					"  Only able to set values for %d elements out of %d\n"
					"  Either source field isn't defined in element "
					"or destination field could not be set.",
					data.success_count, data.selected_count);
				return_code = 0;
			}
			Cmiss_field_cache_destroy(&field_cache);
			Cmiss_field_module_end_change(field_module);
			Cmiss_field_module_destroy(&field_module);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_mesh_assign_grid_field_from_source.  "
				"Number of components in source and destination fields must match.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_mesh_assign_grid_field_from_source.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}
