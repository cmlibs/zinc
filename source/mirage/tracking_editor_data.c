/*******************************************************************************
FILE : tracking_editor_data.c

LAST MODIFIED : 15 April 1999

DESCRIPTION :
Contains operations performed on the tracking editor data structures.
==============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "general/compare.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/multi_range.h"
#include "general/object.h"
#include "mirage/tracking_editor_data.h"
#include "mirage/movie.h"
#include "user_interface/message.h"

/*
Module variables
----------------
*/

/*
Global variables
----------------
*/

/*
Global types
------------
*/

struct Node_status
/*******************************************************************************
LAST MODIFIED : 23 March 1998

DESCRIPTION :
The status of a node that is being tracked.
==============================================================================*/
{
	int node_no;
	struct Multi_range *ranges;
	int access_count;
}; /* Node_status */

FULL_DECLARE_INDEXED_LIST_TYPE(Node_status);

/*
Module functions
----------------
*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Node_status, \
	node_no,int,compare_int)

/*
Global functions
----------------
*/

struct Node_status *CREATE(Node_status)(int node_no)
/*******************************************************************************
LAST MODIFIED : 23 March 1998

DESCRIPTION :
Returns a new status range structure for following the tracking status of
node node_no.
==============================================================================*/
{
	struct Node_status *node_status;

	ENTER(CREATE(Node_status));
	/* allocate memory for structure */
	if (ALLOCATE(node_status,struct Node_status,1)&&
		(node_status->ranges=CREATE(Multi_range)()))
	{
		node_status->node_no=node_no;
		node_status->access_count=0;
	}
	else
	{
		if (node_status)
		{
			DEALLOCATE(node_status);
		}
		display_message(ERROR_MESSAGE,
			"CREATE(Node_status).  Not enough memory");
	}
	LEAVE;

	return (node_status);
} /* CREATE(Node_status) */

int DESTROY(Node_status)(struct Node_status **node_status_address)
/*******************************************************************************
LAST MODIFIED : 1 September 2000

DESCRIPTION :
Frees the memory for the node_status_ranges and sets <*node_status_address>
to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Node_status));
	if (node_status_address && *node_status_address)
	{
		if (0==(*node_status_address)->access_count)
		{
			DESTROY(Multi_range)(&((*node_status_address)->ranges));
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Node_status).  Non-zero access count!");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Node_status).  Missing address");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Node_status) */

DECLARE_OBJECT_FUNCTIONS(Node_status)
DECLARE_INDEXED_LIST_FUNCTIONS(Node_status)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Node_status, \
	node_no,int,compare_int)

