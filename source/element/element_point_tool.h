/*******************************************************************************
FILE : element_point_tool.h

LAST MODIFIED : 5 July 2002

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
	struct Computed_field_package *computed_field_package,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface,
	struct Time_keeper *time_keeper,
	struct Execute_command *execute_command);
/*******************************************************************************
LAST MODIFIED : 5 July 2002

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

int Element_point_tool_pop_up_dialog(
	struct Element_point_tool *element_point_tool);
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Pops up a dialog for editing settings of the Element_point_tool.
==============================================================================*/

int Element_point_tool_pop_down_dialog(
	struct Element_point_tool *element_point_tool);
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Hides the dialog for editing settings of the Element_point_tool.
==============================================================================*/

struct Computed_field *Element_point_tool_get_command_field(
	struct Element_point_tool *element_point_tool);
/*******************************************************************************
LAST MODIFIED : 30 September 2003

DESCRIPTION :
Returns the command_field to be executed when the element is clicked on in the 
<element_point_tool>.
==============================================================================*/

int Element_point_tool_set_command_field(
	struct Element_point_tool *element_point_tool,
	struct Computed_field *command_field);
/*******************************************************************************
LAST MODIFIED : 30 September 2003

DESCRIPTION :
Sets the command_field to be executed when the element is clicked on in the 
<element_point_tool>.
==============================================================================*/

#endif /* !defined (ELEMENT_POINT_TOOL_H) */
