/*******************************************************************************
FILE : node_operations.h

LAST MODIFIED : 20 June 2001

DESCRIPTION :
Functions for mouse controlled node selection and position and vector editing
based on input from devices.
==============================================================================*/
#if !defined (NODE_OPERATIONS_H)
#define NODE_OPERATIONS_H

#include "selection/node_selection.h"

/*
Global functions
----------------
*/

int destroy_listed_nodes(struct LIST(FE_node) *node_list,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(FE_element) *element_manager,
	struct FE_node_selection *node_selection);
/*******************************************************************************
LAST MODIFIED : 20 June 2002

DESCRIPTION :
Destroys all the nodes in <node_list> that are not accessed outside
<node_manager>, the groups in <node_group_manager> and <node_selection>.
Nodes in use by elements in the <element_manager> cannot be destroyed so are
immediately ruled out in order to keep them in the node groups and selection.
<node_group_manager>, <element_manager> and <node_selection> are optional.
Upon return <node_list> contains all the nodes that could not be destroyed.
==============================================================================*/

#endif /* !defined (NODE_OPERATIONS_H) */
