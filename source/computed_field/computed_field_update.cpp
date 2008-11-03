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
extern "C" {
#include "computed_field/computed_field.h"
}
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "computed_field/computed_field_update.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_discretization.h"
#include "general/debug.h"
#include "user_interface/message.h"
}

int Computed_field_copy_values_at_node(struct FE_node *node,
	struct Computed_field *destination_field,
	struct Computed_field *source_field, FE_value time)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluates <source_field> at node and sets <destination_field> to those values.
<node> must not be managed -- ie. it should be a local copy.
Both fields must have the same number of values.
Assumes both fields are defined at the location
Up to user to call Computed_field_clear_cache for each field after calls to
this function are finished.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_copy_values_at_node);
	if (node && destination_field && source_field &&
		(Computed_field_get_number_of_components(destination_field) ==
			Computed_field_get_number_of_components(source_field)))
	{
		Field_node_location location(node, time);

		if (Computed_field_evaluate_cache_at_location(source_field, &location))
		{
			if (Computed_field_set_values_at_location(destination_field,
				&location, source_field->values))
			{
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_copy_values_at_node.  "
					"Destination field not defined at node");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_copy_values_at_node.  "
				"Source field not defined at node");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_copy_values_at_node.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_copy_values_at_node */

struct Computed_field_update_nodal_values_from_source_data
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int selected_count, success_count;
	FE_value *values;
	FE_value time;
	struct Computed_field *source_field;
	struct Computed_field *destination_field;
	struct FE_node_selection *node_selection;
};

