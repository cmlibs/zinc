/*******************************************************************************
FILE : element_tool.h

LAST MODIFIED : 20 July 2000

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
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct FE_element_selection *element_selection,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

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

int Element_tool_get_select_elements_enabled(struct Element_tool *element_tool);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns flag controlling whether top-level & 3-D elements can be selected.
==============================================================================*/

int Element_tool_set_select_elements_enabled(struct Element_tool *element_tool,
	int select_elements_enabled);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Sets flag controlling whether top-level & 3-D elements can be selected.
==============================================================================*/

int Element_tool_get_select_faces_enabled(struct Element_tool *element_tool);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns flag controlling whether face & 2-D top-level elements can be selected.
==============================================================================*/

int Element_tool_set_select_faces_enabled(struct Element_tool *element_tool,
	int select_faces_enabled);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns flag controlling whether face & 2-D top-level elements can be selected.
==============================================================================*/

int Element_tool_get_select_lines_enabled(struct Element_tool *element_tool);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns flag controlling whether line & 1-D top-level elements can be selected.
==============================================================================*/

int Element_tool_set_select_lines_enabled(struct Element_tool *element_tool,
	int select_lines_enabled);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns flag controlling whether line & 1-D top-level elements can be selected.
==============================================================================*/

int destroy_listed_elements(struct LIST(FE_element) *element_list,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct FE_element_selection *element_selection,
	struct Element_point_ranges_selection *element_point_ranges_selection);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Destroys all the elements in <element_list> that are not accessed outside
<element_manager>, the groups in <element_group_manager>,
<element_selection> and <element_point_ranges_selection>.
<element_group_manager>, <element_selection> and
<element_point_ranges_selection> are optional. Upon return <element_list>
contains all the elements that could not be destroyed.
Note that <element_list> should only contain top-level-elements, at present.
???RC Should really be in its own module.
==============================================================================*/

#endif /* !defined (ELEMENT_TOOL_H) */
