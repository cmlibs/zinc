/*******************************************************************************
FILE : node_viewer.h

LAST MODIFIED : 24 January 2003

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
#include "time/time.h"
#include "user_interface/user_interface.h"

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
	struct Node_viewer **node_viewer_address,
	char *dialog_title,
	struct FE_node *initial_node,
	struct Cmiss_region *root_region,
	struct FE_node_selection *node_selection,
	struct Computed_field_package *computed_field_package,
	struct Time_object *time_object,	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 24 January 2003

DESCRIPTION :
Creates a dialog for choosing nodes and displaying and editing their fields.
Nodes, starting with <initial_node> may be chosen from <this_node_manager>.
Pass <initial_data> and <data_manager> in these parameters for data viewer.
Since both nodes and data can depend on embedded fields, the
<actual_node_manager> and <actual_element_manager> also need to be passed.
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