int Computed_field_update_nodal_values_from_source_sub(
	struct FE_node *node, void *data_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_update_nodal_values_from_source_data *data;

	ENTER(Computed_field_update_nodal_values_from_source_sub);
	if (node && (data=
		(struct Computed_field_update_nodal_values_from_source_data *)data_void))
	{
		return_code = 1;
		if (((struct FE_node_selection *)NULL == data->node_selection) ||
			FE_node_selection_is_node_selected(data->node_selection, node))
		{
			if (Computed_field_is_defined_at_node(data->source_field, node) &&
				Computed_field_is_defined_at_node(data->destination_field, node))
			{
				if (Computed_field_evaluate_at_node(data->source_field, node,
					data->time, data->values))
				{
					if (Computed_field_set_values_at_node(data->destination_field,
							node, data->time, data->values))
					{
						data->success_count++;
					}
				}
			}
			data->selected_count++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_update_nodal_values_from_source_sub.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_update_nodal_values_from_source_sub */

int Computed_field_update_nodal_values_from_source(
	struct Computed_field *destination_field,	struct Computed_field *source_field,
	struct Cmiss_region *region, int use_data,
	struct FE_node_selection *node_selection,	FE_value time)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Set <destination_field> in all the nodes in <node_group> or <node_manager> if
not supplied to the values from <source_field>.
Restricts update to nodes in <node_selection>, if supplied.
==============================================================================*/
{
	int return_code;
	struct Computed_field_update_nodal_values_from_source_data data;
	struct FE_region *fe_region;

	ENTER(Computed_field_update_nodal_values_from_source);
	return_code = 0;
	if (destination_field && source_field && region && 
		(fe_region = Cmiss_region_get_FE_region(region)) &&
		(!use_data || (fe_region = FE_region_get_data_FE_region(fe_region))))
	{
		if (Computed_field_get_number_of_components(source_field) ==
			Computed_field_get_number_of_components(destination_field))
		{
			if (ALLOCATE(data.values, FE_value, 
				Computed_field_get_number_of_components(source_field)))
			{
				data.source_field = source_field;
				data.destination_field = destination_field;
				data.node_selection = node_selection;
				data.selected_count = 0;
				data.success_count = 0;
				data.time = time;

				FE_region_begin_change(fe_region);
				FE_region_for_each_FE_node(fe_region,
					Computed_field_update_nodal_values_from_source_sub,
						(void *)&data);
				if (data.success_count != data.selected_count)
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_update_nodal_values_from_source.  "
						"Only able to set values for %d nodes out of %d\n"
						"  Either source field isn't defined at node "
						"or destination field could not be set.",
						data.success_count, data.selected_count);
				}
				/* to be safe, clear cache of source and destination fields */
				Computed_field_clear_cache(source_field);
				Computed_field_clear_cache(destination_field);
				FE_region_end_change(fe_region);				
				DEALLOCATE(data.values);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_update_nodal_values_from_source.  "
					"Unable to allocate value storage.");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_update_nodal_values_from_source.  "
				"Number of components in source and destination fields must match.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_update_nodal_values_from_source.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_update_nodal_values_from_source */

struct Computed_field_update_element_values_from_source_data
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int selected_count, success_count, xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	FE_value time;
	struct Computed_field *source_field;
	struct Computed_field *destination_field;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct FE_element_selection *element_selection;
};

int Computed_field_update_element_values_from_source_sub(
	struct FE_element *element, void *data_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	FE_value *values, xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int can_select_individual_points, destination_field_is_grid_based,
		element_selected, grid_point_number, i, maximum_element_point_number,
		number_of_ranges, return_code, start, stop;
	struct Computed_field_update_element_values_from_source_data *data;
	struct Element_point_ranges *element_point_ranges;
	struct Element_point_ranges_identifier element_point_ranges_identifier;
	struct Multi_range *selected_ranges;

	ENTER(Computed_field_update_element_values_from_source_sub);
	if (element && (data =
		(struct Computed_field_update_element_values_from_source_data *)data_void))
	{
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
			if (data->element_selection)
			{
				if (FE_element_selection_is_element_selected(data->element_selection,
					element))
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
					element_point_ranges_identifier.xi_discretization_mode =
						XI_DISCRETIZATION_CELL_CORNERS;
					/* already set the number_in_xi, above */
					if (element_point_ranges = ACCESS(Element_point_ranges)(
							 FIND_BY_IDENTIFIER_IN_LIST(
							 Element_point_ranges, identifier)(&element_point_ranges_identifier,
								 Element_point_ranges_selection_get_element_point_ranges_list(
									 data->element_point_ranges_selection))))
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
					element_point_ranges_identifier.xi_discretization_mode =
						XI_DISCRETIZATION_CELL_CORNERS;
					/* already set the number_in_xi, above */
					element_point_ranges = ACCESS(Element_point_ranges)(
						CREATE(Element_point_ranges)(
							&element_point_ranges_identifier));
					FE_element_get_xi_points(element, 
						element_point_ranges_identifier.xi_discretization_mode,
						element_point_ranges_identifier.number_in_xi,
						element_point_ranges_identifier.exact_xi,
						/*coordinate_field*/(struct Computed_field *)NULL,
						/*density_field*/(struct Computed_field *)NULL,
						&maximum_element_point_number,
						/*xi_points_address*/(Triple **)NULL,
						data->time);
					Element_point_ranges_add_range(element_point_ranges,
						0, maximum_element_point_number - 1);
				}
			}
			if (element_selected)
			{
				data->selected_count++;
				if (destination_field_is_grid_based &&
					Computed_field_is_defined_in_element(data->source_field, element)
					&& ALLOCATE(values, FE_value, Computed_field_get_number_of_components(data->source_field)))
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
											 element, element_point_ranges_identifier.xi_discretization_mode,
											 element_point_ranges_identifier.number_in_xi, element_point_ranges_identifier.exact_xi,
											 /*coordinate_field*/(struct Computed_field *)NULL,
											 /*density_field*/(struct Computed_field *)NULL,
											 grid_point_number, xi, data->time))
									{
										Field_element_xi_location location(element, xi,
											data->time);

										if (Computed_field_evaluate_cache_at_location(
												data->source_field, &location))
										{
											Computed_field_set_values_at_location(
												data->destination_field, 
												&location, data->source_field->values);
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
			"Computed_field_update_element_values_from_source_sub.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_update_element_values_from_source_sub */

int Computed_field_update_element_values_from_source(
	struct Computed_field *destination_field,	struct Computed_field *source_field,
	struct Cmiss_region *region, struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection, FE_value time)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Set grid-based <destination_field> in all the elements in <element_group> or
<element_manager> if not supplied to the values from <source_field>.
Restricts update to grid points which are in <element_point_ranges_selection>
or whose elements are in <element_selection>, if either supplied.
Note the union of these two selections is used if both supplied.
==============================================================================*/
{
	int return_code;
	struct Computed_field_update_element_values_from_source_data data;
	struct FE_region *fe_region;

	ENTER(Computed_field_update_element_values_from_source);
	return_code = 0;
	if (destination_field && source_field && region && 
		(fe_region = Cmiss_region_get_FE_region(region)))
	{
		if (Computed_field_get_number_of_components(source_field) ==
			 Computed_field_get_number_of_components(destination_field))
		{
			return_code = 1;
			data.source_field = source_field;
			data.destination_field = destination_field;
			data.element_point_ranges_selection = element_point_ranges_selection;
			data.element_selection = element_selection;
			data.selected_count = 0;
			data.success_count = 0;
			data.time = time;
			FE_region_begin_change(fe_region);
			FE_region_for_each_FE_element(fe_region,
				Computed_field_update_element_values_from_source_sub,
				(void *)&data);
			if (data.success_count != data.selected_count)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_update_element_values_from_source."
					"  Only able to set values for %d elements out of %d\n"
					"  Either source field isn't defined in element "
					"or destination field could not be set.",
					data.success_count, data.selected_count);
				return_code=0;
			}
			/* to be safe, clear cache of source and destination fields */
			Computed_field_clear_cache(source_field);
			Computed_field_clear_cache(destination_field);
			FE_region_end_change(fe_region);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_update_element_values_from_source.  "
				"Number of components in source and destination fields must match.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_update_element_values_from_source.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_update_element_values_from_source */
