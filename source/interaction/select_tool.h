/*******************************************************************************
FILE : select_tool.h

LAST MODIFIED : 24 August 2000

DESCRIPTION :
Interactive tool for selecting Any_objects associated with Scene_objects with
mouse and other devices.
==============================================================================*/
#if !defined (SELECT_TOOL_H)
#define SELECT_TOOL_H

#include "graphics/material.h"
#include "interaction/interactive_tool.h"
#include "selection/any_object_selection.h"

/*
Global types
------------
*/

struct Select_tool;
/*******************************************************************************
LAST MODIFIED : 24 August 2000

DESCRIPTION :
The contents of this structure are private.
==============================================================================*/

/*
Global functions
----------------
*/

struct Select_tool *CREATE(Select_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct Any_object_selection *any_object_selection,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Creates an Select_tool with Interactive_tool in
<interactive_tool_manager>. Selects Any_objects represented by scene_objects in
<any_object_selection> in response to interactive_events.
==============================================================================*/

int DESTROY(Select_tool)(struct Select_tool **select_tool_address);
/*******************************************************************************
LAST MODIFIED : 24 August 2000

DESCRIPTION :
Frees and deaccesses objects in the <select_tool> and deallocates the
structure itself.
==============================================================================*/

#endif /* !defined (SELECT_TOOL_H) */
