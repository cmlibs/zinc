/*******************************************************************************
FILE : finite_element_adjacent_elements.c

LAST MODIFIED : 13 March 2003

DESCRIPTION :
Functions for finding elements adjacent to other ones.  These functions have
been separated out from finite_element.c due to their dependence on 
indexed_multi_range.
==============================================================================*/

#include "general/debug.h"
#include "general/indexed_multi_range.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_adjacent_elements.h"
#include "finite_element/finite_element_region.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Find_element_data
{
	struct FE_node *node;
	struct FE_element *exclude_element;
	int number_of_elements;
	struct FE_element **elements;
};

/*
Module functions
----------------
*/

static int FE_element_add_nodes_to_node_element_list(struct FE_element *element,
	void *node_element_list_void)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Adds the references to the <element> to each node index belonging to each node
the <element> has.
==============================================================================*/
{
	int element_number, node_index, node_number, number_of_nodes, return_code;
	struct CM_element_information cm;
	struct FE_node *node;
	struct Index_multi_range *node_elements;
	struct LIST(Index_multi_range) *list;

	ENTER(FE_element_add_nodes_to_node_element_list);
	if (element &&
		(list = (struct LIST(Index_multi_range) *)node_element_list_void))
	{
		return_code = 1;
		if (get_FE_element_identifier(element, &cm) && (cm.type == CM_ELEMENT) &&
			get_FE_element_number_of_nodes(element, &number_of_nodes))
		{
			element_number = cm.number;
			for (node_index = 0 ; return_code && (node_index < number_of_nodes) ; 
				node_index++)
			{
				if (get_FE_element_node(element, node_index, &node) && node)
				{
					node_number = get_FE_node_identifier(node);
					if (node_elements = FIND_BY_IDENTIFIER_IN_LIST
						(Index_multi_range,index_number)(node_number,list))
					{
						return_code = Index_multi_range_add_range(node_elements,
							element_number, element_number);
					}
					else
					{
						if (!((node_elements=CREATE(Index_multi_range)(node_number))
							&& Index_multi_range_add_range(node_elements,
								element_number, element_number) && 
							ADD_OBJECT_TO_LIST(Index_multi_range)(node_elements, list)))
						{
							DESTROY(Index_multi_range)(&node_elements);
							return_code = 0;
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_add_nodes_to_node_element_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_add_nodes_to_node_element_list */

/*
Global functions
----------------
*/

int adjacent_FE_element_from_nodes(struct FE_element *element,
	int node_index, int *number_of_adjacent_elements, 
	struct FE_element ***adjacent_elements, 
	struct LIST(Index_multi_range) *node_element_list,
	struct FE_region *fe_region)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
For a 1D top level element this routine will return the list of 
<adjacent_elements> not including <element> which share the node indicated by
<node_index>.  <adjacent_elements> is ALLOCATED to the 
correct size and should be DEALLOCATED when calls to this function are finished.
==============================================================================*/
{
	int element_number, i, j, node_number, number_of_elements, number_of_ranges,
		range_no, return_code, start, stop;
	struct CM_element_information element_id;
	struct FE_element *adjacent_element;
	struct FE_node *node;
	struct Index_multi_range *node_elements;

	ENTER(adjacent_FE_element_from_nodes);
	if (element && node_element_list && fe_region)
	{
		return_code = 1;
		get_FE_element_identifier(element, &element_id);
		element_number = element_id.number;
		if (get_FE_element_node(element, node_index, &node) && node)
		{
			node_number = get_FE_node_identifier(node);
			if (node_elements = FIND_BY_IDENTIFIER_IN_LIST
				(Index_multi_range,index_number)(node_number, node_element_list))
			{
				/* This list includes the element itself so we are safe but probably
					overallocating the array */
				number_of_elements = Index_multi_range_get_total_number_in_ranges(
					node_elements);
				if (ALLOCATE(*adjacent_elements, struct FE_element *,
					number_of_elements))
				{
					element_id.type = CM_ELEMENT;
					i = 0;
					if (0<(number_of_ranges=Index_multi_range_get_number_of_ranges(
						node_elements)))
					{
						for (range_no=0;return_code&&(range_no<number_of_ranges);range_no++)
						{
							if (return_code=
								Index_multi_range_get_range(node_elements,range_no,&start,&stop))
							{
								for (j = start ; j <= stop ; j++)
								{
									if (j != element_number)
									{
										element_id.number = j;
										if (adjacent_element =
											FE_region_get_FE_element_from_identifier(fe_region,
												&element_id))
										{
											(*adjacent_elements)[i] = adjacent_element;
											i++;
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"adjacent_FE_element_from_nodes.  "
												"Element %d not found in region", j);
											return_code = 0;
										}
									}
								}
							}
						}
						*number_of_adjacent_elements = i;
					}
					if (i == 0)
					{
						/* Don't keep the array if there are no elements */
						DEALLOCATE(*adjacent_elements);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"adjacent_FE_element_from_nodes.  "
						"Unable to allocate element array");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"adjacent_FE_element_from_nodes.  "
					"No index object found for node %d", node_number);
				return_code = 0;
			}
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"adjacent_FE_element_from_nodes.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* adjacent_FE_element_from_nodes */

struct LIST(Index_multi_range) *create_node_element_list(
	struct FE_region *fe_region)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Creates a list indexed by node identifying elements which refer to each node.
==============================================================================*/
{
	struct LIST(Index_multi_range) *list;

	ENTER(create_node_element_list);
	if (fe_region)
	{
		list = CREATE(LIST(Index_multi_range))();
		if (!(FE_region_for_each_FE_element(fe_region,
			FE_element_add_nodes_to_node_element_list, (void *)list)))
		{
			DESTROY(LIST(Index_multi_range))(&list);
			list = (struct LIST(Index_multi_range) *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_node_element_list.  Invalid argument(s)");
		list = (struct LIST(Index_multi_range) *)NULL;
	}
	LEAVE;

	return (list);
} /* create_node_element_list */

