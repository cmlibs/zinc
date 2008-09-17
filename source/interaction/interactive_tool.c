/*******************************************************************************
FILE : interactive_tool.c

LAST MODIFIED : 21 January 2002

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
#include "general/debug.h"
#include "general/list_private.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "interaction/interactive_tool.h"
#include "interaction/interactive_tool_private.h"
#include "user_interface/message.h"

/*
Module types
------------
*/


struct Interactive_tool
/*******************************************************************************
LAST MODIFIED : 25 February 2008

DESCRIPTION :
Structure describing an interactive event (eg. button press at point in space).
ACCESS this object for as long as you need to keep it; it is not modifiable.
==============================================================================*/
{
	/* name identifier for the tool and display name for presentation */
	char *display_name,*name;
	/* This points to the static string which identifies the tool type */
	char *tool_type_name;
	Interactive_event_handler *interactive_event_handler;
	Interactive_tool_get_icon_function *get_icon_function;
	Interactive_tool_bring_up_dialog_function *bring_up_dialog_function;
	Interactive_tool_reset_function *reset_function;
	Interactive_tool_destroy_tool_data_function *destroy_tool_data_function;
	Interactive_tool_copy_function *copy_function;
	/* data for the actual tool receiving the events */
	void *tool_data;
	int access_count;
}; /* struct Interactive_tool */

FULL_DECLARE_LIST_TYPE(Interactive_tool);
FULL_DECLARE_MANAGER_TYPE(Interactive_tool);

/*
Module functions
----------------
*/

DECLARE_LOCAL_MANAGER_FUNCTIONS(Interactive_tool)

/*
Global functions
----------------
*/

struct Interactive_tool *CREATE(Interactive_tool)(const char *name,const char *display_name,
	char *tool_type_name,
	Interactive_event_handler *interactive_event_handler,
	Interactive_tool_get_icon_function *get_icon_function,
	Interactive_tool_bring_up_dialog_function *bring_up_dialog_function,
	Interactive_tool_reset_function *reset_function,
	Interactive_tool_destroy_tool_data_function *destroy_tool_data_function,
	Interactive_tool_copy_function *copy_function,	
	void *tool_data)
/*******************************************************************************
LAST MODIFIED : 25 February 2008

DESCRIPTION :
Creates an Interactive_tool with the given <name> and <icon>. If an
<interactive_event_handler> is supplied it is called to pass on any input
events to the interactive_tool, with the <tool_data> passed as the third
parameter.  The <tool_type_name> is used to identify what object type the tool
is and each type of tool should pass a pointer to a static string naming that
type.
==============================================================================*/
{
	struct Interactive_tool *interactive_tool;

	ENTER(CREATE(Interactive_tool));
	if (name&&display_name&&get_icon_function)
	{
		if (ALLOCATE(interactive_tool,struct Interactive_tool,1)&&
			(interactive_tool->name=duplicate_string(name))&&
			(interactive_tool->display_name=duplicate_string(display_name)))
		{
			/* We don't duplicate this string as it is the pointer to the 
				string which identifies its type */
			interactive_tool->tool_type_name=tool_type_name;
			interactive_tool->get_icon_function=get_icon_function;
			interactive_tool->bring_up_dialog_function=bring_up_dialog_function;
			interactive_tool->reset_function=reset_function;
			interactive_tool->interactive_event_handler=interactive_event_handler;
			interactive_tool->destroy_tool_data_function=destroy_tool_data_function;
			interactive_tool->tool_data=tool_data;
			interactive_tool->copy_function=copy_function;
			interactive_tool->access_count=0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Interactive_tool).  Not enough memory");
			if (interactive_tool)
			{
				if (interactive_tool->name)
				{
					DEALLOCATE(interactive_tool->name);
				}
				DEALLOCATE(interactive_tool);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Interactive_tool).  Invalid argument(s)");
		interactive_tool=(struct Interactive_tool *)NULL;
	}
	LEAVE;

	return (interactive_tool);
} /* CREATE(Interactive_tool) */

