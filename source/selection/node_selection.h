/*******************************************************************************
FILE : node_selection.h

LAST MODIFIED : 4 July 2000

DESCRIPTION :
Global store of selected nodes for group actions and highlighting.
==============================================================================*/
#if !defined (NODE_SELECTION_H)
#define NODE_SELECTION_H

#include "finite_element/finite_element.h"
#include "general/callback.h"
#include "general/object.h"

/*
Global types
------------
*/

struct FE_node_selection;
/*******************************************************************************
LAST MODIFIED : 15 March 2000

DESCRIPTION :
Global store of selected nodes for group actions and highlighting.
The contents of this object are private.
==============================================================================*/

struct FE_node_selection_changes
/*******************************************************************************
LAST MODIFIED : 17 March 2000

DESCRIPTION :
Data broadcast with callbacks from <FE_node_selection> describing the changes.
==============================================================================*/
{
	/* list of nodes selected since the last update callback - includes those
     already selected; excludes those subsequently unselected */
	struct LIST(FE_node) *newly_selected_node_list;
	/* list of nodes unselected since the last update callback - excludes both
		 those already unselected and those subsequently reselected */
	struct LIST(FE_node) *newly_unselected_node_list;
}; /* struct FE_node_selection_changes */

DECLARE_CMISS_CALLBACK_TYPES(FE_node_selection_change,struct FE_node_selection *, \
	struct FE_node_selection_changes *);

/*
Global functions
----------------
*/
struct FE_node_selection *CREATE(FE_node_selection)(void);
/*******************************************************************************
LAST MODIFIED : 15 March 2000

DESCRIPTION :
Creates the global store of selected nodes for group actions and highlighting.
==============================================================================*/

int DESTROY(FE_node_selection)(
	struct FE_node_selection **node_selection_address);
/*******************************************************************************
LAST MODIFIED : 15 March 2000

DESCRIPTION :
Destroys the FE_node_selection.
==============================================================================*/

int FE_node_selection_add_callback(struct FE_node_selection *node_selection,
	CMISS_CALLBACK_FUNCTION(FE_node_selection_change) *function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Adds a callback to <node_selection> so that when it changes <function> is
called with <user_data>. <function> has 3 arguments, a struct FE_node_selection,
a struct FE_node_selection_changes and the void *user_data.
==============================================================================*/

int FE_node_selection_remove_callback(struct FE_node_selection *node_selection,
	CMISS_CALLBACK_FUNCTION(FE_node_selection_change) *function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Removes the callback calling <function> with <user_data> from <node_selection>.
==============================================================================*/

int FE_node_selection_begin_cache(struct FE_node_selection *node_selection);
/*******************************************************************************
LAST MODIFIED : 15 March 2000

DESCRIPTION :
Starts caching for changes in the <node_selection>. No update callbacks will
occur until FE_node_selection_end_cache is called.
==============================================================================*/

int FE_node_selection_end_cache(struct FE_node_selection *node_selection);
/*******************************************************************************
LAST MODIFIED : 15 March 2000

DESCRIPTION :
Ends caching for selected list changes in the <selection>. Calls
FE_node_selection_update to inform clients of the changes that took place while
caching was on.
==============================================================================*/

int FE_node_selection_clear(struct FE_node_selection *node_selection);
/*******************************************************************************
LAST MODIFIED : 15 March 2000

DESCRIPTION :
Clears all the selected nodes in the <node_selection>.
==============================================================================*/

int FE_node_selection_select_node(struct FE_node_selection *node_selection,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 15 March 2000

DESCRIPTION :
Ensures <node> is in <node_selection>. Even if it is already selected, <node>
is added to the newly_selected_node_list and removed from the
newly_unselected_node_list if it is currently there.
Calls FE_node_selection_update.
==============================================================================*/

int FE_node_select_in_FE_node_selection(struct FE_node *node,
	void *node_selection_void);
/*******************************************************************************
LAST MODIFIED : 28 April 2000

DESCRIPTION :
FE_node iterator version of FE_node_selection_select_node.
==============================================================================*/

int FE_node_selection_unselect_node(struct FE_node_selection *node_selection,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 15 March 2000

DESCRIPTION :
If <node> is in <node_selection> it is removed and added to the
newly_unselected_node_list. Note it is added to this list if not currently
selected. Also ensures it is removed from the newly_selected_node_list if it is
currently there. Calls FE_node_selection_update.
==============================================================================*/

int FE_node_unselect_in_FE_node_selection(struct FE_node *node,
	void *node_selection_void);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
FE_node iterator version of FE_node_selection_unselect_node.
==============================================================================*/

struct LIST(FE_node) *FE_node_selection_get_node_list(
	struct FE_node_selection *node_selection);
/*******************************************************************************
LAST MODIFIED : 15 March 2000

DESCRIPTION :
Returns the list containing the nodes currently in <node_selection>.
RETURNED LIST IS NOT TO BE MODIFIED.
==============================================================================*/

int FE_node_selection_is_node_selected(struct FE_node_selection *node_selection,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 21 March 2000

DESCRIPTION :
Returns true if the <node> is in the <node_selection>.
==============================================================================*/

#endif /* !defined (NODE_SELECTION_H) */
