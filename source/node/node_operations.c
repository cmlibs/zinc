/*******************************************************************************
FILE : node_operations.c

LAST MODIFIED : 17 January 2003

DESCRIPTION :
FE_node functions that utilise non finite element data structures and therefore
cannot reside in finite element modules.
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
#include <math.h>
#include "general/debug.h"
#include "node/node_operations.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

#if defined (OLD_CODE)
int destroy_listed_nodes(struct LIST(FE_node) *node_list,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(FE_element) *element_manager,
	struct FE_node_selection *node_selection)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Destroys all the nodes in <node_list> that are not accessed outside
<node_manager>, the groups in <node_group_manager> and <node_selection>.
Nodes in use by elements in the <element_manager> cannot be destroyed so are
immediately ruled out in order to keep them in the node groups and selection.
<node_group_manager> and <node_selection> are optional.
Upon return <node_list> contains all the nodes that could not be destroyed.
==============================================================================*/
{
	int number_of_nodes_destroyed,number_of_nodes_not_destroyed,return_code;
	struct FE_node *node;
	struct GROUP(FE_node) *node_group;
	struct LIST(FE_node) *not_destroyed_node_list;

	ENTER(destroy_listed_nodes);
	if (node_list&&node_manager)
	{
		return_code=1;
		/* build list of nodes that could be destroyed */
		not_destroyed_node_list=CREATE(LIST(FE_node))();
		if (element_manager)
		{
			COPY_LIST(FE_node)(not_destroyed_node_list,node_list);
			/* remove all nodes in use by elements = cannot be destroyed */
			FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
				ensure_top_level_FE_element_nodes_are_not_in_list,
				(void *)node_list,element_manager);
			/* remove nodes still in node_list from not_destroyed_node_list so
				 it lists only those that could not be destroyed */
			REMOVE_OBJECTS_FROM_LIST_THAT(FE_node)(
				FE_node_is_in_list,(void *)node_list,not_destroyed_node_list);
		}
		if (node_group_manager)
		{
			/* remove the nodes from all groups they are in */
			while (return_code&&(node_group=
				FIRST_OBJECT_IN_MANAGER_THAT(GROUP(FE_node))(
					FE_node_group_intersects_list,(void *)node_list,
					node_group_manager)))
			{
				MANAGED_GROUP_BEGIN_CACHE(FE_node)(node_group);
				if (!REMOVE_OBJECTS_FROM_GROUP_THAT(FE_node)(
					FE_node_is_in_list,(void *)node_list,node_group))
				{
					return_code=0;
				}
				MANAGED_GROUP_END_CACHE(FE_node)(node_group);
			}
		}
		if (node_selection)
		{
			/* remove nodes from the global node_selection */
			FE_node_selection_begin_cache(node_selection);
			FOR_EACH_OBJECT_IN_LIST(FE_node)(FE_node_unselect_in_FE_node_selection,
				(void *)node_selection,node_list);
			FE_node_selection_end_cache(node_selection);
		}
		/* now remove the nodes from the manager */
		MANAGER_BEGIN_CACHE(FE_node)(node_manager);
		number_of_nodes_destroyed=0;
		while (return_code&&(node=FIRST_OBJECT_IN_LIST_THAT(FE_node)(
			(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,node_list)))
		{
			/* node cannot be destroyed while it is in a list */
			if (REMOVE_OBJECT_FROM_LIST(FE_node)(node, node_list))
			{
				if (MANAGED_OBJECT_NOT_IN_USE(FE_node)(node, node_manager))
				{
					if (REMOVE_OBJECT_FROM_MANAGER(FE_node)(node, node_manager))
					{
						number_of_nodes_destroyed++;
					}
					else
					{
						return_code = 0;
					}
				}
				else
				{
					/* add it to not_destroyed_node_list for reporting */
					ADD_OBJECT_TO_LIST(FE_node)(node, not_destroyed_node_list);
				}
			}
			else
			{
				return_code = 0;
			}
		}
		MANAGER_END_CACHE(FE_node)(node_manager);
		if (0<(number_of_nodes_not_destroyed=
			NUMBER_IN_LIST(FE_node)(not_destroyed_node_list)))
		{
			display_message(WARNING_MESSAGE,"%d node(s) destroyed; "
				"%d node(s) could not be destroyed because in use",
				number_of_nodes_destroyed,number_of_nodes_not_destroyed);
			return_code=0;
		}
		FOR_EACH_OBJECT_IN_LIST(FE_node)(ensure_FE_node_is_in_list,
			(void *)node_list,not_destroyed_node_list);
		DESTROY(LIST(FE_node))(&not_destroyed_node_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,"destroy_listed_nodes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* destroy_listed_nodes */
#endif /* defined (OLD_CODE) */

struct FE_node_fe_region_selection_ranges_condition_data
/*******************************************************************************
LAST MODIFIED : 15 May 2006

DESCRIPTION :
==============================================================================*/
{
	struct FE_region *fe_region;
	int selected_flag;
	struct LIST(FE_node) *node_selection_list;
	struct Multi_range *node_ranges;
	struct Computed_field *conditional_field;
	FE_value conditional_field_time;
	struct LIST(FE_node) *node_list;
}; /* struct FE_node_fe_region_selection_ranges_condition_data */

static int FE_node_add_if_selection_ranges_condition(struct FE_node *node,
	void *data_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code, selected;
	struct FE_node_fe_region_selection_ranges_condition_data *data;

	ENTER(FE_node_fe_region_ranges_condition);
	if (node && 
		(data = (struct FE_node_fe_region_selection_ranges_condition_data *)data_void))
	{
		return_code = 1;
		selected = 1;
		if (selected && data->selected_flag)
		{
			selected = IS_OBJECT_IN_LIST(FE_node)(node, data->node_selection_list);
		}
		if (selected && data->node_ranges)
		{
			selected = FE_node_is_in_Multi_range(node, data->node_ranges);
		}
		if (selected && data->conditional_field)
		{
			selected = Computed_field_is_true_at_node(data->conditional_field,
				node, data->conditional_field_time);
		}
		if (selected)
		{
			return_code = ADD_OBJECT_TO_LIST(FE_node)(node, data->node_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_set_FE_node_field_info.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_set_FE_node_field_info */

static int FE_node_add_if_fe_region_ranges_condition(struct FE_node *node,
	void *data_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code, selected;
	struct FE_node_fe_region_selection_ranges_condition_data *data;

	ENTER(FE_node_fe_region_ranges_condition);
	if (node && 
		(data = (struct FE_node_fe_region_selection_ranges_condition_data *)data_void))
	{
		return_code = 1;
		selected = FE_region_contains_FE_node(data->fe_region, node);
		if (selected && data->node_ranges)
		{
			selected = FE_node_is_in_Multi_range(node, data->node_ranges);
		}
		if (selected && data->conditional_field)
		{
			selected = Computed_field_is_true_at_node(data->conditional_field,
				node, data->conditional_field_time);
		}
		if (selected)
		{
			return_code = ADD_OBJECT_TO_LIST(FE_node)(node, data->node_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_set_FE_node_field_info.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_set_FE_node_field_info */

struct LIST(FE_node) *
	FE_node_list_from_fe_region_selection_ranges_condition(
		struct FE_region *fe_region, struct FE_node_selection *node_selection,
		int selected_flag, struct Multi_range *node_ranges,
		struct Computed_field *conditional_field, FE_value time)
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Creates and returns an node list that is the intersection of:
- all the nodes in <fe_region>;
- all nodes in the <node_selection> if <selected_flag> is set;
- all nodes in the given <node_ranges>, if any.
- all nodes for which the <conditional_field> evaluates as "true"
  in its centre at the specified <time>
Up to the calling function to destroy the returned node list.
==============================================================================*/
{
	int i, node_number, nodes_in_region, nodes_in_selection, nodes_in_ranges,
		number_of_ranges, return_code, selected, start, stop;
	struct FE_node *node;
	struct FE_node_fe_region_selection_ranges_condition_data data;

	ENTER(FE_node_list_from_fe_region_selection_ranges_condition);
	data.node_list = (struct LIST(FE_node) *)NULL;
	if (fe_region && ((!selected_flag) || node_selection))
	{
		if (data.node_list = CREATE(LIST(FE_node))())
		{
			nodes_in_region = FE_region_get_number_of_FE_nodes(fe_region);
			if (selected_flag)
			{
				nodes_in_selection = NUMBER_IN_LIST(FE_node)
					(FE_node_selection_get_node_list(node_selection));
			}
			if (node_ranges)
			{
				nodes_in_ranges = Multi_range_get_total_number_in_ranges(node_ranges);
			}

			data.fe_region = fe_region;
			data.selected_flag = selected_flag;
			data.node_selection_list = FE_node_selection_get_node_list(node_selection);
			/* Seems odd to specify an empty node_ranges but I have
				maintained the previous behaviour */
			if (data.node_ranges &&
				(0 < (number_of_ranges = Multi_range_get_number_of_ranges(node_ranges))))
			{
				data.node_ranges = node_ranges;
			}
			else
			{
				data.node_ranges = (struct Multi_range *)NULL;
			}
			data.conditional_field = conditional_field;
			data.conditional_field_time = time;

			if (data.node_ranges
				&& (nodes_in_ranges < nodes_in_region)
				&& (!selected_flag || (nodes_in_ranges < nodes_in_selection)))
			{
				return_code = 1;
				for (i = 0 ; i < number_of_ranges ; i++)
				{
					Multi_range_get_range(node_ranges, i, &start, &stop);
					for (node_number = start ; node_number <= stop ; node_number++)
					{
						if (node = FE_region_get_FE_node_from_identifier(
								 fe_region, node_number))
						{
							selected = 1;
							if (data.selected_flag)
							{
								if (!FIND_BY_IDENTIFIER_IN_LIST(FE_node,cm_node_identifier)
									(node_number, data.node_selection_list))
								{
									selected = 0;
								}
							}
							if (selected && conditional_field)
							{
								selected = Computed_field_is_true_at_node(conditional_field,
									node, time);
							}
							if (selected)
							{
								ADD_OBJECT_TO_LIST(FE_node)(node, data.node_list);
							}
						}
					}
				}
			}
			else if (selected_flag && (nodes_in_selection < nodes_in_region))
			{
				return_code = FOR_EACH_OBJECT_IN_LIST(FE_node)(
					FE_node_add_if_fe_region_ranges_condition,
					(void *)&data, data.node_selection_list);
			}
			else
			{
				return_code =  FE_region_for_each_FE_node(fe_region,
					FE_node_add_if_selection_ranges_condition, (void *)&data);
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"FE_node_list_from_fe_region_selection_ranges_condition.  "
					"Error building list");
				DESTROY(LIST(FE_node))(&data.node_list);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_node_list_from_fe_region_selection_ranges_condition.  "
				"Could not create list");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_list_from_fe_region_selection_ranges_condition.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (data.node_list);
} /* FE_node_list_from_fe_region_selection_ranges_condition */

struct FE_node_values_number
/*******************************************************************************
LAST MODIFIED : 22 December 2000

DESCRIPTION :
Data for changing node identifiers.
==============================================================================*/
{
	struct FE_node *node;
	int number_of_values;
	FE_value *values;
	int new_number;
};

static int compare_FE_node_values_number_values(
	const void *node_values1_void, const void *node_values2_void)
/*******************************************************************************
LAST MODIFIED : 22 December 2000

DESCRIPTION :
Compares the values in <node_values1> and <node_values2> from the last to the
first, returning -1 as soon as a value in <node_values1> is less than its
counterpart in <node_values2>, or 1 if greater. 0 is returned if all values
are identival. Used as a compare function for qsort.
==============================================================================*/
{
	int i, number_of_values, return_code;
	struct FE_node_values_number *node_values1, *node_values2;

	ENTER(compare_FE_node_values_number_values);
	return_code = 0;
	if ((node_values1 = (struct FE_node_values_number *)node_values1_void) &&
		(node_values2 = (struct FE_node_values_number *)node_values2_void) &&
		(0 < (number_of_values = node_values1->number_of_values)) &&
		(number_of_values == node_values2->number_of_values))
	{
		for (i = number_of_values - 1; (!return_code) && (0 <= i); i--)
		{
			if (node_values1->values[i] < node_values2->values[i])
			{
				return_code = -1;
			}
			else if (node_values1->values[i] > node_values2->values[i])
			{
				return_code = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"compare_FE_node_values_number_values.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* compare_FE_node_values_number_values */

struct FE_node_and_values_to_array_data
{
	FE_value time;
	struct FE_node_values_number *node_values;
	struct Computed_field *sort_by_field;
}; /* FE_node_and_values_to_array_data */

static int FE_node_and_values_to_array(struct FE_node *node,
	void *array_data_void)
/*******************************************************************************
LAST MODIFIED : 22 December 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct FE_node_and_values_to_array_data *array_data;

	ENTER(FE_node_and_values_to_array);
	if (node && (array_data =
		(struct FE_node_and_values_to_array_data *)array_data_void) &&
		array_data->node_values)
	{
		return_code = 1;
		array_data->node_values->node = node;
		if (array_data->sort_by_field)
		{
			if (!(array_data->node_values->values && Computed_field_evaluate_at_node(
				array_data->sort_by_field, node, array_data->time,
				array_data->node_values->values)))
			{
				display_message(ERROR_MESSAGE, "FE_node_and_values_to_array.  "
					"sort_by field could not be evaluated at node");
				return_code = 0;
			}
		}
		(array_data->node_values)++;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_and_values_to_array.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_and_values_to_array */

int FE_region_change_node_identifiers(struct FE_region *fe_region,
	int node_offset, struct Computed_field *sort_by_field, FE_value time)
/*******************************************************************************
LAST MODIFIED : 18 February 2003

DESCRIPTION :
Changes the identifiers of all nodes in <fe_region>.
If <sort_by_field> is NULL, adds <node_offset> to the identifiers.
If <sort_by_field> is specified, it is evaluated for all nodes
in <fe_region> and they are sorted by it - changing fastest with the first
component and keeping the current order where the field has the same values.
Checks for and fails if attempting to give any of the nodes in <fe_region> an
identifier already used by a node in the same master FE_region.
Calls to this function should be enclosed in FE_region_begin_change/end_change.
Note function avoids iterating through FE_region node lists as this is not
allowed during identifier changes.
==============================================================================*/
{
	int i, next_spare_node_number, number_of_nodes, number_of_values, return_code;
	struct FE_node *node_with_identifier;
	struct FE_node_and_values_to_array_data array_data;
	struct FE_node_values_number *node_values;
	struct FE_region *master_fe_region;

	ENTER(FE_region_change_node_identifiers);
	if (fe_region)
	{
		return_code = 1;
		number_of_nodes = FE_region_get_number_of_FE_nodes(fe_region);
		if (0 < number_of_nodes)
		{
			FE_region_get_ultimate_master_FE_region(fe_region, &master_fe_region);
			if (sort_by_field)
			{
				number_of_values =
					Computed_field_get_number_of_components(sort_by_field);
			}
			else
			{
				number_of_values = 0;
			}
			if (ALLOCATE(node_values, struct FE_node_values_number,
				number_of_nodes))
			{
				for (i = 0; i < number_of_nodes; i++)
				{
					node_values[i].number_of_values = number_of_values;
					node_values[i].values = (FE_value *)NULL;
				}
				if (sort_by_field)
				{
					for (i = 0; (i < number_of_nodes) && return_code; i++)
					{
						if (!ALLOCATE(node_values[i].values, FE_value, number_of_values))
						{
							display_message(ERROR_MESSAGE,
								"FE_region_change_node_identifiers.  "
								"Not enough memory");
							return_code = 0;
						}
					}
				}
				if (return_code)
				{
					/* make a linear array of the nodes in the group in current order */
					array_data.node_values = node_values;
					array_data.sort_by_field = sort_by_field;
					array_data.time = time;
					if (!FE_region_for_each_FE_node(fe_region,
						FE_node_and_values_to_array, (void *)&array_data))
					{
						display_message(ERROR_MESSAGE,
							"FE_region_change_node_identifiers.  "
							"Could not build node/field values array");
						return_code = 0;
					}
				}
				if (return_code)
				{
					if (sort_by_field)
					{
						/* sort by field values with higher components more significant */
						qsort(node_values, number_of_nodes,
							sizeof(struct FE_node_values_number),
							compare_FE_node_values_number_values);
						/* give the nodes sequential values starting at node_offset */
						for (i = 0; i < number_of_nodes; i++)
						{
							node_values[i].new_number = node_offset + i;
						}
					}
					else
					{
						/* offset node numbers by node_offset */
						for (i = 0; i < number_of_nodes; i++)
						{
							node_values[i].new_number =
								get_FE_node_identifier(node_values[i].node) + node_offset;
						}
					}
					/* check node numbers are positive and ascending */
					for (i = 0; (i < number_of_nodes) && return_code; i++)
					{
						if (0 >= node_values[i].new_number)
						{
							display_message(ERROR_MESSAGE,
								"FE_region_change_node_identifiers.  "
								"node_offset would give negative node numbers");
							return_code = 0;
						}
						else if ((0 < i) &&
							(node_values[i].new_number <= node_values[i - 1].new_number))
						{
							display_message(ERROR_MESSAGE,
								"FE_region_change_node_identifiers.  "
								"Node numbers are not strictly increasing");
							return_code = 0;
						}
					}
				}
				if (return_code)
				{
					/* check no new numbers are in use by nodes not in node_group */
					for (i = 0; (i < number_of_nodes) && return_code; i++)
					{
						if ((node_with_identifier = FE_region_get_FE_node_from_identifier(
							master_fe_region, node_values[i].new_number)) &&
							(!FE_region_contains_FE_node(fe_region, node_with_identifier)))
						{
							display_message(ERROR_MESSAGE,
								"FE_region_change_node_identifiers.  "
								"Node using new number exists in master region");
							return_code = 0;
						}
					}
				}
				if (return_code)
				{
					/* change identifiers */
					/* maintain next_spare_node_number to renumber nodes in same group
						 which already have the same number as the new_number */
					next_spare_node_number =
						node_values[number_of_nodes - 1].new_number + 1;
					for (i = 0; (i < number_of_nodes) && return_code; i++)
					{
						node_with_identifier = FE_region_get_FE_node_from_identifier(
							fe_region, node_values[i].new_number);
						/* only modify if node doesn't already have correct identifier */
						if (node_with_identifier != node_values[i].node)
						{
							if (node_with_identifier)
							{
								while ((struct FE_node *)NULL !=
									FE_region_get_FE_node_from_identifier(fe_region,
										next_spare_node_number))
								{
									next_spare_node_number++;
								}
								if (!FE_region_change_FE_node_identifier(master_fe_region,
									node_with_identifier, next_spare_node_number))
								{
									return_code = 0;
								}
							}
							if (!FE_region_change_FE_node_identifier(master_fe_region,
								node_values[i].node, node_values[i].new_number))
							{
								display_message(ERROR_MESSAGE,
									"FE_region_change_node_identifiers.  "
									"Could not change node identifier");
								return_code = 0;
							}
						}
					}
				}
				for (i = 0; i < number_of_nodes; i++)
				{
					if (node_values[i].values)
					{
						DEALLOCATE(node_values[i].values);
					}
				}
				DEALLOCATE(node_values);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_region_change_node_identifiers.  Not enough memory");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_region_change_node_identifiers.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_region_change_node_identifiers */

