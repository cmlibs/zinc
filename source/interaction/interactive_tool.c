/*******************************************************************************
FILE : interactive_tool.c

LAST MODIFIED : 11 May 2000

DESCRIPTION :
Active CMGUI tools should create a wrapper Interactive_tool that supplies a
consistent way of choosing which one is to receive input from a given input
device, and listing which ones are active.
Only certain such tools will be capable of receiving input events from scene
viewer/mouse input and other devices, but many more will follow and change the
content of the global selections and objects with text input.
==============================================================================*/
#include "general/debug.h"
#include "general/list_private.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "interaction/interactive_tool.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Interactive_tool
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Structure describing an interactive event (eg. button press at point in space).
ACCESS this object for as long as you need to keep it; it is not modifiable.
==============================================================================*/
{
	/* name identifier for the tool and display name for presentation */
	char *display_name,*name;
	Interactive_event_handler *interactive_event_handler;
	Interactive_tool_make_button_function *make_button_function;
	Interactive_tool_bring_up_dialog_function *bring_up_dialog_function;
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

struct Interactive_tool *CREATE(Interactive_tool)(char *name,char *display_name,
	Interactive_event_handler *interactive_event_handler,
	Interactive_tool_make_button_function *make_button_function,
	Interactive_tool_bring_up_dialog_function *bring_up_dialog_function,
	void *tool_data)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Creates an Interactive_tool with the given <name> and <icon>. If an
<interactive_event_handler> is supplied it is called to pass on any input
events to the interactive_tool, with the <tool_data> passed as the third
parameter.
==============================================================================*/
{
	struct Interactive_tool *interactive_tool;

	ENTER(CREATE(Interactive_tool));
	if (name&&display_name&&make_button_function)
	{
		if (ALLOCATE(interactive_tool,struct Interactive_tool,1)&&
			(interactive_tool->name=duplicate_string(name))&&
			(interactive_tool->display_name=duplicate_string(display_name)))
		{
			interactive_tool->make_button_function=make_button_function;
			interactive_tool->bring_up_dialog_function=bring_up_dialog_function;
			interactive_tool->interactive_event_handler=interactive_event_handler;
			interactive_tool->tool_data=tool_data;
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

int Interactive_tool_handle_interactive_event(
	struct Interactive_tool *interactive_tool,void *device_id,
	struct Interactive_event *interactive_event)
/*******************************************************************************
LAST MODIFIED : 10 April 2000

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
				interactive_tool->tool_data);
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

Widget Interactive_tool_make_button(struct Interactive_tool *interactive_tool,
	Widget parent)
/*******************************************************************************
LAST MODIFIED : 8 April 2000

DESCRIPTION :
Makes and returns a toggle_button widget representing <interactive_tool> as a
child of <parent>. <parent> is expected to be a RowColumn widget with an entry
callback receiving value changes from the toggle_button.
==============================================================================*/
{
	Widget widget;

	ENTER(Interactive_tool_make_button);
	if (interactive_tool)
	{
		widget=(interactive_tool->make_button_function)(
			interactive_tool->tool_data,parent);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Interactive_tool_make_button.  Invalid argument(s)");
		widget=(Widget)NULL;
	}
	LEAVE;

	return (widget);
} /* Interactive_tool_make_button */

int Interactive_tool_bring_up_dialog(struct Interactive_tool *interactive_tool)
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
			return_code=(interactive_tool->bring_up_dialog_function)(
				interactive_tool->tool_data);
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
LAST MODIFIED : 13 June 2000

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
		*number_of_tools=
			NUMBER_IN_MANAGER(Interactive_tool)(interactive_tool_manager);
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
					if (FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(tool_names[i],
						interactive_tool_manager) == current_interactive_tool)
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
	else
	{
		display_message(ERROR_MESSAGE,
			"interactive_tool_manager_get_tool_names.  Invalid argument(s)");
	}
	LEAVE;

	return (tool_names);
} /* interactive_tool_manager_get_tool_names */
