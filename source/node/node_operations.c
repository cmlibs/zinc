/*******************************************************************************
FILE : node_operations.c

LAST MODIFIED : 21 June 2002

DESCRIPTION :
Functions for mouse controlled node position and vector editing based on
Scene input.
==============================================================================*/
#include <math.h>
#include "general/debug.h"
#include "node/node_operations.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

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
