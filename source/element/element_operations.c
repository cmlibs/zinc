/*******************************************************************************
FILE : element_operations.c

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
#include <stdlib.h>
#include "command/command.h"
#include "element/element_operations.h"
#include "finite_element/finite_element_discretization.h"
#include "general/debug.h"
#include "graphics/auxiliary_graphics_types.h"
#include "user_interface/message.h"

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
	enum CM_element_type cm_element_type;
	struct FE_region *fe_region;
	int selected_flag;
	struct LIST(FE_element) *element_selection_list;
	struct Multi_range *element_ranges;
	struct Computed_field *conditional_field;
	FE_value conditional_field_time;
	struct LIST(FE_element) *element_list;
}; /* struct FE_element_fe_region_selection_ranges_condition_data */

static int FE_element_add_if_selection_ranges_condition(struct FE_element *element,
	void *data_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code, selected;
	struct CM_element_type_Multi_range_data element_data;
	struct FE_element_fe_region_selection_ranges_condition_data *data;

	ENTER(FE_element_fe_region_ranges_condition);
	if (element && 
		(data = (struct FE_element_fe_region_selection_ranges_condition_data *)data_void))
	{
		return_code = 1;
		selected = 1;
		if (selected && data->selected_flag)
		{
			selected = IS_OBJECT_IN_LIST(FE_element)(element, data->element_selection_list);
		}
		if (selected && data->element_ranges)
		{
			element_data.cm_element_type = data->cm_element_type;
			element_data.multi_range = data->element_ranges;
			selected = FE_element_of_CM_element_type_is_in_Multi_range(element,
				(void *)&element_data);
		}
		if (selected && data->conditional_field)
		{
			selected = Computed_field_is_true_in_element(data->conditional_field,
				element, data->conditional_field_time);
		}
		if (selected)
		{
			return_code = ADD_OBJECT_TO_LIST(FE_element)(element, data->element_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_set_FE_element_field_info.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_set_FE_element_field_info */

static int FE_element_add_if_fe_region_ranges_condition(struct FE_element *element,
	void *data_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code, selected;
	struct CM_element_type_Multi_range_data element_data;
	struct FE_element_fe_region_selection_ranges_condition_data *data;

	ENTER(FE_element_fe_region_ranges_condition);
	if (element && 
		(data = (struct FE_element_fe_region_selection_ranges_condition_data *)data_void))
	{
		return_code = 1;
		selected = FE_region_contains_FE_element(data->fe_region, element);
		if (selected && data->element_ranges)
		{
			element_data.cm_element_type = data->cm_element_type;
			element_data.multi_range = data->element_ranges;
			selected = FE_element_of_CM_element_type_is_in_Multi_range(element,
				(void *)&element_data);
		}
		if (selected && data->conditional_field)
		{
			selected = Computed_field_is_true_in_element(data->conditional_field,
				element, data->conditional_field_time);
		}
		if (selected)
		{
			return_code = ADD_OBJECT_TO_LIST(FE_element)(element, data->element_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_set_FE_element_field_info.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_set_FE_element_field_info */

struct LIST(FE_element) *
	FE_element_list_from_fe_region_selection_ranges_condition(
		struct FE_region *fe_region, enum CM_element_type cm_element_type,
		struct FE_element_selection *element_selection,
		int selected_flag, struct Multi_range *element_ranges,
		struct Computed_field *conditional_field, FE_value time)
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Creates and returns an element list that is the intersection of:
- all the elements in <fe_region>;
- all elements in the <element_selection> if <selected_flag> is set;
- all elements in the given <element_ranges>, if any.
- all elements for which the <conditional_field> evaluates as "true"
  in its centre at the specified <time>
Up to the calling function to destroy the returned element list.
==============================================================================*/
{
	int i, element_number, elements_in_region, elements_in_selection, elements_in_ranges,
		number_of_ranges, return_code, selected, start, stop;
	struct CM_element_information element_id;
	struct FE_element *element;
	struct FE_element_fe_region_selection_ranges_condition_data data;

	ENTER(FE_element_list_from_fe_region_selection_ranges_condition);
	data.element_list = (struct LIST(FE_element) *)NULL;
	if (fe_region && ((!selected_flag) || element_selection))
	{
		if (data.element_list = CREATE(LIST(FE_element))())
		{
			elements_in_region = FE_region_get_number_of_FE_elements(fe_region);
			if (selected_flag)
			{
				elements_in_selection = NUMBER_IN_LIST(FE_element)
					(FE_element_selection_get_element_list(element_selection));
			}
			if (element_ranges)
			{
				elements_in_ranges = Multi_range_get_total_number_in_ranges(element_ranges);
			}

			data.fe_region = fe_region;
			data.cm_element_type = cm_element_type;
			data.selected_flag = selected_flag;
			data.element_selection_list = FE_element_selection_get_element_list(element_selection);
			/* Seems odd to specify an empty element_ranges but I have
				maintained the previous behaviour */
			if (element_ranges &&
				(0 < (number_of_ranges = Multi_range_get_number_of_ranges(element_ranges))))
			{
				data.element_ranges = element_ranges;
			}
			else
			{
				data.element_ranges = (struct Multi_range *)NULL;
			}
			data.conditional_field = conditional_field;
			data.conditional_field_time = time;

			if (data.element_ranges
				&& (elements_in_ranges < elements_in_region)
				&& (!selected_flag || (elements_in_ranges < elements_in_selection)))
			{
				return_code = 1;
				for (i = 0 ; i < number_of_ranges ; i++)
				{
					Multi_range_get_range(element_ranges, i, &start, &stop);
					for (element_number = start ; element_number <= stop ; element_number++)
					{
						element_id.type = cm_element_type;
						element_id.number = element_number;
						if (element = FE_region_get_FE_element_from_identifier(
								 fe_region, &element_id))
						{
							selected = 1;
							if (data.selected_flag)
							{
								if (!FIND_BY_IDENTIFIER_IN_LIST(FE_element,identifier)
									(&element_id, data.element_selection_list))
								{
									selected = 0;
								}
							}
							if (selected && conditional_field)
							{
								selected = Computed_field_is_true_in_element(conditional_field,
									element, time);
							}
							if (selected)
							{
								ADD_OBJECT_TO_LIST(FE_element)(element, data.element_list);
							}
						}
					}
				}
			}
			else if (selected_flag && (elements_in_selection < elements_in_region))
			{
				return_code = FOR_EACH_OBJECT_IN_LIST(FE_element)(
					FE_element_add_if_fe_region_ranges_condition,
					(void *)&data, data.element_selection_list);
			}
			else
			{
				return_code =  FE_region_for_each_FE_element(fe_region,
					FE_element_add_if_selection_ranges_condition, (void *)&data);
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

#if defined (OLD_CODE)
struct LIST(FE_element) *
	FE_element_list_from_fe_region_selection_ranges_condition(
		struct FE_region *fe_region, enum CM_element_type cm_element_type,
		struct FE_element_selection *element_selection, int selected_flag,
		struct Multi_range *element_ranges,
		struct Computed_field *conditional_field, FE_value time)
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Creates and returns an element list that is the intersection of:
- all the elements in <fe_region>;
- all elements in the <element_selection> if <selected_flag> is set;
- all elements in the given <element_ranges>, if any.
- all elements for which the <conditional_field> evaluates as "true"
  in its centre at the specified <time>
Up to the calling function to destroy the returned element list.
==============================================================================*/
{
	int return_code;
	struct Computed_field_conditional_data conditional_data;
	struct CM_element_type_Multi_range_data element_type_ranges_data;
	struct FE_element_list_CM_element_type_data element_list_type_data;
	struct LIST(FE_element) *element_list;

	ENTER(FE_element_list_from_fe_region_selection_ranges_condition);
	element_list = (struct LIST(FE_element) *)NULL;
	if (fe_region && ((!selected_flag) || element_selection))
	{
		if (element_list = CREATE(LIST(FE_element))())
		{
			/* start with list of the elements of cm_element_type from fe_region */
			element_list_type_data.cm_element_type = cm_element_type;
			element_list_type_data.element_list = element_list;
			if (return_code = FE_region_for_each_FE_element(fe_region,
				add_FE_element_of_CM_element_type_to_list,
				(void *)&element_list_type_data))
			{
				if (selected_flag)
				{
					return_code = REMOVE_OBJECTS_FROM_LIST_THAT(FE_element)(
						FE_element_is_not_in_list,
						(void *)FE_element_selection_get_element_list(element_selection),
						element_list);
				}
				if (element_ranges &&
					(0 < Multi_range_get_number_of_ranges(element_ranges)))
				{
					element_type_ranges_data.cm_element_type = cm_element_type;
					element_type_ranges_data.multi_range = element_ranges;
					return_code = REMOVE_OBJECTS_FROM_LIST_THAT(FE_element)(
						FE_element_of_CM_element_type_is_not_in_Multi_range,
						(void *)&element_type_ranges_data, element_list);
				}
				if (conditional_field)
				{
					conditional_data.conditional_field = conditional_field;
					conditional_data.time = time;
					return_code = REMOVE_OBJECTS_FROM_LIST_THAT(FE_element)(
						FE_element_Computed_field_is_not_true_iterator, 
						(void *)&conditional_data, element_list);
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"FE_element_list_from_fe_region_selection_ranges_condition.  "
					"Error building list");
				DESTROY(LIST(FE_element))(&element_list);
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

	return (element_list);
} /* FE_element_list_from_fe_region_selection_ranges_condition */
#endif /* defined (OLD_CODE) */

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
	if ((element_values1 =
		(struct FE_element_values_number *)element_values1_void) &&
		(element_values2 =
			(struct FE_element_values_number *)element_values2_void) &&
		(0 < (number_of_values = element_values1->number_of_values)) &&
		(number_of_values == element_values2->number_of_values))
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
	}
	LEAVE;

	return (return_code);
} /* compare_FE_element_values_number_values */

struct FE_element_and_values_to_array_data
{
	enum CM_element_type cm_type;
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
	Triple *xi_points;

	ENTER(FE_element_and_values_to_array);
	if (element && get_FE_element_identifier(element, &cm_element_identifier) &&
		(array_data = (struct FE_element_and_values_to_array_data *)array_data_void)
		&& array_data->element_values)
	{
		return_code = 1;
		if (cm_element_identifier.type == array_data->cm_type)
		{
			array_data->element_values->element = element;
			if (array_data->sort_by_field)
			{
				/* get the centre point of the element */
				dimension = get_FE_element_dimension(element);
				for (i = 0; i < dimension; i++)
				{
					number_in_xi[i] = 1;
				}
				if (get_FE_element_shape(element, &element_shape) &&
					FE_element_shape_get_xi_points_cell_centres(element_shape,
						number_in_xi, &number_of_xi_points, &xi_points))
				{
					if (!(array_data->element_values->values &&
						Computed_field_evaluate_in_element(array_data->sort_by_field,
							element, *xi_points, array_data->time,
							/*top_level_element*/(struct FE_element *)NULL,
							array_data->element_values->values,
							/*derivatives*/(FE_value *)NULL)))
					{
						display_message(ERROR_MESSAGE, "FE_element_and_values_to_array.  "
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_and_values_to_array.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_and_values_to_array */

int FE_region_change_element_identifiers(struct FE_region *fe_region,
	enum CM_element_type cm_type,	int element_offset,
	struct Computed_field *sort_by_field, FE_value time)
/*******************************************************************************
LAST MODIFIED : 18 February 2003

DESCRIPTION :
Changes the identifiers of all elements of <cm_type> in <fe_region>.
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
	struct CM_element_information cm, tmp_cm, next_spare_element_identifier;
	struct FE_element *element_with_identifier;
	struct FE_element_and_values_to_array_data array_data;
	struct FE_element_count_if_type_data count_data;
	struct FE_element_values_number *element_values;
	struct FE_region *master_fe_region;

	ENTER(FE_region_change_element_identifiers);
	if (fe_region)
	{
		return_code = 1;
		FE_region_get_ultimate_master_FE_region(fe_region, &master_fe_region);
		/* count the number of elements of cm_type in FE_region */
		count_data.cm_type = cm_type;
		count_data.number_of_elements = 0;
		if (!FE_region_for_each_FE_element(fe_region, FE_element_count_if_type,
			(void *)&count_data))
		{
			display_message(ERROR_MESSAGE, "FE_region_change_element_identifiers.  "
				"Could not count elements of given type");
			return_code = 0;
		}
		number_of_elements = count_data.number_of_elements;
		if ((0 < number_of_elements) && return_code)
		{
			cm.type = cm_type;
			if (sort_by_field)
			{
				number_of_values =
					Computed_field_get_number_of_components(sort_by_field);
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
					element_values[i].values = (FE_value *)NULL;
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
					array_data.cm_type = cm_type;
					if (!FE_region_for_each_FE_element(fe_region,
						FE_element_and_values_to_array, (void *)&array_data))
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
							if (get_FE_element_identifier(element_values[i].element, &tmp_cm))
							{
								element_values[i].new_number = tmp_cm.number + element_offset;
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
						else if ((0 < i) && (element_values[i].new_number <=
							element_values[i - 1].new_number))
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
						cm.number = element_values[i].new_number;
						if ((element_with_identifier =
							FE_region_get_FE_element_from_identifier(master_fe_region, &cm))
							&& (!FE_region_contains_FE_element(fe_region,
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
					next_spare_element_identifier.type = cm_type;
					next_spare_element_identifier.number =
						element_values[number_of_elements - 1].new_number + 1;
					for (i = 0; (i < number_of_elements) && return_code; i++)
					{
						cm.number = element_values[i].new_number;
						element_with_identifier =
							FE_region_get_FE_element_from_identifier(fe_region, &cm);
						/* only modify if element doesn't already have correct identifier */
						if (element_with_identifier != element_values[i].element)
						{
							if (element_with_identifier)
							{
								while ((struct FE_element *)NULL !=
									FE_region_get_FE_element_from_identifier(fe_region,
										&next_spare_element_identifier))
								{
									next_spare_element_identifier.number++;
								}
								if (!FE_region_change_FE_element_identifier(master_fe_region,
									element_with_identifier, &next_spare_element_identifier))
								{
									return_code = 0;
								}
							}
							if (!FE_region_change_FE_element_identifier(master_fe_region,
								element_values[i].element, &cm))
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
	}
	LEAVE;

	return (return_code);
} /* FE_region_change_element_identifiers */
