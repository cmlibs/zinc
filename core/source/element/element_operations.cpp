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

#include <cmath>
#include <cstdlib>
extern "C"
{
#include "command/command.h"
#include "computed_field/computed_field.h"
#include "api/cmiss_field_logical_operators.h"
#include "api/cmiss_field_subobject_group.h"
#include "computed_field/computed_field_group.h"
#include "element/element_operations.h"
#include "finite_element/finite_element_discretization.h"
#include "general/debug.h"
#include "graphics/auxiliary_graphics_types.h"
#include "user_interface/message.h"
}
#include "mesh/cmiss_element_private.hpp"
#include "mesh/cmiss_node_private.hpp"

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
	Cmiss_field_cache_id field_cache;
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
				if (!(array_data->element_values->values &&
					Cmiss_field_cache_set_mesh_location(array_data->field_cache, element, dimension, *xi_points) &&
					Cmiss_field_evaluate_real(array_data->sort_by_field, array_data->field_cache,
						Cmiss_field_get_number_of_components(array_data->sort_by_field), array_data->element_values->values)))
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
					struct FE_element_and_values_to_array_data array_data;
					Cmiss_field_module_id field_module = Cmiss_region_get_field_module(FE_region_get_Cmiss_region(fe_region));
					Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
					Cmiss_field_cache_set_time(field_cache, time);
					array_data.field_cache = field_cache;
					array_data.element_values = element_values;
					array_data.sort_by_field = sort_by_field;
					if (!FE_region_for_each_FE_element_of_dimension(fe_region,
						dimension, FE_element_and_values_to_array,
						(void *) &array_data))
					{
						display_message(ERROR_MESSAGE,
							"FE_region_change_element_identifiers.  "
							"Could not build element/field values array");
						return_code = 0;
					}
					Cmiss_field_cache_destroy(&field_cache);
					Cmiss_field_module_destroy(&field_module);
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

/***************************************************************************//**
 * Create an element list from the elements in mesh optionally restricted to
 * those within the element_ranges or where conditional_field is true at time.
 *
 * @param mesh  Handle to the mesh.
 * @param element_ranges  Optional Multi_range of elements.
 * @param conditional_field  Field interpreted as a boolean value which must be
 * true for an element from mesh to be included.
 * @param time  Time to evaluate the conditional_field at.
 * @return  The element list, or NULL on failure.
 */
struct LIST(FE_element) *Cmiss_mesh_get_selected_element_list(Cmiss_mesh_id mesh,
	struct Multi_range *element_ranges, struct Computed_field *conditional_field,
	FE_value time)
{
	if (!mesh)
		return 0;
	struct LIST(FE_element) *element_list = Cmiss_mesh_create_element_list_internal(mesh);
	Cmiss_element_id element = 0;
	Cmiss_field_cache_id field_cache = 0;
	if (conditional_field)
	{
		Cmiss_field_module_id field_module = Cmiss_field_get_field_module(conditional_field);
		field_cache = Cmiss_field_module_create_cache(field_module);
		Cmiss_field_cache_set_time(field_cache, (double)time);
		Cmiss_field_module_destroy(&field_module);
	}
	if (element_ranges && (2*Multi_range_get_total_number_in_ranges(element_ranges) < Cmiss_mesh_get_size(mesh)))
	{
		const int number_of_ranges = Multi_range_get_number_of_ranges(element_ranges);
		for (int i = 0; (i < number_of_ranges) && element_list; ++i)
		{
			int start, stop;
			Multi_range_get_range(element_ranges, i, &start, &stop);
			for (int identifier = start; (identifier <= stop) && element_list; ++identifier)
			{
				element = Cmiss_mesh_find_element_by_identifier(mesh, identifier);
				if (element)
				{
					bool add = true;
					if (conditional_field)
					{
						Cmiss_field_cache_set_element(field_cache, element);
						add = Cmiss_field_evaluate_boolean(conditional_field, field_cache);
					}
					if (add && (!ADD_OBJECT_TO_LIST(FE_element)(element, element_list)))
					{
						DESTROY(LIST(FE_element))(&element_list);
					}
					Cmiss_element_destroy(&element);
				}
			}
		}
	}
	else
	{
		Cmiss_element_iterator_id iterator = Cmiss_mesh_create_element_iterator(mesh);
		while (element_list && (0 != (element = Cmiss_element_iterator_next(iterator))))
		{
			bool add = true;
			if (element_ranges)
			{
				add = Multi_range_is_value_in_range(element_ranges, Cmiss_element_get_identifier(element));
			}
			if (add && conditional_field)
			{
				Cmiss_field_cache_set_element(field_cache, element);
				add = Cmiss_field_evaluate_boolean(conditional_field, field_cache);
			}
			if (add && (!ADD_OBJECT_TO_LIST(FE_element)(element, element_list)))
			{
				DESTROY(LIST(FE_element))(&element_list);
			}
			Cmiss_element_destroy(&element);
		}
		Cmiss_element_iterator_destroy(&iterator);
	}
	Cmiss_field_cache_destroy(&field_cache);
	return element_list;
}

