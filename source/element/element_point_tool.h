/*******************************************************************************
FILE : element_point_tool.h

LAST MODIFIED : 14 July 2000

DESCRIPTION :
Interactive tool for selecting element/grid points with mouse and other devices.
==============================================================================*/
#if !defined (ELEMENT_POINT_TOOL_H)
#define ELEMENT_POINT_TOOL_H

#include "graphics/material.h"
#include "interaction/interactive_tool.h"
#include "selection/element_point_ranges_selection.h"

/*
Global types
------------
*/

struct Element_point_tool;
/*******************************************************************************
LAST MODIFIED : 16 May 2000

DESCRIPTION :
The contents of this structure are private.
==============================================================================*/

/*
Global functions
----------------
*/

struct Element_point_tool *CREATE(Element_point_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Graphical_material *rubber_band_material);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Creates an Element_point_tool with Interactive_tool in
<interactive_tool_manager>. Selects element/grid points in
<element_point_ranges_selection> in response to interactive_events.
==============================================================================*/

int DESTROY(Element_point_tool)(
	struct Element_point_tool **element_point_tool_address);
/*******************************************************************************
LAST MODIFIED : 16 May 2000

DESCRIPTION :
Frees and deaccesses objects in the <element_point_tool> and deallocates the
structure itself.
==============================================================================*/

#endif /* !defined (ELEMENT_POINT_TOOL_H) */