int Node_status_add(struct Node_status *node_status,
	struct Node_status *node_status_to_add)
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
Adds the ranges in the second Node_status to the first.
==============================================================================*/
{
	int return_code,number_of_ranges,range_no,start,stop;

	ENTER(Node_status_add);
	if (node_status&&node_status_to_add)
	{
		number_of_ranges=
			Multi_range_get_number_of_ranges(node_status_to_add->ranges);
		return_code=1;
		for (range_no=0;return_code&&(range_no<number_of_ranges);range_no++)
		{
			return_code=Multi_range_get_range(node_status_to_add->ranges,
				range_no,&start,&stop)&&
				Multi_range_add_range(node_status->ranges,start,stop);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Node_status_add.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_add */

int Node_status_add_to_list(struct Node_status *node_status,
	void *node_status_list_void)
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
Iterator function for adding the ranges in node_status to those for the same
node_no in node_status_list.
==============================================================================*/
{
	int return_code;
	struct Node_status *node_status_to_modify;
	struct LIST(Node_status) *node_status_list;

	ENTER(Node_status_add_to_list);
	if (node_status&&(node_status_list=(struct LIST(Node_status) *)
		node_status_list_void))
	{
		if (node_status_to_modify=FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
			node_status->node_no,node_status_list))
		{
			return_code=Node_status_add(node_status_to_modify,node_status);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Node_status_add_to_list.  "
				"Could not find status of node %d",node_status->node_no);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_add_to_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_add_to_list */

struct Node_status_modify_at_value_data
{
	struct LIST(Node_status) *node_status_list_to_modify;
	int value;
	/* adding range if the following is set, otherwise removing */
	int adding;
};

static int Node_status_modify_at_value_to_list(struct Node_status *node_status,
	void *modify_data_void)
/*******************************************************************************
LAST MODIFIED : 10 April 1998

DESCRIPTION :
Iterator function for modifying the ranges in node_status to those for the same
node_no in node_status_list - but only at the single value.
The modification is either adding or removing.
Used to update the tracking editor bar chart during processing.
Note that only members common to both the node_status_list_to_modify and the
list it is modified by may be changed; it is no error if they do not match up.
==============================================================================*/
{
	int return_code;
	struct Node_status *node_status_to_modify;
	struct Node_status_modify_at_value_data *modify_data;

	ENTER(Node_status_modify_at_value_to_list);
	if (node_status&&
		(modify_data=(struct Node_status_modify_at_value_data *)modify_data_void))
	{
		if (Multi_range_is_value_in_range(node_status->ranges,modify_data->value))
		{
			if (node_status_to_modify=FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
				node_status->node_no,modify_data->node_status_list_to_modify))
			{
				if (modify_data->adding)
				{
					return_code=Multi_range_add_range(node_status_to_modify->ranges,
						modify_data->value,modify_data->value);
				}
				else
				{
					return_code=Multi_range_remove_range(node_status_to_modify->ranges,
						modify_data->value,modify_data->value);
				}
			}
			else
			{
				/* not an error - only modify if in both lists */
				return_code=1;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_modify_at_value_to_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_modify_at_value_to_list */

int Node_status_clear(struct Node_status *node_status,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 1 September 2000

DESCRIPTION :
Clears all ranges in the node_status.
==============================================================================*/
{
	int return_code;

	ENTER(Node_status_clear);
	USE_PARAMETER(dummy_void);
	if (node_status)
	{
		return_code=Multi_range_clear(node_status->ranges);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Node_status_clear.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_clear */

int Node_status_copy(struct Node_status *destination,
	struct Node_status *source)
/*******************************************************************************
LAST MODIFIED : 26 April 1998

DESCRIPTION :
Makes <destination> ranges an exact copy of those in <source>.
The node_no member of destination is not modified.
==============================================================================*/
{
	int return_code;

	ENTER(Node_status_copy);
	if (destination&&source)
	{
		return_code=Multi_range_copy(destination->ranges,source->ranges);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Node_status_copy.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_copy */

int Node_status_copy_to_list(struct Node_status *node_status,
	void *node_status_list_copy_void)
/*******************************************************************************
LAST MODIFIED : 26 April 1998

DESCRIPTION :
Puts a copy of <node_status> in <node_status_list_copy>, creating it if needed.
==============================================================================*/
{
	int return_code;
	struct Node_status *node_status_copy;
	struct LIST(Node_status) *node_status_list_copy;

	ENTER(Node_status_copy_to_list);
	if (node_status&&(node_status_list_copy=
		(struct LIST(Node_status) *)node_status_list_copy_void))
	{
		if (!(node_status_copy=FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
			node_status->node_no,node_status_list_copy)))
		{
			if (node_status_copy=CREATE(Node_status)(node_status->node_no))
			{
				if (!ADD_OBJECT_TO_LIST(Node_status)(node_status_copy,
					node_status_list_copy))
				{
					DESTROY(Node_status)(&node_status_copy);
				}
			}
		}
		if (node_status_copy)
		{
			return_code=
				Multi_range_copy(node_status_copy->ranges,node_status->ranges);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Node_status_copy_to_list.  Could not get copy");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_copy_to_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_copy_to_list */

int Node_status_get_node_no(struct Node_status *node_status)
/*******************************************************************************
LAST MODIFIED : 6 April 1998

DESCRIPTION :
Returns the node number of <node_status>.
==============================================================================*/
{
	int return_code;

	ENTER(Node_status_get_node_no);
	if (node_status)
	{
		return_code=node_status->node_no;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_get_node_no.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_get_node_no */

int Node_status_get_number_of_ranges(struct Node_status *node_status)
/*******************************************************************************
LAST MODIFIED : 3 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Node_status_get_number_of_ranges);
	if (node_status)
	{
		return_code=Multi_range_get_number_of_ranges(node_status->ranges);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_get_number_of_ranges.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_get_number_of_ranges */

int Node_status_get_range(struct Node_status *node_status,int range_no,
	int *start,int *stop)
/*******************************************************************************
LAST MODIFIED : 3 April 1998

DESCRIPTION :
Returns the start and stop values for range[range_no] in <node_status>.
Valid range numbers are from 0 to number_of_ranges-1.
==============================================================================*/
{
	int return_code;

	ENTER(Node_status_get_range);
	if (node_status&&start&&stop)
	{
		return_code=Multi_range_get_range(node_status->ranges,range_no,start,stop);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_get_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_get_range */

int Node_status_get_range_containing_value(struct Node_status *node_status,
	int value,int *start,int *stop)
/*******************************************************************************
LAST MODIFIED : 1 April 1998

DESCRIPTION :
Assuming value is in a range of the node_status, this function returns the
start and stop values for that range.
==============================================================================*/
{
	int return_code;

	ENTER(Node_status_get_range_containing_value);
	if (node_status&&Multi_range_is_value_in_range(node_status->ranges,value)&&
		start&&stop)
	{
		return_code=
			Multi_range_get_last_start_value(node_status->ranges,value+1,start)&&
			Multi_range_get_next_stop_value(node_status->ranges,value-1,stop);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_get_range_containing_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_get_range_containing_value */

int Node_status_get_last_start_value(struct Node_status *node_status,
	int value,int *last_start_value)
/*******************************************************************************
LAST MODIFIED : 10 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Node_status_get_last_start_value);
	if (node_status&&last_start_value)
	{
		return_code=Multi_range_get_last_start_value(node_status->ranges,
			value,last_start_value);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_get_last_start_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_get_last_start_value */

int Node_status_get_last_stop_value(struct Node_status *node_status,
	int value,int *last_stop_value)
/*******************************************************************************
LAST MODIFIED : 2 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Node_status_get_last_stop_value);
	if (node_status&&last_stop_value)
	{
		return_code=Multi_range_get_last_stop_value(node_status->ranges,
			value,last_stop_value);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_get_last_stop_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_get_last_stop_value */

int Node_status_get_next_start_value(struct Node_status *node_status,
	int value,int *next_start_value)
/*******************************************************************************
LAST MODIFIED : 2 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Node_status_get_next_start_value);
	if (node_status&&next_start_value)
	{
		return_code=Multi_range_get_next_start_value(node_status->ranges,
			value,next_start_value);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_get_next_start_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_get_next_start_value */

int Node_status_get_next_stop_value(struct Node_status *node_status,
	int value,int *next_stop_value)
/*******************************************************************************
LAST MODIFIED : 10 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Node_status_get_next_stop_value);
	if (node_status&&next_stop_value)
	{
		return_code=Multi_range_get_next_stop_value(node_status->ranges,
			value,next_stop_value);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_get_next_stop_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_get_next_stop_value */

int Node_status_is_value_in_range(struct Node_status *node_status,int value)
/*******************************************************************************
LAST MODIFIED : 1 September 2000

DESCRIPTION :
Returns true if <value> is in any range in <node_status>.
==============================================================================*/
{
	int return_code;

	ENTER(Node_status_is_value_in_range);
	if (node_status)
	{
		return_code=
			Multi_range_is_value_in_range(node_status->ranges,value);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_is_value_in_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_is_value_in_range */

int Node_status_not_clear(struct Node_status *node_status,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 1 September 2000

DESCRIPTION :
Iterator function returning true if there are any ranges defined for this
node_status.
==============================================================================*/
{
	int return_code;

	ENTER(Node_status_clear);
	USE_PARAMETER(dummy_void);
	if (node_status)
	{
		return_code=(0<Multi_range_get_number_of_ranges(node_status->ranges));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_not_clear.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_clear */

int Node_status_subtract(struct Node_status *node_status,
	struct Node_status *node_status_to_add)
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
Removes the ranges in the second Node_status from the first.
==============================================================================*/
{
	int return_code,number_of_ranges,range_no,start,stop;

	ENTER(Node_status_subtract);
	if (node_status&&node_status_to_add)
	{
		number_of_ranges=
			Multi_range_get_number_of_ranges(node_status_to_add->ranges);
		return_code=1;
		for (range_no=0;return_code&&(range_no<number_of_ranges);range_no++)
		{
			return_code=Multi_range_get_range(node_status_to_add->ranges,
				range_no,&start,&stop)&&
				Multi_range_remove_range(node_status->ranges,start,stop);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Node_status_subtract.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_subtract */

int Node_status_subtract_from_list(struct Node_status *node_status,
	void *node_status_list_void)
/*******************************************************************************
LAST MODIFIED : 1 September 2000

DESCRIPTION :
Iterator function for removing the ranges in node_status from those for the
same node_no in node_status_list.
==============================================================================*/
{
	int return_code;
	struct Node_status *node_status_to_modify;
	struct LIST(Node_status) *node_status_list;

	ENTER(Node_status_subtract_from_list);
	if (node_status&&(node_status_list=(struct LIST(Node_status) *)
		node_status_list_void))
	{
		if (node_status_to_modify=FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
			node_status->node_no,node_status_list))
		{
			return_code=Node_status_subtract(node_status_to_modify,node_status);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Node_status_subtract_from_list.  "
				"Could not find status of node %d",node_status->node_no);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_subtract_from_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_subtract_from_list */

int Node_status_add_range(struct Node_status *node_status,int start,int stop)
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
Adds the range from start to stop to node_status.
==============================================================================*/
{
	int return_code;

	ENTER(Node_status_add_range);
	if (node_status&&(stop >= start))
	{
		return_code=Multi_range_add_range(node_status->ranges,start,stop);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_add_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_add_range */

int Node_status_remove_range(struct Node_status *node_status,int start,int stop)
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
Removes the range from start to stop from node_status.
==============================================================================*/
{
	int return_code;

	ENTER(Node_status_remove_range);
	if (node_status&&(stop >= start))
	{
		return_code=Multi_range_remove_range(node_status->ranges,start,stop);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_remove_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_remove_range */

int Node_status_remove_range_iterator(struct Node_status *node_status,
	void *single_range_void)
/*******************************************************************************
LAST MODIFIED : 1 September 2000

DESCRIPTION :
Removes from <node_status> the range in <single_range>, a struct Single_range *.
==============================================================================*/
{
	int return_code;
	struct Single_range *single_range;

	ENTER(Node_status_remove_range);
	if (node_status&&(single_range=(struct Single_range *)single_range_void))
	{
		return_code = Node_status_remove_range(node_status,
			single_range->start,single_range->stop);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_remove_range.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_remove_range */

struct Node_status_write_data
{
	FILE *out_file;
	char *line_format;
};

int Node_status_write(struct Node_status *node_status,
	void *node_status_write_data_void)
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
For each range write the node_no, start and stop with the supplied format.
==============================================================================*/
{
	int return_code,number_of_ranges,range_no,start,stop;
	struct Node_status_write_data *write_data;

	ENTER(Node_status_write);
	if (node_status&&
		(write_data=(struct Node_status_write_data *)node_status_write_data_void))
	{
		number_of_ranges=Multi_range_get_number_of_ranges(node_status->ranges);
		return_code=1;
		for (range_no=0;return_code&&(range_no<number_of_ranges);range_no++)
		{
			if (return_code=Multi_range_get_range(node_status->ranges,
				range_no,&start,&stop))
			{
				fprintf(write_data->out_file,write_data->line_format,
					node_status->node_no,start,stop);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Node_status_write.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_write */

int Node_status_list_add(struct LIST(Node_status) *node_status_list,
	struct LIST(Node_status) *node_status_list_to_add)
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
Adds the ranges in the second node_status_list to those in the first.
==============================================================================*/
{
	int return_code;

	ENTER(Node_status_list_add);
	if (node_status_list&&node_status_list_to_add)
	{
		return_code=FOR_EACH_OBJECT_IN_LIST(Node_status)(
			Node_status_add_to_list,(void *)node_status_list,node_status_list_to_add);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_list_add.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_list_add */

int Node_status_list_add_at_value(struct LIST(Node_status) *node_status_list,
	struct LIST(Node_status) *node_status_list_to_add,int value)
/*******************************************************************************
LAST MODIFIED : 10 April 1998

DESCRIPTION :
Adds the ranges in the second node_status_list to those in the first, but only
at the single specified value.
==============================================================================*/
{
	int return_code;
	struct Node_status_modify_at_value_data add_data;

	ENTER(Node_status_list_add_at_value);
	if (node_status_list&&node_status_list_to_add)
	{
		add_data.node_status_list_to_modify=node_status_list;
		add_data.value=value;
		add_data.adding=1;
		return_code=FOR_EACH_OBJECT_IN_LIST(Node_status)(
			Node_status_modify_at_value_to_list,(void *)&add_data,
			node_status_list_to_add);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_list_add_at_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_list_add_at_value */

int Node_status_list_clear(struct LIST(Node_status) *node_status_list)
/*******************************************************************************
LAST MODIFIED : 25 March 1998

DESCRIPTION :
Clears all ranges for all nodes in the list.
==============================================================================*/
{
	int return_code;

	ENTER(Node_status_list_clear);
	if (node_status_list)
	{
		return_code=FOR_EACH_OBJECT_IN_LIST(Node_status)(Node_status_clear,
			(void *)NULL,node_status_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_list_clear.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_list_clear */

struct LIST(Node_status) *Node_status_list_duplicate(
	struct LIST(Node_status) *node_status_list)
/*******************************************************************************
LAST MODIFIED : 1 September 2000

DESCRIPTION :
Returns a new list containing an exact copy of <node_status_list>.
==============================================================================*/
{
	struct LIST(Node_status) *node_status_list_copy;

	ENTER(Node_status_list_duplicate);
	if (node_status_list)
	{
		if (node_status_list_copy=CREATE(LIST(Node_status))())
		{
			if (!FOR_EACH_OBJECT_IN_LIST(Node_status)(Node_status_copy_to_list,
				(void *)node_status_list_copy,node_status_list))
			{
				display_message(ERROR_MESSAGE,
					"Node_status_list_duplicate.  Could not copy list");
				DESTROY(LIST(Node_status))(&node_status_list_copy);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Node_status_list_duplicate.  Could not create copy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_list_duplicate.  Invalid argument(s)");
		node_status_list_copy=(struct LIST(Node_status) *)NULL;
	}
	LEAVE;

	return (node_status_list_copy);
} /* Node_status_list_duplicate */

int Node_status_list_subtract(struct LIST(Node_status) *node_status_list,
	struct LIST(Node_status) *node_status_list_to_subtract)
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
Subtracts the ranges in the second node_status_list from those in the first.
==============================================================================*/
{
	int return_code;

	ENTER(Node_status_list_subtract);
	if (node_status_list&&node_status_list_to_subtract)
	{
		return_code=FOR_EACH_OBJECT_IN_LIST(Node_status)(
			Node_status_subtract_from_list,(void *)node_status_list,
			node_status_list_to_subtract);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_list_subtract.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_list_subtract */

int Node_status_list_subtract_at_value(
	struct LIST(Node_status) *node_status_list,
	struct LIST(Node_status) *node_status_list_to_subtract,int value)
/*******************************************************************************
LAST MODIFIED : 10 April 1998

DESCRIPTION :
Subtracts the ranges in the second node_status_list from those in the first, but
only at the single specified value.
==============================================================================*/
{
	int return_code;
	struct Node_status_modify_at_value_data subtract_data;

	ENTER(Node_status_list_subtract_at_value);
	if (node_status_list&&node_status_list_to_subtract)
	{
		subtract_data.node_status_list_to_modify=node_status_list;
		subtract_data.value=value;
		subtract_data.adding=0;
		return_code=FOR_EACH_OBJECT_IN_LIST(Node_status)(
			Node_status_modify_at_value_to_list,(void *)&subtract_data,
			node_status_list_to_subtract);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_list_subtract_at_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_list_subtract_at_value */

int Node_status_list_read(struct LIST(Node_status) *node_status_list,
	char *file_name)
/*******************************************************************************
LAST MODIFIED : 26 March 1998

DESCRIPTION :
Reads the lines containing node_no, start and stop from file_name into the
node_status_list.
==============================================================================*/
{
	char type,buff[BUFSIZ];
	FILE *in_file;
	int return_code,node_no,start,stop,end_of_file;
	struct Node_status *node_status;

	ENTER(Node_status_list_read);
	if (node_status_list&&file_name)
	{
		if (in_file=fopen(file_name,"r"))
		{
			return_code=1;
			end_of_file=0;
			while (return_code&&!end_of_file)
			{
				if (NULL==fgets(buff,BUFSIZ,in_file))
				{
					end_of_file=1;
				}
				else
				{
					if ((buff[0] != '#')&&(buff[0] != '!'))
					{
#if defined (OLD_CODE)
						int view_no;
						if (5==sscanf(buff,"%c %d %d %d %d",&type,&node_no,&view_no,
							&start,&stop))
#endif /* defined (OLD_CODE) */
						if (4==sscanf(buff,"%c %d %d %d",&type,&node_no,&start,&stop))
						{
							if (node_status=FIND_BY_IDENTIFIER_IN_LIST(Node_status,node_no)(
								node_no,node_status_list))
							{
								return_code=Node_status_add_range(node_status,start,stop);
							}
							else
							{
								display_message(ERROR_MESSAGE,"Node_status_list_read.  "
									"Could not find information for node %d",node_no);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Node_status_list_read.  Incomplete line of data in file");
							return_code=0;
						}
					}
				}
			}
			fclose(in_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Node_status_list_read.  Could not read file '%s'",file_name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_list_read.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_list_read */

int Node_status_list_write(struct LIST(Node_status) *node_status_list,
	char *file_name,char *header_text,char *line_format)
/*******************************************************************************
LAST MODIFIED : 26 March 1998

DESCRIPTION :
For each range of each node write the node_no, start and stop with the supplied
line format. Also write the header_text at the top of the file.
==============================================================================*/
{
	int return_code;
	struct Node_status_write_data write_data;

	ENTER(Node_status_list_write);
	if (node_status_list&&file_name&&header_text&&line_format)
	{
		if (write_data.out_file=fopen(file_name,"w"))
		{
			fprintf(write_data.out_file,header_text);
			write_data.line_format=line_format;
			return_code=FOR_EACH_OBJECT_IN_LIST(Node_status)(Node_status_write,
				(void *)&write_data,node_status_list);
			fclose(write_data.out_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Node_status_list_write.  Could not create file '%s'",file_name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_status_list_write.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_status_list_write */

#if defined (OLD_CODE)
int tracking_editor_execute(enum Operation_type operation)
/*******************************************************************************
LAST MODIFIED : 28 February 1998

DESCRIPTION :
Execute the operation upon the data in pending. Then transfer the items in
pending to history. Returns 0 on sucess, and the error code on failure.
==============================================================================*/
{
	int error_code;
	char buff[BUFSIZ];
	int i;
	struct Track_element_array *tracking_array;

	ENTER(tracking_editor_execute);

	/* first check to see that we have some data to work with */

	if (pending->n_elem > 0)
	{

		if (our_movie != NULL)
		{

			/* set the operation type to the pending data: this so it is correct
				when transfered across to the history array. */

			for (i=0;i<pending->n_elem;i++)
			{
				pending->elem[i].operation = operation;
			}
			(void) write_track_element_array(history_file,history,"a");

			/* now copy across the pending data */

			track_element_array_copy_append(pending,history);

			/* do the exe stuff here */

			switch (operation)
			{
				case XVG:
				{
					char *req_filename,*bad_filename;

					if (ALLOCATE(req_filename,char,strlen(our_movie->name)+5)&&
						ALLOCATE(bad_filename,char,strlen(our_movie->name)+5))
					{
						sprintf(req_filename,"%s_req",our_movie->name);
						sprintf(bad_filename,"%s_bad",our_movie->name);

						write_xvg_request_file_new(req_filename,pending);

						sprintf(buff,"runxvg  %s %s %s +1",
							req_filename,our_movie->name,bad_filename);

						fprintf(stderr,
							"\ntracking_editor_execute:\n\n running ... \"%s\"\n\n\n",buff);

						/* Run the xvg script */

						if (error_code = system(buff))
						{
							display_message(ERROR_MESSAGE,"tracking_editor_execute.  "
								"error executing runxvg: return code %d",error_code);
						}
						else
						{
#if defined (OLD_CODE)
							/* transfer the items from the pending list to the history list */
							for (i=0;i<n_items;i++)
							{
								int view,node,start,stop;

								tracking_editor_get_value_at_index(&view,&node,&start,&stop,i,
									PENDING);
								write_value_to_list(tracking_editor_dialog->history_list,
									0,view,node,start,stop,operation);
							}
							XmListSelectPos(tracking_editor_dialog->history_list,0,True);
							XmListSetBottomPos(tracking_editor_dialog->history_list,0);
#endif /* defined (OLD_CODE) */
							/* should update the placed lists here */
							/* clear the pending list */
							tracking_editor_delete_all(PENDING);
							tracking_editor_update_list(PENDING);

							/* get the new bad node list */
							tracking_editor_delete_all(PROBLEM);
							tracking_array=tracking_editor_get_array_from_name(PROBLEM);
							read_xvg_request_file_new(bad_filename,tracking_array);
							printf("\nSORTING OUT THE BAD %i\n\n",tracking_array->n_elem);
							tracking_editor_update_list(PROBLEM);
							for (i=0;i<tracking_array->n_elem;i++)
							{
								printf("BAD %i Node %d Start %d Stop %d\n",i,
									tracking_array->elem[i].node,
									tracking_array->elem[i].start,
									tracking_array->elem[i].stop);
							}
						}
						write_comment(history_file,buff);
						free(req_filename);
						free(bad_filename);
					}
				}	break;

				/* SVD: we run the svdreq script, which uses edbasis, svd, rotate and
								translate.
					???: The svd basis file is hard coded as svd.basis, and the
								rotation control node file is hard coded as rot.exnode.
								These values should be read in from somewhere */

				case SVD:
				{
					char tmpfile[] = "/tmp/svdreq_request_XXXXXX";
					char *tmp_request = mktemp(tmpfile);

					write_xvg_request_file(tmp_request,pending);

					sprintf(buff,"svdreq svd.basis rot.exnode %s %s -overwrite",
						tmp_request,our_movie->node_file_name_template);

					fprintf(stderr,"\n\n\n running ... \"%s\"\n\n\n",buff);

					/* Run the svd script */

					if (error_code = system(buff))
					{
						display_message(ERROR_MESSAGE,"tracking_editor_execute.  "
							"error executing svdreq: return code %d",error_code);
					}
					else
					{
#if defined (OLD_CODE)
						/* transfer the items from the pending list to the history list */
						for (i=0;i<n_items;i++)
						{
							int view,node,start,stop;

							tracking_editor_get_value_at_index(&view,&node,&start,&stop,i,
								PENDING);
							write_value_to_list(tracking_editor_dialog->history_list,
								0,view,node,start,stop,operation);
						}
						XmListSelectPos(tracking_editor_dialog->history_list,0,True);
						XmListSetBottomPos(tracking_editor_dialog->history_list,0);
#endif /* defined (OLD_CODE) */

						/* clear the pending list */
						tracking_editor_delete_all(PENDING);
						tracking_editor_update_list(PENDING);

						/* clear the bad node list */
						tracking_editor_delete_all(PROBLEM);
						tracking_editor_update_list(PROBLEM);
					}
					write_comment(history_file,buff);
				} break;

				/* Linear interpolation: we run the lininterp executable. */
				case LINEAR_INTERPOLATION:
				{
					char tmpfile[] = "/tmp/lininterp_request_XXXXXX";
					char *tmp_request = mktemp(tmpfile);

					write_xvg_request_file(tmp_request,pending);

					sprintf(buff,"lininterp -r %s -b %s",tmp_request,
						our_movie->node_file_name_template);

					fprintf(stderr,"\n\n\n running ... \"%s\"\n\n\n",buff);

					if (error_code = system(buff))
					{
						display_message(ERROR_MESSAGE,"tracking_editor_execute.  "
							"error executing lininterp: return code %d",error_code);
					}
					else
					{
#if defined (OLD_CODE)
						/* transfer the items from the pending list to the history list */
						for (i=0;i<n_items;i++)
						{
							int view,node,start,stop;

							tracking_editor_get_value_at_index(&view,&node,&start,&stop,i,
								PENDING);
							write_value_to_list(tracking_editor_dialog->history_list,
								0,view,node,start,stop,operation);
						}
						XmListSelectPos(tracking_editor_dialog->history_list,0,True);
						XmListSetBottomPos(tracking_editor_dialog->history_list,0);
#endif /* defined (OLD_CODE) */

						/* clear the pending list */
						tracking_editor_delete_all(PENDING);
						tracking_editor_update_list(PENDING);

						/* clear the bad node list */
						tracking_editor_delete_all(PROBLEM);
						tracking_editor_update_list(PROBLEM);
					}
					write_comment(history_file,buff);
				} break;

				default:
				{
					display_message(ERROR_MESSAGE,"tracking_editor_execute.  "
						"unknown operation type");
				} break;
			}
			write_comment(history_file,"done!");
		}
		else
		{
			display_message(ERROR_MESSAGE,"tracking_editor_execute.  no movie data");
		}
	}
	LEAVE;

	return error_code;
} /* tracking_editor_execute */

#endif /* defined (OLD_CODE) */
