/*******************************************************************************
FILE : tracking_editor_data.h

LAST MODIFIED : 26 April 1998

DESCRIPTION :
Contains data structures for the tracking editor, mainly the list
of retracks that are to be performed.
==============================================================================*/
#if !defined (TRACKING_EDITOR_DATA_H)
#define TRACKING_EDITOR_DATA_H

#include "general/list.h"
#include "general/object.h"

/*
Global types
------------
*/
struct Node_status;

DECLARE_LIST_TYPES(Node_status);

/*
Global functions
----------------
*/

struct Node_status *CREATE(Node_status)(int node_no);
/*******************************************************************************
LAST MODIFIED : 23 March 1998

DESCRIPTION :
Returns a new Node_status structure for following the tracking status of
node node_no.
==============================================================================*/

int DESTROY(Node_status)(struct Node_status **node_status_address);
/*******************************************************************************
LAST MODIFIED : 23 March 1998

DESCRIPTION :
Frees the memory for the node_status and sets <*node_status_address>
to NULL.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Node_status);

PROTOTYPE_LIST_FUNCTIONS(Node_status);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Node_status,node_no,int);

int Node_status_add(struct Node_status *node_status,
	struct Node_status *node_status_to_add);
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
Adds the ranges in the second Node_status to the first.
==============================================================================*/

int Node_status_clear(struct Node_status *node_status,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 25 March 1998

DESCRIPTION :
Clears all ranges in the node_status.
==============================================================================*/

int Node_status_copy(struct Node_status *destination,
	struct Node_status *source);
/*******************************************************************************
LAST MODIFIED : 26 April 1998

DESCRIPTION :
Makes <destination> ranges an exact copy of those in <source>.
The node_no member of destination is not modified.
==============================================================================*/

int Node_status_get_node_no(struct Node_status *node_status);
/*******************************************************************************
LAST MODIFIED : 6 April 1998

DESCRIPTION :
Returns the node number of <node_status>.
==============================================================================*/

int Node_status_get_number_of_ranges(struct Node_status *node_status);
/*******************************************************************************
LAST MODIFIED : 3 April 1998

DESCRIPTION :
==============================================================================*/

int Node_status_get_range(struct Node_status *node_status,int range_no,
	int *start,int *stop);
/*******************************************************************************
LAST MODIFIED : 3 April 1998

DESCRIPTION :
Returns the start and stop values for range[range_no] in <node_status>.
Valid range numbers are from 0 to number_of_ranges-1.
==============================================================================*/

int Node_status_get_range_containing_value(struct Node_status *node_status,
	int value,int *start,int *stop);
/*******************************************************************************
LAST MODIFIED : 1 April 1998

DESCRIPTION :
Assuming value is in a range of the node_status, this function returns the
start and stop values for that range.
==============================================================================*/

int Node_status_get_last_start_value(struct Node_status *node_status,
	int value,int *last_start_value);
/*******************************************************************************
LAST MODIFIED : 10 April 1998

DESCRIPTION :
==============================================================================*/

int Node_status_get_last_stop_value(struct Node_status *node_status,
	int value,int *last_stop_value);
/*******************************************************************************
LAST MODIFIED : 2 April 1998

DESCRIPTION :
==============================================================================*/

int Node_status_get_next_start_value(struct Node_status *node_status,
	int value,int *next_start_value);
/*******************************************************************************
LAST MODIFIED : 2 April 1998

DESCRIPTION :
==============================================================================*/

int Node_status_get_next_stop_value(struct Node_status *node_status,
	int value,int *next_stop_value);
/*******************************************************************************
LAST MODIFIED : 10 April 1998

DESCRIPTION :
==============================================================================*/

int Node_status_is_value_in_range(struct Node_status *node_status,int value);
/*******************************************************************************
LAST MODIFIED : 26 March 1998

DESCRIPTION :
Returns true if <value> is in any range in <node_status>.
==============================================================================*/

int Node_status_not_clear(struct Node_status *node_status,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
Iterator function returning true if there are any ranges defined for this
node_status.
==============================================================================*/

int Node_status_subtract(struct Node_status *node_status,
	struct Node_status *node_status_to_add);
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
Removes the ranges in the second Node_status from the first.
==============================================================================*/

int Node_status_add_range(struct Node_status *node_status,int start,int stop);
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
Adds the range from start to stop to node_status.
==============================================================================*/

int Node_status_remove_range(struct Node_status *node_status,
	int start,int stop);
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
Removes the range from start to stop from node_status.
==============================================================================*/

int Node_status_list_add(struct LIST(Node_status) *node_status_list,
	struct LIST(Node_status) *node_status_list_to_add);
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
Adds the ranges in the second node_status_list to those in the first.
==============================================================================*/

int Node_status_list_add_at_value(struct LIST(Node_status) *node_status_list,
	struct LIST(Node_status) *node_status_list_to_add,int value);
/*******************************************************************************
LAST MODIFIED : 10 April 1998

DESCRIPTION :
Adds the ranges in the second node_status_list to those in the first, but only
at the single specified value.
==============================================================================*/

int Node_status_list_clear(struct LIST(Node_status) *node_status_list);
/*******************************************************************************
LAST MODIFIED : 25 March 1998

DESCRIPTION :
Clears all ranges for all nodes in the list.
==============================================================================*/

struct LIST(Node_status) *Node_status_list_duplicate(
	struct LIST(Node_status) *node_status_list);
/*******************************************************************************
LAST MODIFIED : 26 April 1998

DESCRIPTION :
Returns a new list containing an exact copy of <node_status_list>.
==============================================================================*/

int Node_status_list_subtract(struct LIST(Node_status) *node_status_list,
	struct LIST(Node_status) *Node_status_list_to_subtract);
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
Subtracts the ranges in the second node_status_list from those in the first.
==============================================================================*/

int Node_status_list_subtract_at_value(
	struct LIST(Node_status) *node_status_list,
	struct LIST(Node_status) *node_status_list_to_subtract,int value);
/*******************************************************************************
LAST MODIFIED : 10 April 1998

DESCRIPTION :
Subtracts the ranges in the second node_status_list from those in the first, but
only at the single specified value.
==============================================================================*/

int Node_status_list_read(struct LIST(Node_status) *node_status_list,
	char *file_name);
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
Reads the lines containing node_no, start and stop from file_name into the
node_status_list.
==============================================================================*/

int Node_status_list_write(struct LIST(Node_status) *node_status_list,
	char *file_name,char *header_text,char *line_format);
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
For each range of each node write the node_no, start and stop with the supplied
line format. Also write the header_text at the top of the file.
==============================================================================*/


#endif /* !defined (TRACKING_EDITOR_DATA_H) */
