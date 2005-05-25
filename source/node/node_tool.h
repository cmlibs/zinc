/*******************************************************************************
FILE : node_tool.h

LAST MODIFIED : 15 January 2003

DESCRIPTION :
Functions for mouse controlled node selection and position and vector editing
based on input from devices.
==============================================================================*/
#if !defined (NODE_TOOL_H)
#define NODE_TOOL_H

#include "finite_element/finite_element.h"
#include "graphics/material.h"
#include "interaction/interactive_tool.h"
#include "selection/node_selection.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/

enum Node_tool_edit_mode
{
	NODE_TOOL_EDIT_AUTOMATIC,
	NODE_TOOL_EDIT_POSITION,
	NODE_TOOL_EDIT_VECTOR
};

struct Node_tool;
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Object storing all the parameters for converting scene input messages into
changes in node position and derivatives etc.
The contents of this structure are private.
==============================================================================*/

/*
Global functions
----------------
*/

struct Node_tool *CREATE(Node_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct Cmiss_region *root_region, int use_data,
	struct FE_node_selection *node_selection,
	struct Computed_field_package *computed_field_package,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface,
	struct Time_keeper *time_keeper,
	struct Execute_command *execute_command);
/*******************************************************************************
LAST MODIFIED : 15 January 2003

DESCRIPTION :
Creates a Node_tool for editing nodes/data in the <node_manager>,
using the <node_selection>.
The <use_data> flag indicates that <node_manager> and <node_selection>
refer to data, not nodes, needed since different GT_element_settings types are
used to represent them. <element_manager> should be NULL if <use_data> is true.
==============================================================================*/

int DESTROY(Node_tool)(struct Node_tool **node_tool_address);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Frees and deaccesses objects in the <node_tool> and deallocates the
structure itself.
==============================================================================*/

int Node_tool_pop_up_dialog(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 20 June 2001

DESCRIPTION :
Pops up a dialog for editing settings of the Node_tool.
==============================================================================*/

int Node_tool_pop_down_dialog(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 20 June 2001

DESCRIPTION :
Hides the dialog for editing settings of the Node_tool.
==============================================================================*/

struct Computed_field *Node_tool_get_coordinate_field(
	struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 12 September 2000

DESCRIPTION :
Returns the coordinate field used by the <node_tool> when create/define are on.
==============================================================================*/

int Node_tool_set_coordinate_field(struct Node_tool *node_tool,
	struct Computed_field *coordinate_field);
/*******************************************************************************
LAST MODIFIED : 12 September 2000

DESCRIPTION :
Sets the coordinate field to be defined by the <node_tool> when create/define
are on.
==============================================================================*/

int Node_tool_get_create_enabled(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Returns flag controlling whether nodes can be created when none are selected
on a mouse button press.
==============================================================================*/

int Node_tool_set_create_enabled(struct Node_tool *node_tool,
	int create_enabled);
/*******************************************************************************
LAST MODIFIED : 11 September 2000

DESCRIPTION :
Sets flag controlling whether nodes can be created when none are selected
on a mouse button press. Also ensures define is enabled if create is.
==============================================================================*/

int Node_tool_get_define_enabled(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Returns flag controlling whether nodes can be defined when none are selected
on a mouse button press.
==============================================================================*/

int Node_tool_set_define_enabled(struct Node_tool *node_tool,
	int define_enabled);
/*******************************************************************************
LAST MODIFIED : 12 September 2000

DESCRIPTION :
Sets flag controlling whether the coordinate field can be defined on any new
or individually selected existing nodes.
==============================================================================*/

int Node_tool_get_edit_enabled(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/

int Node_tool_set_edit_enabled(struct Node_tool *node_tool,int edit_enabled);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Sets flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/

enum Node_tool_edit_mode Node_tool_get_edit_mode(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns the current edit mode of <node_tool>.
==============================================================================*/

int Node_tool_set_edit_mode(struct Node_tool *node_tool,
	enum Node_tool_edit_mode edit_mode);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets the <edit_mode> of <node_tool> - controls whether the editor
can select or edit nodes, and whether the editing is restricted to position or
vector only.
==============================================================================*/

int Node_tool_get_motion_update_enabled(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/

int Node_tool_set_motion_update_enabled(struct Node_tool *node_tool,
	int motion_update_enabled);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/

int Node_tool_get_region_path(struct Node_tool *node_tool,
	char **path_address);
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Returns in <path_address> the path to the Cmiss_region where nodes created by
the <node_tool> are put.
Up to the calling function to DEALLOCATE the returned path.
==============================================================================*/

int Node_tool_set_region_path(struct Node_tool *node_tool,
	char *path);
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Sets the <path> to the region/FE_region where nodes created by
<node_tool> are placed.
==============================================================================*/

int Node_tool_get_select_enabled(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns flag controlling whether existing nodes can be selected.
==============================================================================*/

int Node_tool_set_select_enabled(struct Node_tool *node_tool,
	int select_enabled);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Sets flag controlling whether existing nodes can be selected.
==============================================================================*/

int Node_tool_get_streaming_create_enabled(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 14 May 2001

DESCRIPTION :
Returns flag controlling, if create_enabled, whether a stream of nodes is
created as the user drags the mouse around.
==============================================================================*/

int Node_tool_set_streaming_create_enabled(struct Node_tool *node_tool,
	int streaming_create_enabled);
/*******************************************************************************
LAST MODIFIED : 14 May 2001

DESCRIPTION :
Sets flag controlling, if create_enabled, whether a stream of nodes is
created as the user drags the mouse around.
==============================================================================*/

int Node_tool_get_constrain_to_surface(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 26 May 2005

DESCRIPTION :
Returns flag controlling, if create_enabled, whether new nodes will be created
on the closest surface element or just halfway between near and far.
==============================================================================*/

int Node_tool_set_constrain_to_surface(struct Node_tool *node_tool,
	int constrain_to_surface);
/*******************************************************************************
LAST MODIFIED : 26 May 2005

DESCRIPTION :
Sets flag controlling, if create_enabled, whether new nodes will be created
on the closest surface element or just halfway between near and far.
==============================================================================*/

struct Computed_field *Node_tool_get_command_field(
	struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 30 September 2003

DESCRIPTION :
Returns the command_field to be executed when the node is clicked on in the <node_tool>.
==============================================================================*/

int Node_tool_set_command_field(struct Node_tool *node_tool,
	struct Computed_field *command_field);
/*******************************************************************************
LAST MODIFIED : 30 September 2003

DESCRIPTION :
Sets the command_field to be executed when the node is clicked on in the <node_tool>.
==============================================================================*/

struct Interactive_tool *Node_tool_get_interactive_tool(
	struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 7 April 2005

DESCRIPTION :
Returns the generic interactive_tool the represents the <node_tool>.
==============================================================================*/

#endif /* !defined (NODE_TOOL_H) */
