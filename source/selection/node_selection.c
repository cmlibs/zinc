/*******************************************************************************
FILE : node_selection.c

LAST MODIFIED : 3 April 2003

DESCRIPTION :
Global store of selected nodes for group actions and highlighting.
==============================================================================*/
#include <stdio.h>
#include "general/callback_private.h"
#include "general/debug.h"
#include "selection/node_selection.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

FULL_DECLARE_CMISS_CALLBACK_TYPES(FE_node_selection_change, \
	struct FE_node_selection *,struct FE_node_selection_changes *);

struct FE_node_selection
/*******************************************************************************
LAST MODIFIED : 3 April 2003

DESCRIPTION :
Global store of selected nodes for group actions and highlighting.
==============================================================================*/
{
	/* the FE_region the selection is from */
	struct FE_region *fe_region;
	/* flag indicating whether the cache is on */
	int cache;
	/* list of all nodes currently selected */
	struct LIST(FE_node) *node_list;
	/* list of all nodes selected since the last update; includes those already
		 selected; excludes those subsequently unselected */
	struct LIST(FE_node) *newly_selected_node_list;
	/* list of all nodes unselected since the last update; excludes those not
		 already selected; excludes those subsequently selected */
	struct LIST(FE_node) *newly_unselected_node_list;
	/* list of callbacks requested by other objects when selction changes */
	struct LIST(CMISS_CALLBACK_ITEM(FE_node_selection_change)) *change_callback_list;
}; /* struct FE_node_selection */

/*
Module functions
----------------
*/

DEFINE_CMISS_CALLBACK_MODULE_FUNCTIONS(FE_node_selection_change)

DEFINE_CMISS_CALLBACK_FUNCTIONS(FE_node_selection_change, \
	struct FE_node_selection *,struct FE_node_selection_changes *)

