/*******************************************************************************
 FILE : element_operations.cpp

 LAST MODIFIED : 3 March 2003

 DESCRIPTION :
 FE_element functions that utilise non finite element data structures and
 therefore cannot reside in finite element modules.
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

extern "C"
{
#include <stdlib.h>
#include "command/command.h"
#include "computed_field/computed_field.h"
#include "api/cmiss_field_subobject_group.h"
#include "computed_field/computed_field_group.h"
#include "element/element_operations.h"
#include "finite_element/finite_element_discretization.h"
#include "general/debug.h"
#include "graphics/auxiliary_graphics_types.h"
#include "user_interface/message.h"
}

/*
 Global functions
 ----------------
 */

struct FE_element_fe_region_selection_ranges_condition_data
/*******************************************************************************
 LAST MODIFIED : 15 May 2006

 DESCRIPTION :
 ==============================================================================*/
{
	struct FE_region *fe_region;
	struct Multi_range *element_ranges;
	struct Computed_field *conditional_field, *group_field;
	FE_value conditional_field_time;
	struct LIST(FE_element) *element_list;
}; /* struct FE_element_fe_region_selection_ranges_condition_data */

struct FE_element_values_number
/*******************************************************************************
 LAST MODIFIED : 22 December 2000

 DESCRIPTION :
 Data for changing element identifiers.
 ==============================================================================*/
{
	struct FE_element *element;
	int number_of_values;
	FE_value *values;
	int new_number;
};

static int compare_FE_element_values_number_values(
		const void *element_values1_void, const void *element_values2_void)
/*******************************************************************************
 LAST MODIFIED : 22 December 2000

 DESCRIPTION :
 Compares the values in <element_values1> and <element_values2> from the last to
 then first, returning -1 as soon as a value in <element_values1> is less than
 its counterpart in <element_values2>, or 1 if greater. 0 is returned if all
 values are identival. Used as a compare function for qsort.
 ==============================================================================*/
{
	int i, number_of_values, return_code;
	struct FE_element_values_number *element_values1, *element_values2;

	ENTER(compare_FE_element_values_number_values);
	return_code = 0;
	if ((element_values1
			= (struct FE_element_values_number *) element_values1_void)
			&& (element_values2
				= (struct FE_element_values_number *) element_values2_void)
			&& (0 < (number_of_values = element_values1->number_of_values))
			&& (number_of_values == element_values2->number_of_values))
	{
		for (i = number_of_values - 1; (!return_code) && (0 <= i); i--)
		{
			if (element_values1->values[i] < element_values2->values[i])
			{
				return_code = -1;
			}
			else if (element_values1->values[i] > element_values2->values[i])
			{
				return_code = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"compare_FE_element_values_number_values.  Invalid argument(s)");
	}LEAVE;

	return (return_code);
} /* compare_FE_element_values_number_values */

struct FE_element_and_values_to_array_data
{
	FE_value time;
	struct FE_element_values_number *element_values;
	struct Computed_field *sort_by_field;
}; /* FE_element_and_values_to_array_data */

static int FE_element_and_values_to_array(struct FE_element *element,
		void *array_data_void)
/*******************************************************************************
 LAST MODIFIED : 16 January 2003

 DESCRIPTION :
 ==============================================================================*/
{
	struct CM_element_information cm_element_identifier;
	int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS], number_of_xi_points;
	int dimension, i, return_code;
	struct FE_element_and_values_to_array_data *array_data;
	struct FE_element_shape *element_shape;
	FE_value_triple *xi_points;

	ENTER(FE_element_and_values_to_array);
	if (element && get_FE_element_identifier(element, &cm_element_identifier)
			&& (array_data
				= (struct FE_element_and_values_to_array_data *) array_data_void)
			&& array_data->element_values)
	{
		return_code = 1;
		array_data->element_values->element = element;
		if (array_data->sort_by_field)
		{
			/* get the centre point of the element */
			dimension = get_FE_element_dimension(element);
			for (i = 0; i < dimension; i++)
			{
				number_in_xi[i] = 1;
			}
			if (get_FE_element_shape(element, &element_shape)
					&& FE_element_shape_get_xi_points_cell_centres(
						element_shape, number_in_xi,
						&number_of_xi_points, &xi_points))
			{
				if (!(array_data->element_values->values
						&& Computed_field_evaluate_in_element(
							array_data->sort_by_field,
							element,
							*xi_points,
							array_data->time,
							/*top_level_element*/(struct FE_element *) NULL,
							array_data->element_values->values,
							/*derivatives*/(FE_value *) NULL)))
				{
					display_message(ERROR_MESSAGE,
						"FE_element_and_values_to_array.  "
						"sort_by field could not be evaluated in element");
					return_code = 0;
				}
				DEALLOCATE(xi_points);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_and_values_to_array.  Error getting centre of element");
				return_code = 0;
			}
		}
		(array_data->element_values)++;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_and_values_to_array.  Invalid argument(s)");
		return_code = 0;
	}LEAVE;

	return (return_code);
} /* FE_element_and_values_to_array */