struct LIST(FE_element) *FE_element_list_from_region_and_selection_group(
	struct Cmiss_region *region, int dimension,
	struct Multi_range *element_ranges, struct Computed_field *group_field,
	struct Computed_field *conditional_field, FE_value time)
{
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_field_module_begin_change(field_module);
	Cmiss_mesh_id mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, dimension);
	Cmiss_field_id use_conditional_field = 0;
	if (group_field && conditional_field)
		use_conditional_field = Cmiss_field_module_create_or(field_module, group_field, conditional_field);
	else if (group_field)
		use_conditional_field = Cmiss_field_access(group_field);
	else if (conditional_field)
		use_conditional_field = Cmiss_field_access(conditional_field);
	struct LIST(FE_element) *element_list = 0;
	if (((!group_field) && (!conditional_field)) || use_conditional_field)
	{
		// code assumes no ranges = ranges not specified.
		struct Multi_range *use_element_ranges = (Multi_range_get_number_of_ranges(element_ranges) > 0) ? element_ranges : 0;
		element_list = Cmiss_mesh_get_selected_element_list(mesh, use_element_ranges, use_conditional_field, time);
	}
	if (use_conditional_field)
	{
		Cmiss_field_destroy(&use_conditional_field);
	}
	Cmiss_mesh_destroy(&mesh);
	Cmiss_field_module_end_change(field_module);
	Cmiss_field_module_destroy(&field_module);
	return element_list;
}

