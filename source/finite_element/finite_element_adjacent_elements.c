/*******************************************************************************
FILE : finite_element_adjacent_elements.c

LAST MODIFIED : 1 November 2000

DESCRIPTION :
Functions for finding elements adjacent to other ones.  These functions have
been separated out from finite_element.c due to their dependence on 
indexed_multi_range.
==============================================================================*/

#include "general/debug.h"
#include "general/indexed_multi_range.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_adjacent_elements.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Adjacent_element_data
{
	struct FE_element *element;
	int number_of_adjacent_elements;
	struct FE_element **adjacent_elements;
};

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

static int FE_element_parent_adjacent_elements(struct FE_element_parent *parent,
	void *adjacent_element_data_void)
/*******************************************************************************
LAST MODIFIED : 27 October 2000

DESCRIPTION :
Adds the element to the adjacent_elements array in the data if the 
<parent> element is not the element.
==============================================================================*/
{
	int return_code;
	struct Adjacent_element_data *data;
	struct FE_element **new_array;

	ENTER(Adjacent_elements_not_element);
	if (parent&&(data = (struct Adjacent_element_data *)adjacent_element_data_void)
		&& data->element)
	{
		return_code = 1;
		if (parent->parent!=data->element)
		{
			if (REALLOCATE(new_array, data->adjacent_elements,
				struct FE_element *, data->number_of_adjacent_elements + 1))
			{
				new_array[data->number_of_adjacent_elements] = parent->parent;
				data->adjacent_elements = new_array;
				data->number_of_adjacent_elements++;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Adjacent_elements_not_element.  Unable to reallocate array");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Adjacent_elements_not_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Adjacent_elements_not_element */

#if defined (OLD_CODE)
static int FE_element_has_node_but_is_not_element(struct FE_element *element,
	void *find_element_data_void)
/*******************************************************************************
LAST MODIFIED : 31 October 2000

DESCRIPTION :
Adds the element to the elements array in the data if the 
element is not the exclude_element but does refer to the supplied node.
==============================================================================*/
{
	int node_index, number_of_nodes, return_code;
	struct Find_element_data *data;
	struct FE_element **new_array;

	ENTER(FE_element_has_node);
	if (element&&(data = (struct Find_element_data *)find_element_data_void)
		&& data->node)
	{
		return_code = 1;
		if (element!=data->exclude_element)
		{
			number_of_nodes = element->information->number_of_nodes;
			for (node_index = 0 ; node_index < number_of_nodes ; node_index++)
			{
				if (element->information->nodes[node_index] == data->node)
				{
					if (REALLOCATE(new_array, data->elements,
						struct FE_element *, data->number_of_elements + 1))
					{
						new_array[data->number_of_elements] = element;
						data->elements = new_array;
						data->number_of_elements++;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"FE_element_has_node.  Unable to reallocate array");
						return_code=0;
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_has_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_has_node */
#endif /* defined (OLD_CODE) */

static int FE_element_add_nodes_to_node_element_list(struct FE_element *element,
	void *node_element_list_void)
/*******************************************************************************
LAST MODIFIED : 1 November 2000

DESCRIPTION :
Adds the references to the <element> to each node index belonging to each node
the <element> has.
==============================================================================*/
{
	struct Index_multi_range *node_elements;
	struct LIST(Index_multi_range) *list;
	int element_number, node_index, node_number, number_of_nodes, return_code;

	ENTER(FE_element_add_nodes_to_node_element_list);
	if (element&&(list = (struct LIST(Index_multi_range) *)node_element_list_void))
	{
		return_code = 1;
		if (element->cm.type == CM_ELEMENT)
		{
			element_number = element->cm.number;
			number_of_nodes = element->information->number_of_nodes;
			for (node_index = 0 ; return_code && (node_index < number_of_nodes) ; 
				  node_index++)
			{
				node_number = get_FE_node_cm_node_identifier(element->information->
					nodes[node_index]);
				if (node_elements=FIND_BY_IDENTIFIER_IN_LIST(Index_multi_range,index_number)
					(node_number,list))
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

int adjacent_FE_element(struct FE_element *element,
	int face_number, int *number_of_adjacent_elements, 
	struct FE_element ***adjacent_elements)
/*******************************************************************************
LAST MODIFIED : 27 October 2000

DESCRIPTION :
Returns the list of <adjacent_elements> not including <element> which share the 
face indicated by <face_number>.  <adjacent_elements> is ALLOCATED to the 
correct size and should be DEALLOCATED when finished with.
==============================================================================*/
{
	int return_code;
	struct Adjacent_element_data data;

	ENTER(adjacent_FE_element);
	if (element&&(element->shape)&&(element->faces))
	{
		if ((0<=face_number)&&(face_number<element->shape->number_of_faces)&&
			 ((element->faces)[face_number]))
		{
			data.element = element;
			data.number_of_adjacent_elements = 0;
			data.adjacent_elements = (struct FE_element **)NULL;
			if (FOR_EACH_OBJECT_IN_LIST(FE_element_parent)(
				FE_element_parent_adjacent_elements,(void *)&data,
				((element->faces)[face_number])->parent_list))
			{
				if (data.number_of_adjacent_elements)
				{
					*adjacent_elements=data.adjacent_elements;
					*number_of_adjacent_elements = data.number_of_adjacent_elements;
					return_code = 1;
				}
				else
				{
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
			/* In the generic heart mesh there are elements that
				do not have faces defined this isn't an error */
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"adjacent_FE_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* adjacent_FE_element */

int adjacent_FE_element_from_nodes(struct FE_element *element,
	int node_index, int *number_of_adjacent_elements, 
	struct FE_element ***adjacent_elements, 
	struct LIST(Index_multi_range) *node_element_list,
	struct MANAGER(FE_element) *fe_element_manager)
/*******************************************************************************
LAST MODIFIED : 27 October 2000

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
	struct Index_multi_range *node_elements;

	ENTER(adjacent_FE_element_from_nodes);
	if (element&&node_element_list&&fe_element_manager)
	{
		return_code = 1;
		element_number = element->cm.number;
		if ((0<=node_index)&&(node_index<element->information->number_of_nodes)&&
			((element->information->nodes)[node_index]))
		{
			node_number = get_FE_node_cm_node_identifier(element->information->
				nodes[node_index]);			
			if (node_elements=FIND_BY_IDENTIFIER_IN_LIST(Index_multi_range,index_number)
				(node_number,node_element_list))
			{
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
										if (adjacent_element = FIND_BY_IDENTIFIER_IN_MANAGER(
											FE_element,identifier)(&element_id,fe_element_manager))
										{
											(*adjacent_elements)[i] = adjacent_element;
											i++;
										}
										else
										{
											display_message(ERROR_MESSAGE,"adjacent_FE_element_from_nodes.  "
												"Element %d not found in manager", j);
											return_code = 0;
										}
									}
								}
							}
						}
						*number_of_adjacent_elements = i;
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
   struct MANAGER(FE_element) *fe_element_manager)
/*******************************************************************************
LAST MODIFIED : 1 November 2000

DESCRIPTION :
Creates a list indexed by node identifying elements which refer to each node.
==============================================================================*/
{
	struct LIST(Index_multi_range) *list;

	ENTER(create_node_element_list);
	if (fe_element_manager)
	{
		list = CREATE(LIST(Index_multi_range))();
		if (!(FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
			FE_element_add_nodes_to_node_element_list, (void *)list, 
			fe_element_manager)))
		{
			DESTROY(LIST(Index_multi_range))(&list);
			list = (struct LIST(Index_multi_range) *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_node_element_list.  "
			"Invalid argument(s)");
		list = (struct LIST(Index_multi_range) *)NULL;
	}
	LEAVE;

	return (list);
} /* create_node_element_list */