int DESTROY(Interactive_tool)(
	struct Interactive_tool **interactive_tool_address)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Destroys the Interactive_tool.
==============================================================================*/
{
	int return_code;
	struct Interactive_tool *interactive_tool;

	ENTER(DESTROY(Interactive_tool));
	if (interactive_tool_address&&
		(interactive_tool= *interactive_tool_address))
	{
		if (0==interactive_tool->access_count)
		{
			if (interactive_tool->destroy_tool_data_function)
			{
				(interactive_tool->destroy_tool_data_function)(&interactive_tool->tool_data);
			}
			DEALLOCATE(interactive_tool->name);
			DEALLOCATE(interactive_tool->display_name);
			DEALLOCATE(*interactive_tool_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Interactive_tool).  Non-zero access count!");
			*interactive_tool_address=(struct Interactive_tool *)NULL;
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Interactive_tool).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Interactive_tool) */

DECLARE_OBJECT_FUNCTIONS(Interactive_tool)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Interactive_tool)
DECLARE_LIST_FUNCTIONS(Interactive_tool)
DECLARE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Interactive_tool, \
	name,char *,strcmp)
DECLARE_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Interactive_tool,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Interactive_tool,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Interactive_tool,name));
	if (source&&destination)
	{
		if (source->name)
		{
			if (ALLOCATE(name,char,strlen(source->name)+1))
			{
				strcpy(name,source->name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITH_IDENTIFIER(Interactive_tool,name).  "
					"Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			if (return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Interactive_tool,name)(
				destination, source))
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITH_IDENTIFIER(Interactive_tool,name).  "
					"Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITH_IDENTIFIER(Interactive_tool,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Interactive_tool,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Interactive_tool,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Interactive_tool,name));
	if (source&&destination)
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Interactive_tool,name).  "
			"Not implemented");
		return_code=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Interactive_tool,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Interactive_tool,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Interactive_tool,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Interactive_tool,name));
	if (name&&destination)
	{
		if (name)
		{
			if (ALLOCATE(destination_name,char,strlen(name)+1))
			{
				strcpy(destination_name,name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_IDENTIFIER(Interactive_tool,name).  "
					"Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			/* copy name */
			DEALLOCATE(destination->name);
			destination->name=destination_name;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Interactive_tool,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Interactive_tool,name) */

DECLARE_MANAGER_FUNCTIONS(Interactive_tool)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Interactive_tool)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Interactive_tool,name,char *)

char *Interactive_tool_get_display_name(
	struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns the display_name of the tool = what should be shown on widget/window
descriptions of the tool. The actual name generally uses underscore _ characters
instead of spaces for more terse commands.
Up to calling function to DEALLOCATE the returned copy of the display_name.
==============================================================================*/
{
	char *display_name;

	ENTER(Interactive_tool_get_display_name);
	if (interactive_tool)
	{
		display_name=duplicate_string(interactive_tool->display_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Interactive_tool_get_display_name.  Invalid argument(s)");
		display_name=(char *)NULL;
	}
	LEAVE;

	return (display_name);
} /* Interactive_tool_get_display_name */

char *Interactive_tool_get_tool_type_name(
	struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
Returns the static pointer to the string which belongs to the tool type.  Do
not DEALLOCATE this pointer.
==============================================================================*/
{
	char *tool_type_name;

	ENTER(Interactive_tool_get_tool_type_name);
	if (interactive_tool)
	{
		tool_type_name = interactive_tool->tool_type_name;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Interactive_tool_get_tool_type_name.  Invalid argument(s)");
		tool_type_name=(char *)NULL;
	}
	LEAVE;

	return (tool_type_name);
} /* Interactive_tool_get_tool_type_name */

void *Interactive_tool_get_tool_data(
	struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
Priveleged function for objects of type Interactive tool to get their own
type specific data.
==============================================================================*/
{
	void *tool_data;

	ENTER(Interactive_tool_get_tool_data);
	if (interactive_tool)
	{
		tool_data = interactive_tool->tool_data;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Interactive_tool_get_tool_data.  Invalid argument(s)");
		tool_data = NULL;
	}
	LEAVE;

	return (tool_data);
} /* Interactive_tool_get_tool_data */

int Interactive_tool_handle_interactive_event(
	struct Interactive_tool *interactive_tool,void *device_id,
	struct Interactive_event *interactive_event,
	struct Graphics_buffer *graphics_buffer)
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Passes the <interactive_event> from <device_id> to the tool wrapped by the
<interactive_tool> object.
==============================================================================*/
{
	int return_code;

	ENTER(Interactive_tool_handle_interactive_event);
	if (interactive_tool&&interactive_event)
	{
		if (interactive_tool->interactive_event_handler)
		{
			(interactive_tool->interactive_event_handler)(device_id,interactive_event,
				interactive_tool->tool_data, graphics_buffer);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Interactive_tool_handle_interactive_event.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Interactive_tool_handle_interactive_event */

struct Cmgui_image *Interactive_tool_get_icon(struct Colour *foreground,
	struct Colour *background, struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Returns the icon which a user_interface can use to represent the tool.
==============================================================================*/
{
   struct Cmgui_image *image;

	ENTER(Interactive_tool_get_icon);
	if (interactive_tool)
	{
		image=(interactive_tool->get_icon_function)(foreground,
			background, interactive_tool->tool_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Interactive_tool_get_icon.  Invalid argument(s)");
		image=(struct Cmgui_image *)NULL;
	}
	LEAVE;

	return (image);
} /* Interactive_tool_get_icon */

int Interactive_tool_copy(struct Interactive_tool *destination_interactive_tool,
   struct Interactive_tool *source_interactive_tool,
	struct MANAGER(Interactive_tool) *destination_tool_manager)
/*******************************************************************************
LAST MODIFIED : 29 March 2007

DESCRIPTION :
Copies the settings of the <source_interactive_tool> into either the
<destination_interactive_tool> if that is set or creates a copy of the tool
if <destination_interactive_tool> is NULL and adds that to the manager.
==============================================================================*/
{
	int return_code;

	ENTER(Interactive_tool_copy);
	if (source_interactive_tool && destination_interactive_tool || destination_tool_manager)
	{
		if (destination_interactive_tool)
		{
			return_code = (source_interactive_tool->copy_function)(
				destination_interactive_tool->tool_data, source_interactive_tool->tool_data,
				destination_tool_manager);
		}
		else
		{
			return_code = (source_interactive_tool->copy_function)(
				NULL, source_interactive_tool->tool_data,
				destination_tool_manager);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Interactive_tool_copy.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Interactive_tool_copy */

int Interactive_tool_create_copy_iterator(struct Interactive_tool *interactive_tool,
   void *interactive_tool_manager_void)
/*******************************************************************************
LAST MODIFIED : 4 April 2007

DESCRIPTION :
An iterator which copies the <interactive_tool>, creating a new one of the same
type by calling Interactive_tool_copy and then adds this tool to the manager
pointed to by <interactive_tool_manager_void>.
==============================================================================*/
{
	int return_code;
	struct MANAGER(Interactive_tool) *new_interactive_tool_manager;

	ENTER(Interactive_tool_create_copy_iterator);
	if (interactive_tool && (new_interactive_tool_manager = 
			(struct MANAGER(Interactive_tool) *)interactive_tool_manager_void))
	{
		return_code = Interactive_tool_copy(
			(struct Interactive_tool *)NULL, interactive_tool,
			new_interactive_tool_manager);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Interactive_tool_create_copy_iterator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Interactive_tool_create_copy_iterator */

int Interactive_tool_bring_up_dialog(struct Interactive_tool *interactive_tool, struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
If the bring_up_dialog function is defined for <interactive_tool> calls it to
bring up the dialog for changing its settings.
==============================================================================*/
{
	int return_code;

	ENTER(Interactive_tool_bring_up_dialog);
	if (interactive_tool)
	{
		if (interactive_tool->bring_up_dialog_function)
		{
			return_code=(interactive_tool->bring_up_dialog_function)(interactive_tool->tool_data, graphics_window);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Interactive_tool_bring_up_dialog.  Invalid argument(s)");
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* Interactive_tool_bring_up_dialog */

void Interactive_tool_reset(struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 25 February 2008

DESCRIPTION :
Resets any current edits in the tool.
Call on old tool when a new tool is activated, esp. on asynchronous tool change.
==============================================================================*/
{
	ENTER(Interactive_tool_reset);
	if (interactive_tool)
	{
		if (interactive_tool->reset_function)
		{
			(interactive_tool->reset_function)(interactive_tool->tool_data);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Interactive_tool_bring_up_dialog.  Invalid argument(s)");
	}
	LEAVE;
} /* Interactive_tool_bring_up_dialog */

int Interactive_tool_name_to_array(
	struct Interactive_tool *interactive_tool,void *tool_name_address_void)
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
Allocates a copy of the name of <interactive_tool>, puts a pointer to it at
<**tool_name> and increments <*tool_name>.
==============================================================================*/
{
	char ***tool_name_address;
	int return_code;

	ENTER(Interactive_tool_name_to_array);
	if (interactive_tool&&(tool_name_address=(char ***)tool_name_address_void)&&
		(*tool_name_address))
	{
		if (GET_NAME(Interactive_tool)(interactive_tool,*tool_name_address))
		{
			(*tool_name_address)++;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Interactive_tool_name_to_array.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Interactive_tool_name_to_array.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Interactive_tool_name_to_array */

char **interactive_tool_manager_get_tool_names(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	int *number_of_tools,struct Interactive_tool *current_interactive_tool,
	char **current_tool_name)
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
Returns an array of strings containing the names of the tools - suitable for
choosing in a text command. On success, also returns <*number_of_tools>,
and in <current_tool_name> the pointer to the tool_names
string for <current_interactive_tool>, or the first one if it is not found.
Up to calling function to deallocate the returned array AND the strings in it.
==============================================================================*/
{
	char **tool_names,**tool_name;
	int i;

	ENTER(interactive_tool_manager_get_tool_names);
	tool_names=(char **)NULL;
	if (interactive_tool_manager&&number_of_tools&&current_tool_name)
	{
		if (0<(*number_of_tools=
			NUMBER_IN_MANAGER(Interactive_tool)(interactive_tool_manager)))
		{
			if (ALLOCATE(tool_names,char *,*number_of_tools))
			{
				for (i=0;i< *number_of_tools;i++)
				{
					tool_names[i]=(char *)NULL;
				}
				tool_name=tool_names;
				if (FOR_EACH_OBJECT_IN_MANAGER(Interactive_tool)(
					Interactive_tool_name_to_array,(void *)&tool_name,
					interactive_tool_manager))
				{
					*current_tool_name = tool_names[0];
					for (i=1;i<(*number_of_tools);i++)
					{
						if (FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(
							tool_names[i],interactive_tool_manager) ==
							current_interactive_tool)
						{
							*current_tool_name = tool_names[i];
						}
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"interactive_tool_manager_get_tool_names.  Failed");
					for (i=0;i<(*number_of_tools);i++)
					{
						if (tool_name[i])
						{
							DEALLOCATE(tool_names[i]);
						}
						DEALLOCATE(tool_names);
					}
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"interactive_tool_manager_get_tool_names.  Not enough memory");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"interactive_tool_manager_get_tool_names.  Invalid argument(s)");
	}
	LEAVE;

	return (tool_names);
} /* interactive_tool_manager_get_tool_names */
