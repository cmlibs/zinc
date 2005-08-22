/*******************************************************************************
FILE : element_tool.h

LAST MODIFIED : 20 March 2003

DESCRIPTION :
Interactive tool for selecting elements with mouse and other devices.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
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
	struct Cmiss_region *region,
	struct FE_element_selection *element_selection,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Computed_field_package *computed_field_package,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface,
	struct Time_keeper *time_keeper,
	struct Execute_command *execute_command);
/*******************************************************************************
LAST MODIFIED : 20 March 2003

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

int Element_tool_pop_up_dialog(struct Element_tool *element_tool);
/*******************************************************************************
LAST MODIFIED : 20 June 2001

DESCRIPTION :
Pops up a dialog for editing settings of the Element_tool.
==============================================================================*/

int Element_tool_pop_down_dialog(struct Element_tool *element_tool);
/*******************************************************************************
LAST MODIFIED : 20 June 2001

DESCRIPTION :
Hides the dialog for editing settings of the Element_tool.
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

struct Computed_field *Element_tool_get_command_field(
	struct Element_tool *element_tool);
/*******************************************************************************
LAST MODIFIED : 30 September 2003

DESCRIPTION :
Returns the command_field to be executed when the element is clicked on in the
 <element_tool>.
==============================================================================*/

int Element_tool_set_command_field(struct Element_tool *element_tool,
	struct Computed_field *command_field);
/*******************************************************************************
LAST MODIFIED : 30 September 2003

DESCRIPTION :
Sets the command_field to be executed when the element is clicked on in the 
<element_tool>.
==============================================================================*/

#endif /* !defined (ELEMENT_TOOL_H) */
