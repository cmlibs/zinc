/*******************************************************************************
FILE : node_tool.h

LAST MODIFIED : 10 July 2000

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
	struct MANAGER(FE_node) *node_manager,int use_data,
	struct FE_node_selection *node_selection,
	struct Computed_field_package *computed_field_package,
	struct Graphical_material *rubber_band_material);
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
Creates a Node_tool for editing nodes/data in the <node_manager>,
using the <node_selection>.
The <use_data> flag indicates that <node_manager> and <node_selection>
refer to data, not nodes, needed since different GT_element_settings types are
used to represent them.
==============================================================================*/

int DESTROY(Node_tool)(struct Node_tool **node_tool_address);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Frees and deaccesses objects in the <node_tool> and deallocates the
structure itself.
==============================================================================*/

struct FE_field *Node_tool_get_coordinate_field(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets the coordinate field of nodes created by <node_tool>.
==============================================================================*/

int Node_tool_set_coordinate_field(struct Node_tool *node_tool,
	struct FE_field *coordinate_field);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets the coordinate field of nodes created by <node_tool>.
==============================================================================*/

int Node_tool_get_create_enabled(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/

int Node_tool_set_create_enabled(struct Node_tool *node_tool,
	int create_enabled);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
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
LAST MODIFIED : 11 May 2000

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

struct GROUP(FE_node) *Node_tool_get_node_group(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns the node group new nodes are created in by <node_tool>.
==============================================================================*/

int Node_tool_set_node_group(struct Node_tool *node_tool,
	struct GROUP(FE_node) *node_group);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets the node group new nodes are created in by <node_tool>.
==============================================================================*/

int Node_tool_get_select_enabled(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/

int Node_tool_set_select_enabled(struct Node_tool *node_tool,
	int select_enabled);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/

#endif /* !defined (NODE_TOOL_H) */
