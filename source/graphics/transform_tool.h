/*******************************************************************************
FILE : transform_tool.h

LAST MODIFIED : 12 June 2000

DESCRIPTION :
Icon/tool representing the transform function on a graphics window.
Eventually use to store parameters for the transform function.
==============================================================================*/
#if !defined (TRANSFORM_TOOL_H)
#define TRANSFORM_TOOL_H

#include "interaction/interactive_tool.h"

/*
Global types
------------
*/

struct Transform_tool;
/*******************************************************************************
LAST MODIFIED : 12 June 2000

DESCRIPTION :
The contents of this object are private.
==============================================================================*/

/*
Global functions
----------------
*/

int Interactive_tool_is_Transform_tool(struct Interactive_tool *interactive_tool);
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
Identifies whether an Interactive_tool is a Transform_tool.
==============================================================================*/

int Interactive_tool_transform_get_free_spin(
	struct Interactive_tool *interactive_tool);
/*******************************************************************************
LAST MODIFIED : 9 October 2000

DESCRIPTION :
If the interactive tool is of type Transform this function specifies whether 
the window should spin freely when tumbling.
==============================================================================*/

int Interactive_tool_transform_set_free_spin(struct Interactive_tool *interactive_tool,
	int free_spin);
/*******************************************************************************
LAST MODIFIED : 9 October 2000

DESCRIPTION :
If the interactive tool is of type Transform this function controls whether 
the window will spin freely when tumbling.
==============================================================================*/

struct Interactive_tool *create_Interactive_tool_transform(
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 9 October 2000

DESCRIPTION :
Creates a transform type Interactive_tool which control the transformation of
scene_viewers.
==============================================================================*/
#endif /* !defined (TRANSFORM_TOOL_H) */
