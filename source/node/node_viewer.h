/*******************************************************************************
FILE : node_viewer.h

LAST MODIFIED : 11 May 2000

DESCRIPTION :
Dialog for selecting nodes and viewing and/or editing field values. Works with
FE_node_selection to display the last selected node, or set it if entered in
this dialog.
==============================================================================*/
#if !defined (NODE_VIEWER_H)
#define NODE_VIEWER_H

#include "general/callback.h"
#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "selection/node_selection.h"

/*
Global Types
------------
*/

struct Node_viewer;
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Contains all the information carried by the node_viewer widget.
The contents of this object are private.
==============================================================================*/

/*
Global Functions
----------------
*/

struct Node_viewer *CREATE(Node_viewer)(
	struct Node_viewer **node_viewer_address,char *dialog_title,
	struct MANAGER(FE_node) *node_manager,struct FE_node *initial_node,
	struct FE_node_selection *node_selection,
	struct Computed_field_package *computed_field_package,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Creates a dialog for choosing nodes and displaying and editing their fields.
==============================================================================*/

int DESTROY(Node_viewer)(struct Node_viewer **node_viewer_address);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION:
Destroys the Node_viewer. See also Node_viewer_close_CB.
==============================================================================*/

struct FE_node *Node_viewer_get_node(struct Node_viewer *node_viewer);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns the node currently in the <node_viewer>.
==============================================================================*/

int Node_viewer_set_node(struct Node_viewer *node_viewer,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Makes <node> the current_node in <node_viewer>.
==============================================================================*/

int Node_viewer_bring_window_to_front(struct Node_viewer *node_viewer);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Pops the window for <node_viewer> to the front of those visible.
==============================================================================*/
#endif /* !defined (NODE_VIEWER_H) */
