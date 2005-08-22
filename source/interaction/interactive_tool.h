/*******************************************************************************
FILE : interactive_tool.h

LAST MODIFIED : 18 July 2000

DESCRIPTION :
Active CMGUI tools should create a wrapper Interactive_tool that supplies a
consistent way of choosing which one is to receive input from a given input
device, and listing which ones are active.
Only certain such tools will be capable of receiving input events from scene
viewer/mouse input and other devices, but many more will follow and change the
content of the global selections and objects with text input.
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
#if !defined (INTERACTIVE_TOOL_H)
#define INTERACTIVE_TOOL_H

#include "general/image_utilities.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/colour.h"
#include "interaction/interactive_event.h"

/*
Global types
------------
*/

typedef void Interactive_event_handler(void *device_id,
	struct Interactive_event *event,void *user_data);
typedef int Interactive_tool_bring_up_dialog_function(void *user_data);
typedef struct Cmgui_image *Interactive_tool_get_icon_function(
	struct Colour *foreground, struct Colour *background, void *user_data);
typedef int Interactive_tool_destroy_tool_data_function(
   void **interactive_tool_data_address);

struct Interactive_tool;
/*******************************************************************************
LAST MODIFIED : 10 April 2000

DESCRIPTION :
Wrapper object for CMGUI tools giving them a name, a consistent interface for
sending input device events to, and can be put in a manager.
The contents of this object are private.
==============================================================================*/

DECLARE_LIST_TYPES(Interactive_tool);
DECLARE_MANAGER_TYPES(Interactive_tool);

/*
Global functions
----------------
*/

struct Interactive_tool *CREATE(Interactive_tool)(char *name,char *display_name,
	char *tool_type_name,
	Interactive_event_handler *interactive_event_handler,
	Interactive_tool_get_icon_function *get_icon_function,
	Interactive_tool_bring_up_dialog_function *bring_up_dialog_function,
   Interactive_tool_destroy_tool_data_function *destroy_tool_data_function,
	void *tool_data);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Creates an Interactive_tool with the given <name> and <icon>. If an
<interactive_event_handler> is supplied it is called to pass on any input
events to the interactive_tool, with the <tool_data> passed as the third
parameter.
==============================================================================*/

int DESTROY(Interactive_tool)(
	struct Interactive_tool **interactive_tool_address);
/*******************************************************************************
LAST MODIFIED : 10 April 2000

DESCRIPTION :
Destroys the Interactive_tool.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Interactive_tool);
PROTOTYPE_LIST_FUNCTIONS(Interactive_tool);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Interactive_tool,name,char *);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Interactive_tool);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Interactive_tool,name,char *);
PROTOTYPE_MANAGER_FUNCTIONS(Interactive_tool);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Interactive_tool,name,char *);

char *Interactive_tool_get_display_name(
	struct Interactive_tool *interactive_tool);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns the display_name of the tool = what should be shown on widget/window
descriptions of the tool. The actual name generally uses underscore _ characters
instead of spaces for more terse commands.
Up to calling function to DEALLOCATE the returned copy of the display_name.
==============================================================================*/

char *Interactive_tool_get_tool_type_name(
	struct Interactive_tool *interactive_tool);
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
Returns the static pointer to the string which belongs to the tool type.  Do
not DEALLOCATE this pointer.
==============================================================================*/

int Interactive_tool_handle_interactive_event(
	struct Interactive_tool *interactive_tool,void *device_id,
	struct Interactive_event *interactive_event);
/*******************************************************************************
LAST MODIFIED : 10 April 2000

DESCRIPTION :
Passes the <interactive_event> from <device_id> to the tool wrapped by the
<interactive_tool> object.
==============================================================================*/

struct Cmgui_image *Interactive_tool_get_icon(struct Colour *foreground, 
	struct Colour *background, struct Interactive_tool *interactive_tool);
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Returns the icon which a user_interface can use to represent the tool.
==============================================================================*/

int Interactive_tool_bring_up_dialog(struct Interactive_tool *interactive_tool);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
If the bring_up_dialog function is defined for <interactive_tool> calls it to
bring up the dialog for changing its settings.
==============================================================================*/

char **interactive_tool_manager_get_tool_names(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	int *number_of_tools,struct Interactive_tool *current_interactive_tool,
	char **current_tool_name);
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
Returns an array of strings containing the names of the tools - suitable for
choosing in a text command. On success, also returns <*number_of_tools>,
and in <current_tool_name> the pointer to the tool_names
string for <current_interactive_tool>, or the first one if it is not found.
Up to calling function to deallocate the returned array AND the strings in it.
==============================================================================*/

#endif /* !defined (INTERACTIVE_TOOL_H) */
