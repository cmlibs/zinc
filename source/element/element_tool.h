/*******************************************************************************
FILE : element_tool.h

LAST MODIFIED : 14 July 2000

DESCRIPTION :
Interactive tool for selecting elements with mouse and other devices.
==============================================================================*/
#if !defined (ELEMENT_TOOL_H)
#define ELEMENT_TOOL_H

#include "interaction/interactive_tool.h"
#include "selection/element_selection.h"

/*
Global types
------------
*/

struct Element_tool;
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
The contents of this structure are private.
==============================================================================*/

/*
Global functions
----------------
*/

struct Element_tool *CREATE(Element_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct FE_element_selection *element_selection,
	struct Graphical_material *rubber_band_material);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Creates an Element_tool with Interactive_tool in <interactive_tool_manager>.
Selects elements in <element_selection> in response to interactive_events.
==============================================================================*/

int DESTROY(Element_tool)(struct Element_tool **element_tool_address);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Frees and deaccesses objects in the <element_tool> and deallocates the
structure itself.
==============================================================================*/

#endif /* !defined (ELEMENT_TOOL_H) */
