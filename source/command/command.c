/*******************************************************************************
FILE : command.c

LAST MODIFIED : 15 July 2002

DESCRIPTION :
Functions associated with commands.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "command/command.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/

struct Execute_command
/*******************************************************************************
LAST MODIFIED : 5 June 1996

DESCRIPTION :
==============================================================================*/
{
	Execute_command_function *function;
	void *data;
}; /* struct Execute_command */

/*
Global functions
----------------
*/

#if defined (MOTIF)
void callback_command(Widget widget, XtPointer string, XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 July 2002

DESCRIPTION :
Allows easy execution of command <string>s from menu buttons.
==============================================================================*/
{
	char *command_string;
	struct Execute_command *execute_command;

	ENTER(callback_command);
	USE_PARAMETER(call_data);
	if (widget && (command_string = (char *)string))
	{
		execute_command=(struct Execute_command *)NULL;
		XtVaGetValues(widget, XmNuserData, (XtPointer)(&execute_command), NULL);
		Execute_command_execute_string(execute_command, command_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,"callback_command.  Invalid argument(s)");
	}
	LEAVE;
} /* callback_command */
#endif /* defined (MOTIF) */

int read_iod_file_via_selection_box(char *file_name,void *execute_command_void)
/*******************************************************************************
LAST MODIFIED : 15 July 2002

DESCRIPTION:
Submits a command to open an iod file.
==============================================================================*/
{
	char *command_string,*file_extension;
	int length,return_code;
	struct Execute_command *execute_command;

	ENTER(read_iod_file_via_selection_box);
	/* check arguments */
	if (file_name &&
		(execute_command=(struct Execute_command *)execute_command_void))
	{
		/* remove the file extension */
		if (file_extension=strrchr(file_name,'.'))
		{
			length=file_extension-file_name;
		}
		else
		{
			length=strlen(file_name);
		}
		if (ALLOCATE(command_string,char,length+14))
		{
			strcpy(command_string,"FEM read ");
			strncpy(command_string+9,file_name,length);
			strcpy(command_string+(9+length),";iod");
			return_code =
				Execute_command_execute_string(execute_command, command_string);
			DEALLOCATE(command_string);
		}
		else
		{
			display_message(ERROR_MESSAGE,"read_iod_file_via_selection_box.  "
				"Could not allocate memory for command string");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_iod_file_via_selection_box.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_iod_file_via_selection_box */

struct Execute_command *CREATE(Execute_command)(void)
/*******************************************************************************
LAST MODIFIED : 15 July 2002

DESCRIPTION :
Creates a blank execute command. Must call Execute_command_set_command_function
to set the function it calls, and the user data to be passed with it.
==============================================================================*/
{
	struct Execute_command *execute_command;

	ENTER(CREATE(Execute_command));
	if (ALLOCATE(execute_command,struct Execute_command, 1))
	{
		execute_command->function = (Execute_command_function *)NULL;
		execute_command->data = (void *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Execute_command).  "
			"Unable to allocate Execute_command structure");
		execute_command = (struct Execute_command *)NULL;
	}
	LEAVE;

	return (execute_command);
} /* CREATE(Execute_command) */

int DESTROY(Execute_command)(struct Execute_command **execute_command_address)
/*******************************************************************************
LAST MODIFIED : 5 October 2001

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Execute_command));
	return_code=0;
	if (execute_command_address&&(*execute_command_address))
	{
		DEALLOCATE(*execute_command_address);
		*execute_command_address=(struct Execute_command *)NULL;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Execute_command).  Missing execute_command");
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Execute_command) */

int Execute_command_set_command_function(
	struct Execute_command *execute_command,
	Execute_command_function *execute_command_function,
	void *command_function_data)
/*******************************************************************************
LAST MODIFIED : 15 July 2002

DESCRIPTION :
Sets the function called by <execute_command>, and the user data to be passed
with it.
==============================================================================*/
{
	int return_code;

	ENTER(Execute_command_set_command_function);
	if (execute_command && execute_command_function)
	{
		execute_command->function = execute_command_function;
		execute_command->data = command_function_data;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Execute_command_set_command_function.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Execute_command_set_command_function */

int Execute_command_execute_string(struct Execute_command *execute_command,
	char *command_string)
/*******************************************************************************
LAST MODIFIED : 15 July 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Execute_command_execute_string);
	return_code = 0;
	if (execute_command)
	{
		if (command_string)
		{
			if (execute_command->function)
			{
				return_code =
					(*(execute_command->function))(command_string, execute_command->data);
			}
			else
			{
				display_message(ERROR_MESSAGE, "Execute_command_execute_string.  "
					"Missing function for executing '%s'",command_string);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Execute_command_execute_string.  Missing command string");
		}
	}
	else
	{
		if (command_string)
		{
			display_message(ERROR_MESSAGE, "Execute_command_execute_string.  "
				"Missing Execute command for '%s'",command_string);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Execute_command_execute_string.  Missing Execute command");
		}
	}
	LEAVE;

	return (return_code);
} /* Execute_command_execute_string */

int execute_comfile(char *file_name,struct IO_stream_package *io_stream_package,
	struct Execute_command *execute_command)
/******************************************************************************
LAST MODIFIED : 3 September 2004

DESCRIPTION :
Opens, executes and then closes a com file.  No window is created.
=============================================================================*/
{
	char *command_string;
	int return_code;
	struct IO_stream *comfile;

	ENTER(execute_comfile);
	if (file_name)
	{
		if (execute_command)
		{
			if ((comfile=CREATE(IO_stream)(io_stream_package)) &&
				IO_stream_open_for_read(comfile, file_name))
			{
				IO_stream_scan(comfile," ");
				while (!IO_stream_end_of_stream(comfile)&&
					(IO_stream_read_string(comfile,"[^\n]",&command_string)))
				{
					Execute_command_execute_string(execute_command, command_string);
					DEALLOCATE(command_string);
					IO_stream_scan(comfile," ");
				}
				IO_stream_close(comfile);
				DESTROY(IO_stream)(&comfile);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"Could not open: %s",file_name);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"execute_comfile.  "
				"Invalid execute command");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_comfile.  Missing file name");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_comfile */

