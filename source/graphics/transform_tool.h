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

struct Transform_tool *CREATE(Transform_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager);
/*******************************************************************************
LAST MODIFIED : 12 June 2000

DESCRIPTION :
Creates a Transform_tool which works like a placeholder for a graphics windows
own transform mode - so it appears like any other tool in a toolbar.
==============================================================================*/

int DESTROY(Transform_tool)(struct Transform_tool **transform_tool_address);
/*******************************************************************************
LAST MODIFIED : 12 June 2000

DESCRIPTION :
Frees and deaccesses objects in the <transform_tool> and deallocates the
structure itself.
==============================================================================*/

#endif /* !defined (TRANSFORM_TOOL_H) */