int Cmiss_mesh_create_gauss_points(Cmiss_mesh_id mesh, int order,
	Cmiss_nodeset_id gauss_points_nodeset, int first_identifier,
	Cmiss_field_stored_mesh_location_id gauss_location_field,
	Cmiss_field_finite_element_id gauss_weight_field)
{
	const struct
	{
		FE_value location;                               FE_value weight;
	} GaussPt[10] =
	{
		// 1 point
		{ 0.5,                                           1.0 },
		// 2 points
		{ (-1.0/sqrt(3.0)+1.0)/2.0,                      0.5 },
		{ (+1.0/sqrt(3.0)+1.0)/2.0,                      0.5 },
		// 3 points
		{ (-sqrt(0.6)+1.0)/2.0,                          5.0/18.0 },
		{ 0.5, 4.0/9.0 },
		{ (+sqrt(0.6)+1.0)/2.0,                          5.0/18.0 },
		// 4 points
		{ (-sqrt((3.0+2.0*sqrt(6.0/5.0))/7.0)+1.0)/2.0,  (18.0-sqrt(30.0))/72.0 },
		{ (-sqrt((3.0-2.0*sqrt(6.0/5.0))/7.0)+1.0)/2.0,  (18.0+sqrt(30.0))/72.0 },
		{ (+sqrt((3.0-2.0*sqrt(6.0/5.0))/7.0)+1.0)/2.0,  (18.0+sqrt(30.0))/72.0 },
		{ (+sqrt((3.0+2.0*sqrt(6.0/5.0))/7.0)+1.0)/2.0,  (18.0-sqrt(30.0))/72.0 }
	};
	const int offset[] = { 0, 1, 3, 6 };

	const struct
	{
		FE_value location[2];                               FE_value weight;
	} TriangleGaussPt[14] =
	{
		// order 1 = 1 point
		{ { 1.0/3.0, 1.0/3.0 },                             0.5 },
		// order 2 = 3 points
		{ { 1.0/6.0, 1.0/6.0 },                             1.0/6.0 },
		{ { 2.0/3.0, 1.0/6.0 },                             1.0/6.0 },
		{ { 1.0/6.0, 2.0/3.0 },                             1.0/6.0 },
		// order 3 = 4 points
		{ { 1.0/5.0, 1.0/5.0 },                             25.0 / 96.0 },
		{ { 3.0/5.0, 1.0/5.0 },                             25.0 / 96.0 },
		{ { 1.0/5.0, 3.0/5.0 },                             25.0 / 96.0 },
		{ { 1.0/3.0, 1.0/3.0 },                             -27.0 / 96.0 },
		// order 4 = 6 points
		{ { 0.091576213509771, 0.091576213509771 },         0.109951743655322*0.5 },
		{ { 0.816847572980459, 0.091576213509771 },         0.109951743655322*0.5 },
		{ { 0.091576213509771, 0.816847572980459 },         0.109951743655322*0.5 },
		{ { 0.445948490915965, 0.108103018168070 },         0.223381589678011*0.5 },
		{ { 0.108103018168070, 0.445948490915965 },         0.223381589678011*0.5 },
		{ { 0.445948490915965, 0.445948490915965 },         0.223381589678011*0.5 }
	};
	const int triangleOffset[] = { 0, 1, 4, 8 };
	const int triangleCount[] = { 1, 3, 4, 6 };

	const struct
	{
		FE_value location[3];                                          FE_value weight;
	} TetrahedronGaussPt[21] =
	{
		// order 1 = 1 point
		{ { 0.25, 0.25, 0.25 },                                        1.0/6.0 },
		// order 2 = 4 points
		{ { 0.138196601125011, 0.138196601125011, 0.138196601125011 }, 0.25/6.0 },
		{ { 0.585410196624969, 0.138196601125011, 0.138196601125011 }, 0.25/6.0 },
		{ { 0.138196601125011, 0.585410196624969, 0.138196601125011 }, 0.25/6.0 },
		{ { 0.138196601125011, 0.138196601125011, 0.585410196624969 }, 0.25/6.0 },
		// order 3 = 5 points
		{ { 1.0/6.0, 1.0/6.0, 1.0/6.0 },                               0.45/6.0 },
		{ { 1.0/2.0, 1.0/6.0, 1.0/6.0 },                               0.45/6.0 },
		{ { 1.0/6.0, 1.0/2.0, 1.0/6.0 },                               0.45/6.0 },
		{ { 1.0/6.0, 1.0/6.0, 1.0/2.0 },                               0.45/6.0 },
		{ { 0.25, 0.25, 0.25 },                                        -0.8/6.0 },
		// order 4 = 11 points
		{ { 0.071428571428571, 0.071428571428571, 0.071428571428571 }, 0.007622222222222 },
		{ { 0.785714285714286, 0.071428571428571, 0.071428571428571 }, 0.007622222222222 },
		{ { 0.071428571428571, 0.785714285714286, 0.071428571428571 }, 0.007622222222222 },
		{ { 0.071428571428571, 0.071428571428571, 0.785714285714286 }, 0.007622222222222 },
		{ { 0.399403576166799, 0.100596423833201, 0.100596423833201 }, 0.024888888888889 },
		{ { 0.100596423833201, 0.399403576166799, 0.100596423833201 }, 0.024888888888889 },
		{ { 0.399403576166799, 0.399403576166799, 0.100596423833201 }, 0.024888888888889 },
		{ { 0.100596423833201, 0.100596423833201, 0.399403576166799 }, 0.024888888888889 },
		{ { 0.399403576166799, 0.100596423833201, 0.399403576166799 }, 0.024888888888889 },
		{ { 0.100596423833201, 0.399403576166799, 0.399403576166799 }, 0.024888888888889 },
		{ { 0.25, 0.25, 0.25 },                                        -0.013155555555556 }
	};
	const int tetrahedronOffset[] = { 0, 1, 5, 10 };
	const int tetrahedronCount[] = { 1, 4, 5, 11 };

	int return_code = 0;
	Cmiss_region_id nodeset_region = Cmiss_nodeset_get_region_internal(gauss_points_nodeset);
	Cmiss_region_id master_region = Cmiss_mesh_get_master_region_internal(mesh);
	Cmiss_field_id gauss_location_field_base = Cmiss_field_stored_mesh_location_base_cast(gauss_location_field);
	Cmiss_field_id gauss_weight_field_base = Cmiss_field_finite_element_base_cast(gauss_weight_field);
	if (mesh && (1 <= order) && (order <= 4) && gauss_points_nodeset && (first_identifier >= 0) &&
		(Cmiss_nodeset_get_master_region_internal(gauss_points_nodeset) == master_region) &&
		gauss_location_field && gauss_weight_field &&
		(Cmiss_field_get_number_of_components(gauss_location_field_base) == 1))
	{
		int dimension = Cmiss_mesh_get_dimension(mesh);
		const int order_offset = offset[order - 1];
		int number_of_gauss_points = 1;
		for (int i = 0; i < dimension; i++)
		{
			number_of_gauss_points *= order;
		}
		FE_value *gauss_locations = new FE_value[number_of_gauss_points*dimension];
		FE_value *gauss_weights = new FE_value[number_of_gauss_points];
		for (int g = 0; g < number_of_gauss_points; g++)
		{
			gauss_weights[g] = 1.0;
			int shift_g = g;
			for (int i = 0; i < dimension; i++)
			{
				int g1 = order_offset + shift_g % order;
				gauss_locations[g*dimension + i] = GaussPt[g1].location;
				gauss_weights[g] *= GaussPt[g1].weight;
				shift_g /= order;
			}
		}
		return_code = 1;
		Cmiss_field_module_id field_module = Cmiss_region_get_field_module(nodeset_region);
		Cmiss_field_module_begin_change(field_module);
		Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
		Cmiss_node_template_id node_template = Cmiss_nodeset_create_node_template(gauss_points_nodeset);
		if (!Cmiss_node_template_define_field(node_template, gauss_location_field_base))
			return_code = 0;
		if (!Cmiss_node_template_define_field(node_template, gauss_weight_field_base))
			return_code = 0;
		Cmiss_element_iterator_id iterator = Cmiss_mesh_create_element_iterator(mesh);
		Cmiss_element_id element = 0;
		int id = first_identifier;
		bool first_unknown_shape = true;
		Cmiss_nodeset_id master_gauss_points_nodeset = Cmiss_nodeset_get_master(gauss_points_nodeset);
		while ((0 != (element = Cmiss_element_iterator_next_non_access(iterator))) && return_code)
		{
			Cmiss_element_shape_type shape_type = Cmiss_element_get_shape_type(element);
			switch (shape_type)
			{
			case CMISS_ELEMENT_SHAPE_LINE:
			case CMISS_ELEMENT_SHAPE_SQUARE:
			case CMISS_ELEMENT_SHAPE_CUBE:
				{
					for (int g = 0; g < number_of_gauss_points; g++)
					{
						Cmiss_node_id node = 0;
						while ((0 != (node = Cmiss_nodeset_find_node_by_identifier(master_gauss_points_nodeset, id))))
						{
							Cmiss_node_destroy(&node);
							++id;
						}
						node = Cmiss_nodeset_create_node(gauss_points_nodeset, id, node_template);
						Cmiss_field_cache_set_node(field_cache, node);
						Cmiss_field_assign_mesh_location(gauss_location_field_base, field_cache, element, dimension, gauss_locations + g*dimension);
						Cmiss_field_assign_real(gauss_weight_field_base, field_cache, /*number_of_values*/1, gauss_weights + g);
						Cmiss_node_destroy(&node);
						id++;
					}
				} break;
			case CMISS_ELEMENT_SHAPE_TRIANGLE:
				{
					const int tri_count = triangleCount[order - 1];
					const int tri_offset = triangleOffset[order - 1];
					for (int g = 0; g < tri_count; g++)
					{
						Cmiss_node_id node = 0;
						while ((0 != (node = Cmiss_nodeset_find_node_by_identifier(master_gauss_points_nodeset, id))))
						{
							Cmiss_node_destroy(&node);
							++id;
						}
						node = Cmiss_nodeset_create_node(gauss_points_nodeset, id, node_template);
						Cmiss_field_cache_set_node(field_cache, node);
						Cmiss_field_assign_mesh_location(gauss_location_field_base, field_cache, element,
							dimension, TriangleGaussPt[g + tri_offset].location);
						Cmiss_field_assign_real(gauss_weight_field_base, field_cache,
							/*number_of_values*/1, &(TriangleGaussPt[g + tri_offset].weight));
						Cmiss_node_destroy(&node);
						id++;
					}
				} break;
			case CMISS_ELEMENT_SHAPE_TETRAHEDRON:
				{
					const int tet_count = tetrahedronCount[order - 1];
					const int tet_offset = tetrahedronOffset[order - 1];
					for (int g = 0; g < tet_count; g++)
					{
						Cmiss_node_id node = 0;
						while ((0 != (node = Cmiss_nodeset_find_node_by_identifier(master_gauss_points_nodeset, id))))
						{
							Cmiss_node_destroy(&node);
							++id;
						}
						node = Cmiss_nodeset_create_node(gauss_points_nodeset, id, node_template);
						Cmiss_field_cache_set_node(field_cache, node);
						Cmiss_field_assign_mesh_location(gauss_location_field_base, field_cache, element,
							dimension, TetrahedronGaussPt[g + tet_offset].location);
						Cmiss_field_assign_real(gauss_weight_field_base, field_cache,
							/*number_of_values*/1, &(TetrahedronGaussPt[g + tet_offset].weight));
						Cmiss_node_destroy(&node);
						id++;
					}
				} break;
			case CMISS_ELEMENT_SHAPE_WEDGE12:
			case CMISS_ELEMENT_SHAPE_WEDGE13:
			case CMISS_ELEMENT_SHAPE_WEDGE23:
				{
					const int tri_count = triangleCount[order - 1];
					const int tri_offset = triangleOffset[order - 1];
					int line_axis, tri_axis1, tri_axis2;
					if (shape_type == CMISS_ELEMENT_SHAPE_WEDGE12)
					{
						line_axis = 2;
						tri_axis1 = 0;
						tri_axis2 = 1;
					}
					else if (shape_type == CMISS_ELEMENT_SHAPE_WEDGE13)
					{
						line_axis = 1;
						tri_axis1 = 0;
						tri_axis2 = 2;
					}
					else // (shape_type == CMISS_ELEMENT_SHAPE_WEDGE23)
					{
						line_axis = 0;
						tri_axis1 = 1;
						tri_axis2 = 2;
					}
					FE_value xi_location[3];
					for (int h = 0; h < order; h++)
					{
						xi_location[line_axis] = GaussPt[order_offset + h].location;
						FE_value line_weight = GaussPt[order_offset + h].weight;
						for (int g = 0; g < tri_count; g++)
						{
							Cmiss_node_id node = 0;
							while ((0 != (node = Cmiss_nodeset_find_node_by_identifier(master_gauss_points_nodeset, id))))
							{
								Cmiss_node_destroy(&node);
								++id;
							}
							node = Cmiss_nodeset_create_node(gauss_points_nodeset, id, node_template);
							Cmiss_field_cache_set_node(field_cache, node);
							xi_location[tri_axis1] = TriangleGaussPt[g + tri_offset].location[0];
							xi_location[tri_axis2] = TriangleGaussPt[g + tri_offset].location[1];
							Cmiss_field_assign_mesh_location(gauss_location_field_base, field_cache, element,
								dimension, xi_location);
							FE_value total_weight = line_weight*TriangleGaussPt[g + tri_offset].weight;
							Cmiss_field_assign_real(gauss_weight_field_base, field_cache,
								/*number_of_values*/1, &total_weight);
							Cmiss_node_destroy(&node);
							id++;
						}
					}
				} break;
			default:
				{
					if (first_unknown_shape)
					{
						display_message(INFORMATION_MESSAGE, "gfx create gauss_points:  "
							"Unknown shape type %d encountered first for element %d. Ignoring.",
							shape_type, Cmiss_element_get_identifier(element));
						first_unknown_shape = 0;
					}
				} break;
			}
		}
		Cmiss_nodeset_destroy(&master_gauss_points_nodeset);
		Cmiss_element_iterator_destroy(&iterator);
		Cmiss_node_template_destroy(&node_template);
		Cmiss_field_cache_destroy(&field_cache);
		Cmiss_field_module_end_change(field_module);
		Cmiss_field_module_destroy(&field_module);
		delete[] gauss_locations;
		delete[] gauss_weights;
	}
	return return_code;
}