static int FE_node_selection_update(struct FE_node_selection *node_selection)
/*******************************************************************************
LAST MODIFIED : 21 March 2000

DESCRIPTION :
Tells the clients of the <node_selection> that objects have been selected or
unselected since the last update callback. Avoids sending message if caching is
on or if no such changes have occurred.
==============================================================================*/
{
	int return_code;
	struct FE_node_selection_changes changes;

	ENTER(FE_node_selection_update);
	if (node_selection)
	{
		if (!node_selection->cache)
		{
			/* only update if something changed */
			if ((0<NUMBER_IN_LIST(FE_node)(
				node_selection->newly_selected_node_list))||
				(0<NUMBER_IN_LIST(FE_node)(node_selection->newly_unselected_node_list)))
			{
				/* construct the call data */
				changes.newly_selected_node_list=
					node_selection->newly_selected_node_list;
				changes.newly_unselected_node_list=
					node_selection->newly_unselected_node_list;
				/* send the callbacks */
				CMISS_CALLBACK_LIST_CALL(FE_node_selection_change)(
					node_selection->change_callback_list,node_selection,&changes);
				/* clear the newly selected and unselected lists */
				REMOVE_ALL_OBJECTS_FROM_LIST(FE_node)(
					node_selection->newly_selected_node_list);
				REMOVE_ALL_OBJECTS_FROM_LIST(FE_node)(
					node_selection->newly_unselected_node_list);
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_selection_update.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_selection_update */

void FE_node_selection_FE_region_change(struct FE_region *fe_region,
	struct FE_region_changes *changes, void *node_selection_void)
/*******************************************************************************
LAST MODIFIED : 3 April 2003

DESCRIPTION :
Callback from <fe_region> with its <changes>.
Nodes removed from the <fe_region> must be removed from the <node_selection>.
==============================================================================*/
{
	enum CHANGE_LOG_CHANGE(FE_node) fe_node_change_summary;
	struct FE_node_selection *node_selection;

	ENTER(FE_node_selection_FE_region_change);
	if (fe_region && changes &&
		(node_selection = (struct FE_node_selection *)node_selection_void))
	{
		if (CHANGE_LOG_GET_CHANGE_SUMMARY(FE_node)(changes->fe_node_changes,
			&fe_node_change_summary) &&
			(fe_node_change_summary & CHANGE_LOG_OBJECT_REMOVED(FE_node)))
		{
			REMOVE_OBJECTS_FROM_LIST_THAT(FE_node)(FE_node_is_not_in_FE_region,
				(void *)fe_region, node_selection->node_list);
			REMOVE_OBJECTS_FROM_LIST_THAT(FE_node)(FE_node_is_not_in_FE_region,
				(void *)fe_region, node_selection->newly_selected_node_list);
			REMOVE_OBJECTS_FROM_LIST_THAT(FE_node)(FE_node_is_not_in_FE_region,
				(void *)fe_region, node_selection->newly_unselected_node_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_selection_FE_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* FE_node_selection_FE_region_change */

/*
Global functions
----------------
*/

struct FE_node_selection *CREATE(FE_node_selection)(
	struct FE_region *fe_region)
/*******************************************************************************
LAST MODIFIED : 3 April 2003

DESCRIPTION :
Creates the global store of selected nodes for group actions and highlighting.
Nodes from the <fe_region> are selectable by this object.
???RC In use <fe_region> should be its own master but this is not enforced.
==============================================================================*/
{
	struct FE_node_selection *node_selection;

	ENTER(CREATE(FE_node_selection));
	node_selection = (struct FE_node_selection *)NULL;
	if (fe_region)
	{
		if (ALLOCATE(node_selection,struct FE_node_selection,1))
		{
			FE_region_add_callback(fe_region,
				FE_node_selection_FE_region_change, (void *)node_selection);
			node_selection->fe_region = ACCESS(FE_region)(fe_region);
			node_selection->cache=0;
			node_selection->node_list=CREATE(LIST(FE_node))();
			node_selection->newly_selected_node_list=CREATE(LIST(FE_node))();
			node_selection->newly_unselected_node_list=CREATE(LIST(FE_node))();
			node_selection->change_callback_list=
				CREATE(LIST(CMISS_CALLBACK_ITEM(FE_node_selection_change)))();
			if (!(node_selection->node_list &&
				node_selection->newly_selected_node_list &&
				node_selection->newly_unselected_node_list &&
				node_selection->change_callback_list))
			{
				display_message(ERROR_MESSAGE,
					"CREATE(FE_node_selection).  Could not create lists");
				DESTROY(FE_node_selection)(&node_selection);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(FE_node_selection).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(FE_node_selection).  Invalid argument");
	}
	LEAVE;

	return (node_selection);
} /* CREATE(FE_node_selection) */

int DESTROY(FE_node_selection)(
	struct FE_node_selection **node_selection_address)
/*******************************************************************************
LAST MODIFIED : 3 April 2003

DESCRIPTION :
Destroys the FE_node_selection.
==============================================================================*/
{
	int return_code;
	struct FE_node_selection *node_selection;

	ENTER(DESTROY(FE_node_selection));
	if (node_selection_address&&(node_selection= *node_selection_address))
	{
		FE_region_remove_callback(node_selection->fe_region,
			FE_node_selection_FE_region_change, (void *)node_selection);
		DEACCESS(FE_region)(&(node_selection->fe_region));
		DESTROY(LIST(FE_node))(&(node_selection->node_list));
		DESTROY(LIST(FE_node))(&(node_selection->newly_selected_node_list));
		DESTROY(LIST(FE_node))(&(node_selection->newly_unselected_node_list));
		DESTROY(LIST(CMISS_CALLBACK_ITEM(FE_node_selection_change)))(
			&(node_selection->change_callback_list));
		DEALLOCATE(*node_selection_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(FE_node_selection).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_node_selection) */

int FE_node_selection_add_callback(struct FE_node_selection *node_selection,
	CMISS_CALLBACK_FUNCTION(FE_node_selection_change) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Adds a callback to <node_selection> so that when it changes <function> is
called with <user_data>. <function> has 3 arguments, a struct FE_node_selection,
a struct FE_node_selection_changes and the void *user_data.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_selection_add_callback);
	if (node_selection&&function)
	{
		if (CMISS_CALLBACK_LIST_ADD_CALLBACK(FE_node_selection_change)(
			node_selection->change_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_node_selection_add_callback.  Could not add callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_selection_add_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_selection_add_callback */

int FE_node_selection_remove_callback(struct FE_node_selection *node_selection,
	CMISS_CALLBACK_FUNCTION(FE_node_selection_change) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Removes the callback calling <function> with <user_data> from <node_selection>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_selection_remove_callback);
	if (node_selection&&function)
	{
		if (CMISS_CALLBACK_LIST_REMOVE_CALLBACK(FE_node_selection_change)(
			node_selection->change_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_node_selection_remove_callback.  Could not remove callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_selection_remove_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_selection_remove_callback */

int FE_node_selection_begin_cache(struct FE_node_selection *node_selection)
/*******************************************************************************
LAST MODIFIED : 15 March 2000

DESCRIPTION :
Starts caching for changes in the <node_selection>. No update callbacks will
occur until FE_node_selection_end_cache is called.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_selection_begin_cache);
	if (node_selection)
	{
		if (node_selection->cache)
		{
			display_message(ERROR_MESSAGE,
				"FE_node_selection_begin_cache.  Cache already enabled");
			return_code=0;
		}
		else
		{
			node_selection->cache=1;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_selection_begin_cache.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_selection_begin_cache */

int FE_node_selection_end_cache(struct FE_node_selection *node_selection)
/*******************************************************************************
LAST MODIFIED : 15 March 2000

DESCRIPTION :
Ends caching for selected list changes in the <selection>. Calls
FE_node_selection_update to inform clients of the changes that took place while
caching was on.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_selection_end_cache);
	if (node_selection)
	{
		if (node_selection->cache)
		{
			node_selection->cache=0;
			FE_node_selection_update(node_selection);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_node_selection_end_cache.  Cache not enabled");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_selection_end_cache.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_selection_end_cache */

int FE_node_selection_clear(struct FE_node_selection *node_selection)
/*******************************************************************************
LAST MODIFIED : 15 March 2000

DESCRIPTION :
Clears all the selected nodes in the <node_selection>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_selection_clear);
	if (node_selection)
	{
		FOR_EACH_OBJECT_IN_LIST(FE_node)(ensure_FE_node_is_in_list,
			(void *)node_selection->newly_unselected_node_list,
			node_selection->node_list);
		REMOVE_ALL_OBJECTS_FROM_LIST(FE_node)(node_selection->node_list);
		REMOVE_ALL_OBJECTS_FROM_LIST(FE_node)(
			node_selection->newly_selected_node_list);
		FE_node_selection_update(node_selection);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_selection_clear.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_selection_clear */

int FE_node_selection_select_node(struct FE_node_selection *node_selection,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 15 March 2000

DESCRIPTION :
Ensures <node> is in <node_selection>. Even if it is already selected, <node>
is added to the newly_selected_node_list and removed from the
newly_unselected_node_list if it is currently there.
Calls FE_node_selection_update.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_selection_select_node);
	if (node_selection&&node)
	{
		return_code=
			ensure_FE_node_is_in_list(node,(void *)node_selection->node_list);
		ensure_FE_node_is_in_list(node,
			(void *)node_selection->newly_selected_node_list);
		ensure_FE_node_is_not_in_list(node,
			(void *)node_selection->newly_unselected_node_list);
		FE_node_selection_update(node_selection);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_selection_select_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_selection_select_node */

int FE_node_select_in_FE_node_selection(struct FE_node *node,
	void *node_selection_void)
/*******************************************************************************
LAST MODIFIED : 28 April 2000

DESCRIPTION :
FE_node iterator version of FE_node_selection_select_node.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_select_in_FE_node_selection);
	return_code=FE_node_selection_select_node(
		(struct FE_node_selection *)node_selection_void,node);
	LEAVE;

	return (return_code);
} /* FE_node_select_in_FE_node_selection */

int FE_node_selection_unselect_node(struct FE_node_selection *node_selection,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 15 March 2000

DESCRIPTION :
If <node> is in <node_selection> it is removed and added to the
newly_unselected_node_list. Note it is added to this list if not currently
selected. Also ensures it is removed from the newly_selected_node_list if it is
currently there. Calls FE_node_selection_update.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_selection_unselect_node);
	if (node_selection&&node)
	{
		if (FIND_BY_IDENTIFIER_IN_LIST(FE_node,cm_node_identifier)(
			get_FE_node_identifier(node),node_selection->node_list))
		{
			ADD_OBJECT_TO_LIST(FE_node)(node,
				node_selection->newly_unselected_node_list);
			ensure_FE_node_is_not_in_list(node,
				(void *)node_selection->newly_selected_node_list);
			return_code=
				REMOVE_OBJECT_FROM_LIST(FE_node)(node,node_selection->node_list);
			FE_node_selection_update(node_selection);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_selection_unselect_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_selection_unselect_node */

int FE_node_unselect_in_FE_node_selection(struct FE_node *node,
	void *node_selection_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
FE_node iterator version of FE_node_selection_unselect_node.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_unselect_in_FE_node_selection);
	return_code=FE_node_selection_unselect_node(
		(struct FE_node_selection *)node_selection_void,node);
	LEAVE;

	return (return_code);
} /* FE_node_unselect_in_FE_node_selection */

struct LIST(FE_node) *FE_node_selection_get_node_list(
	struct FE_node_selection *node_selection)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Returns the list containing the nodes currently in <node_selection>.
RETURNED LIST IS NOT TO BE MODIFIED.
==============================================================================*/
{
	struct LIST(FE_node) *node_list;

	ENTER(FE_node_selection_get_node_list);
	if (node_selection)
	{
		node_list=node_selection->node_list;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_selection_get_node_list.  Invalid argument(s)");
		node_list=(struct LIST(FE_node) *)NULL;
	}
	LEAVE;

	return (node_list);
} /* FE_node_selection_get_node_list */

int FE_node_selection_is_node_selected(struct FE_node_selection *node_selection,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 21 March 2000

DESCRIPTION :
Returns true if the <node> is in the <node_selection>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_selection_is_node_selected);
	if (node_selection&&node)
	{
		if (IS_OBJECT_IN_LIST(FE_node)(node,node_selection->node_list))
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_selection_is_node_selected.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_selection_is_node_selected */
