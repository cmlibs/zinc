/*******************************************************************************
FILE : computed_field_update.c

LAST MODIFIED : 12 October 2001

DESCRIPTION :
Functions for updating values of one computed field from those of another.
==============================================================================*/
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_update.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_discretization.h"
#include "general/debug.h"
#include "user_interface/message.h"

struct Computed_field_update_nodal_values_from_source_data
/*******************************************************************************
LAST MODIFIED : 11 October 2001

DESCRIPTION :
==============================================================================*/
{
	int selected_count, success_count;
	FE_value *values;
	struct Computed_field *source_field;
	struct Computed_field *destination_field;
	struct FE_node_selection *node_selection;
	struct MANAGER(FE_node) *node_manager;
};

static int Computed_field_update_nodal_values_from_source_sub(
	struct FE_node *node, void *data_void)
/*******************************************************************************
LAST MODIFIED : 11 October 2001

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
					data->values))
				{
					if (Computed_field_set_values_at_managed_node(data->destination_field,
						node, data->values, data->node_manager))
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
	struct GROUP(FE_node) *node_group, struct MANAGER(FE_node) *node_manager,
	struct FE_node_selection *node_selection)
/*******************************************************************************
LAST MODIFIED : 11 October 2001

DESCRIPTION :
Set <destination_field> in all the nodes in <node_group> or <node_manager> if
not supplied to the values from <source_field>.
Restricts update to nodes in <node_selection>, if supplied.
==============================================================================*/
{
	int return_code;
	struct Computed_field_update_nodal_values_from_source_data data;

	ENTER(Computed_field_update_nodal_values_from_source);
	return_code = 0;
	if (destination_field && source_field && node_manager)
	{
		if (Computed_field_get_number_of_components(source_field) ==
			Computed_field_get_number_of_components(destination_field))
		{
			if (ALLOCATE(data.values, FE_value, 
				Computed_field_get_number_of_components(source_field)))
			{
				data.source_field = source_field;
				data.destination_field = destination_field;
				data.node_manager = node_manager;
				data.node_selection = node_selection;
				data.selected_count = 0;
				data.success_count = 0;

				MANAGER_BEGIN_CACHE(FE_node)(node_manager);
				if (node_group)
				{
					FOR_EACH_OBJECT_IN_GROUP(FE_node)(
						Computed_field_update_nodal_values_from_source_sub,
						(void *)&data, node_group);
				}
				else
				{
					FOR_EACH_OBJECT_IN_MANAGER(FE_node)(
						Computed_field_update_nodal_values_from_source_sub,
						(void *)&data, node_manager);
				}
				if (data.success_count != data.selected_count)
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_update_nodal_values_from_source.  "
						"Only able to set values for %d nodes out of %d\n"
						"  Either source field isn't defined at node "
						"or destination field could not be set.",
						data.success_count, data.selected_count);
				}
				MANAGER_END_CACHE(FE_node)(node_manager);
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
LAST MODIFIED : 11 October 2001

DESCRIPTION :
==============================================================================*/
{
	int selected_count, success_count;
	struct Computed_field *source_field;
	struct Computed_field *destination_field;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct FE_element_selection *element_selection;
	struct MANAGER(FE_element) *element_manager;
};

static int Computed_field_update_element_values_from_source_sub(
	struct FE_element *element, void *data_void)
/*******************************************************************************
LAST MODIFIED : 15 October 2001

DESCRIPTION :
==============================================================================*/
{
	FE_value *new_value, *new_values, *temp_values, *value, *values;
	int can_select_individual_points, destination_field_is_grid_based,
		element_selected, i, j, k, offset, number_of_components, number_of_ranges,
		number_of_xi_points, return_code, start, stop, success;
	struct Computed_field_update_element_values_from_source_data *data;
	struct Element_point_ranges *element_point_ranges;
	struct Element_point_ranges_identifier element_point_ranges_identifier;
	struct Multi_range *selected_ranges;

	ENTER(Computed_field_update_element_values_from_source_sub);
	if (element && (data =
		(struct Computed_field_update_element_values_from_source_data *)data_void))
	{
		return_code = 1;
		/* elements with information only - no faces or lines */
		if (element->information)
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
			if (can_select_individual_points &&
				destination_field_is_grid_based &&
				data->element_point_ranges_selection)
			{
				element_point_ranges_identifier.element = element;
				element_point_ranges_identifier.top_level_element = element;
				element_point_ranges_identifier.xi_discretization_mode =
					XI_DISCRETIZATION_CELL_CORNERS;
				/* already set the number_in_xi, above */
				if (element_point_ranges = FIND_BY_IDENTIFIER_IN_LIST(
					Element_point_ranges, identifier)(&element_point_ranges_identifier,
						Element_point_ranges_selection_get_element_point_ranges_list(
							data->element_point_ranges_selection)))
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
				element_point_ranges = (struct Element_point_ranges *)NULL;
			}
			if (element_selected)
			{
				data->selected_count++;
				if (destination_field_is_grid_based &&
					Computed_field_is_defined_in_element(data->source_field, element))
				{
					if (Computed_field_get_values_in_element(data->source_field, element,
						element_point_ranges_identifier.number_in_xi, &values))
					{
						/* if individual grid points to be updated, need to evaluate
							 the current field values and overwrite the selected ones */
						if (element_point_ranges)
						{
							if (Computed_field_get_values_in_element(
								data->destination_field, element,
								element_point_ranges_identifier.number_in_xi,
								&temp_values))
							{
								/* make values point at the current values */
								new_values = values;
								values = temp_values;

								success = 1;
								value = values;
								new_value = new_values;
								FE_element_get_xi_points(element,
									element_point_ranges_identifier.xi_discretization_mode,
									element_point_ranges_identifier.number_in_xi,
									element_point_ranges_identifier.exact_xi,
									/*coordinate_field*/(struct Computed_field *)NULL,
									/*density_field*/(struct Computed_field *)NULL,
									&number_of_xi_points,
									/*xi_points_address*/(Triple **)NULL);
								number_of_components = Computed_field_get_number_of_components(
									data->destination_field);
								selected_ranges = 
									Element_point_ranges_get_ranges(element_point_ranges);

								number_of_ranges =
									Multi_range_get_number_of_ranges(selected_ranges);
								for (i = 0; i < number_of_ranges; i++)
								{
									if (Multi_range_get_range(selected_ranges, i, &start, &stop)
										&& (stop < number_of_xi_points))
									{
										for (j = 0; j < number_of_components; j++)
										{
											offset = j*number_of_xi_points + start;
											value = values + offset;
											new_value = new_values + offset;
											for (k = start; k <= stop; k++)
											{
												*value = *new_value;
												value++;
												new_value++;
											}
										}									
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"Computed_field_update_element_values_from_source_sub.  "
											"Invalid element point ranges");
										success = 0;
									}
								}
								DEALLOCATE(new_values);
							}
							else
							{
								success = 0;
							}
						}
						else
						{
							success = 1;
						}
						if (success)
						{
							if (Computed_field_set_values_in_managed_element(
								data->destination_field, element,
								element_point_ranges_identifier.number_in_xi,
								values, data->element_manager))
							{
								data->success_count++;
							}
						}
						DEALLOCATE(values);
					}
				}
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
	struct GROUP(FE_element) *element_group,
	struct MANAGER(FE_element) *element_manager,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection)
/*******************************************************************************
LAST MODIFIED : 11 October 2001

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

	ENTER(Computed_field_update_element_values_from_source);
	return_code = 0;
	if (destination_field && source_field && element_manager)
	{
		if (Computed_field_get_number_of_components(source_field) ==
			 Computed_field_get_number_of_components(destination_field))
		{
			return_code = 1;
			data.source_field = source_field;
			data.destination_field = destination_field;
			data.element_manager = element_manager;
			data.element_point_ranges_selection = element_point_ranges_selection;
			data.element_selection = element_selection;
			data.selected_count = 0;
			data.success_count = 0;
			MANAGER_BEGIN_CACHE(FE_element)(element_manager);
			if (element_group)
			{
				FOR_EACH_OBJECT_IN_GROUP(FE_element)(
					Computed_field_update_element_values_from_source_sub,
					(void *)&data, element_group);
			}
			else
			{
				FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
					Computed_field_update_element_values_from_source_sub,
					(void *)&data, element_manager);
			}
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
			MANAGER_END_CACHE(FE_element)(element_manager);
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