int FE_region_change_element_identifiers(struct FE_region *fe_region,
	int dimension, int element_offset,
	struct Computed_field *sort_by_field, FE_value time)
/*******************************************************************************
 LAST MODIFIED : 18 February 2003

 DESCRIPTION :
 Changes the identifiers of all elements of <dimension> in <fe_region>.
 If <sort_by_field> is NULL, adds <element_offset> to the identifiers.
 If <sort_by_field> is specified, it is evaluated at the centre of all elements
 in <fe_region> and they are sorted by it - changing fastest with the first
 component and keeping the current order where the field has the same values.
 Checks for and fails if attempting to give any of the elements in <fe_region> an
 identifier already used by an element in the same master FE_region.
 Calls to this function should be enclosed in FE_region_begin_change/end_change.
 Note function avoids iterating through FE_region element lists as this is not
 allowed during identifier changes.
 ==============================================================================*/
{
	int i, number_of_elements, number_of_values, return_code;
	struct FE_element *element_with_identifier;
	struct FE_element_and_values_to_array_data array_data;
	struct FE_element_values_number *element_values;
	struct FE_region *master_fe_region;

	ENTER(FE_region_change_element_identifiers);
	if (fe_region)
	{
		return_code = 1;
		FE_region_get_ultimate_master_FE_region(fe_region, &master_fe_region);
		number_of_elements = FE_region_get_number_of_FE_elements_of_dimension(
			fe_region, dimension);
		if ((0 < number_of_elements) && return_code)
		{
			if (sort_by_field)
			{
				number_of_values = Computed_field_get_number_of_components(
					sort_by_field);
			}
			else
			{
				number_of_values = 0;
			}
			if (ALLOCATE(element_values, struct FE_element_values_number,
				number_of_elements))
			{
				for (i = 0; i < number_of_elements; i++)
				{
					element_values[i].number_of_values = number_of_values;
					element_values[i].values = (FE_value *) NULL;
				}
				if (sort_by_field)
				{
					for (i = 0; (i < number_of_elements) && return_code; i++)
					{
						if (!ALLOCATE(element_values[i].values, FE_value, number_of_values))
						{
							display_message(ERROR_MESSAGE,
								"FE_region_change_element_identifiers.  Not enough memory");
							return_code = 0;
						}
					}
				}
				if (return_code)
				{
					/* make a linear array of elements in the group in current order */
					array_data.element_values = element_values;
					array_data.sort_by_field = sort_by_field;
					array_data.time = time;
					if (!FE_region_for_each_FE_element_of_dimension(fe_region,
						dimension, FE_element_and_values_to_array,
						(void *) &array_data))
					{
						display_message(ERROR_MESSAGE,
							"FE_region_change_element_identifiers.  "
							"Could not build element/field values array");
						return_code = 0;
					}
				}
				if (return_code)
				{
					if (sort_by_field)
					{
						/* sort by field values with higher components more significant */
						qsort(element_values, number_of_elements,
							sizeof(struct FE_element_values_number),
							compare_FE_element_values_number_values);
						/* give the elements sequential values starting at element_offset */
						for (i = 0; i < number_of_elements; i++)
						{
							element_values[i].new_number = element_offset + i;
						}
					}
					else
					{
						/* offset element numbers by element_offset */
						for (i = 0; (i < number_of_elements) && return_code; i++)
						{
							struct CM_element_information tmp_cm;
							if (get_FE_element_identifier(
								element_values[i].element, &tmp_cm))
							{
								element_values[i].new_number = tmp_cm.number
									+ element_offset;
							}
						}
					}
					/* check element numbers are positive and ascending */
					for (i = 0; (i < number_of_elements) && return_code; i++)
					{
						if (0 >= element_values[i].new_number)
						{
							display_message(ERROR_MESSAGE,
								"FE_region_change_element_identifiers.  "
								"element_offset gives negative element numbers");
							return_code = 0;
						}
						else if ((0 < i) && (element_values[i].new_number
							<= element_values[i - 1].new_number))
						{
							display_message(ERROR_MESSAGE,
								"FE_region_change_element_identifiers.  "
								"Element numbers are not strictly increasing");
							return_code = 0;
						}
					}
				}
				if (return_code)
				{
					/* check none of the new numbers are in use by other elements
					 in the master_fe_region */
					for (i = 0; (i < number_of_elements) && return_code; i++)
					{
						element_with_identifier = FE_region_get_FE_element_from_identifier(
							master_fe_region, dimension, element_values[i].new_number);
						if ((element_with_identifier) &&
							(!FE_region_contains_FE_element(fe_region,
								element_with_identifier)))
						{
							display_message(ERROR_MESSAGE,
								"FE_region_change_element_identifiers.  "
								"Element using new number already exists in master region");
							return_code = 0;
						}
					}
				}
				if (return_code)
				{
					/* change identifiers */
					/* maintain next_spare_element_number to renumber elements in same
					 group which already have the same number as the new_number */
					int next_spare_element_number =
						element_values[number_of_elements - 1].new_number + 1;
					for (i = 0; (i < number_of_elements) && return_code; i++)
					{
						element_with_identifier = FE_region_get_FE_element_from_identifier(
							fe_region, dimension, element_values[i].new_number);
						/* only modify if element doesn't already have correct identifier */
						if (element_with_identifier
							!= element_values[i].element)
						{
							if (element_with_identifier)
							{
								while ((struct FE_element *)NULL !=
									FE_region_get_FE_element_from_identifier(
										fe_region, dimension, next_spare_element_number))
								{
									++next_spare_element_number;
								}
								if (!FE_region_change_FE_element_identifier(
									master_fe_region,
									element_with_identifier,
									next_spare_element_number))
								{
									return_code = 0;
								}
							}
							if (!FE_region_change_FE_element_identifier(
								master_fe_region,
								element_values[i].element, element_values[i].new_number))
							{
								display_message(ERROR_MESSAGE,
									"FE_region_change_element_identifiers.  "
									"Could not change element identifier");
								return_code = 0;
							}
						}
					}
				}
				for (i = 0; i < number_of_elements; i++)
				{
					if (element_values[i].values)
					{
						DEALLOCATE(element_values[i].values);
					}
				}
				DEALLOCATE(element_values);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_region_change_element_identifiers.  Not enough memory");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_change_element_identifiers.  Invalid argument(s)");
		return_code = 0;
	}LEAVE;

	return (return_code);
} /* FE_region_change_element_identifiers */

static int FE_element_add_if_selection_ranges_condition_with_group(
		struct FE_element *element, void *data_void)
/*******************************************************************************
 LAST MODIFIED : 10 March 2008

 DESCRIPTION :
 ==============================================================================*/
{
	int return_code, selected;
	struct CM_element_information identifier;
	struct FE_element_fe_region_selection_ranges_condition_data *data;

	ENTER(FE_element_add_if_selection_ranges_condition);
	if (element
		&& (data
			= (struct FE_element_fe_region_selection_ranges_condition_data *) data_void))
	{
		return_code = get_FE_element_identifier(element, &identifier);
		selected = 1;
		if (selected && data->element_ranges)
		{
			selected = Multi_range_is_value_in_range(data->element_ranges,
				identifier.number);
		}
		if (selected && data->group_field)
		{
			Cmiss_field_id element_group_field = data->group_field;
			Cmiss_field_element_group_id element_group =
				Cmiss_field_cast_element_group(element_group_field);
			selected = Cmiss_field_element_group_contains_element(
				element_group, element);
			Cmiss_field_destroy(&element_group_field);
		}
		if (selected && data->conditional_field)
		{
			selected = Computed_field_is_true_in_element(
				data->conditional_field, element,
				data->conditional_field_time);
		}
		if (selected)
		{
			return_code = ADD_OBJECT_TO_LIST(FE_element)(element,
				data->element_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_add_if_selection_ranges_condition.  Invalid argument(s)");
		return_code = 0;
	}LEAVE;

	return (return_code);
} /* FE_element_add_if_selection_ranges_condition */

struct LIST(FE_element) *
FE_element_list_from_region_and_selection_group(struct Cmiss_region *region,
		int dimension,
		struct Multi_range *element_ranges, struct Computed_field *group_field,
		struct Computed_field *conditional_field, FE_value time)
{
	int i, element_number, elements_in_region, elements_in_ranges = 0,
		number_of_ranges = 0, start, stop;
	struct FE_element *element;
	struct FE_element_fe_region_selection_ranges_condition_data data;
	struct FE_region *fe_region = NULL;

	ENTER(FE_element_list_from_fe_region_selection_ranges_condition);
	if (region)
	{
		fe_region = Cmiss_region_get_FE_region(region);
	}
	data.element_list = (struct LIST(FE_element) *) NULL;
	if (fe_region)
	{
		data.element_list = CREATE(LIST(FE_element))();
		if (data.element_list)
		{
			int return_code = 1;
			elements_in_region = FE_region_get_number_of_FE_elements_of_dimension(fe_region, dimension);
			if (element_ranges)
			{
				elements_in_ranges = Multi_range_get_total_number_in_ranges(
					element_ranges);
			}

			data.fe_region = fe_region;
			/* Seems odd to specify an empty element_ranges but I have
			 maintained the previous behaviour */
			if (element_ranges && (0 < (number_of_ranges
				= Multi_range_get_number_of_ranges(element_ranges))))
			{
				data.element_ranges = element_ranges;
			}
			else
			{
				data.element_ranges = (struct Multi_range *) NULL;
			}
			data.group_field = group_field;
			data.conditional_field = conditional_field;
			data.conditional_field_time = time;

			Cmiss_field_element_group_id element_group = NULL;
			if (group_field)
			{
				Cmiss_field_group_id sub_group = Cmiss_field_cast_group(group_field);
				if (sub_group)
				{
					Cmiss_fe_mesh_id temp_mesh = Cmiss_region_get_fe_mesh_by_name(region,
						((1 == dimension) ? "cmiss_mesh_1d" :
							((2 == dimension) ? "cmiss_mesh_2d" : "cmiss_mesh_3d")));
					element_group = Cmiss_field_group_get_element_group(sub_group, temp_mesh);
					Cmiss_fe_mesh_destroy(&temp_mesh);
					Cmiss_field_group_destroy(&sub_group);
				}
			}
			if ((!group_field) || (element_group))
			{
				if (data.element_ranges
						&& (elements_in_ranges < elements_in_region))
				{
					for (i = 0; (i < number_of_ranges) && return_code; i++)
					{
						Multi_range_get_range(element_ranges, i, &start, &stop);
						for (element_number = start; (element_number <= stop) && return_code; element_number++)
						{
							element = FE_region_get_FE_element_from_identifier(
								fe_region, dimension, element_number);
							if (element && ((!element_group) ||
								Cmiss_field_element_group_contains_element(element_group, element)))
							{
								return_code = ADD_OBJECT_TO_LIST(FE_element)(element, data.element_list);
							}
						}
					}
				}
				else
				{
					data.group_field = (struct Computed_field *) element_group;
					data.conditional_field = conditional_field;
					return_code	= FE_region_for_each_FE_element_of_dimension(
						fe_region, dimension,
						FE_element_add_if_selection_ranges_condition_with_group,
						(void *) &data);
				}
			}
			if (data.conditional_field)
			{
				Computed_field_clear_cache(data.conditional_field);
			}
			if (data.group_field)
			{
				Computed_field_clear_cache(data.group_field);
			}
			if (element_group)
			{
				Cmiss_field_element_group_destroy(&element_group);
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"FE_element_list_from_fe_region_selection_ranges_condition.  "
					"Error building list");
				DESTROY(LIST(FE_element))(&data.element_list);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_list_from_fe_region_selection_ranges_condition.  "
				"Could not create list");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_list_from_fe_region_selection_ranges_condition.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (data.element_list);
} /* FE_element_list_from_fe_region_selection_ranges_condition */

struct LIST(FE_element) *FE_element_list_from_conditional_field_of_dimension(struct FE_region *fe_region,
	int dimension, struct Computed_field *conditional_field, FE_value time)
{
	struct FE_element_fe_region_selection_ranges_condition_data data;

	ENTER(FE_element_list_from_fe_region_selection_ranges_condition);
	data.element_list = (struct LIST(FE_element) *)NULL;
	if (fe_region)
	{
		data.element_list = CREATE(LIST(FE_element))();
		if (data.element_list)
		{
			int return_code = 1;
			data.fe_region = fe_region;
			/* Seems odd to specify an empty element_ranges but I have
			 maintained the previous behaviour */
			data.element_ranges = (struct Multi_range *) NULL;
			data.group_field = NULL;
			data.conditional_field = conditional_field;
			data.conditional_field_time = time;
			return_code	= FE_region_for_each_FE_element_of_dimension(
				fe_region, dimension,	FE_element_add_if_selection_ranges_condition_with_group,
					(void *) &data);
			if (data.conditional_field)
			{
				Computed_field_clear_cache(data.conditional_field);
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"FE_element_list_from_conditional_field_of_dimension.  "
					"Error building list");
				DESTROY(LIST(FE_element))(&data.element_list);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_list_from_conditional_field_of_dimension.  "
				"Could not create list");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_list_from_conditional_field_of_dimension.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (data.element_list);
}
