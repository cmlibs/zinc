/*******************************************************************************
FILE : command.c

LAST MODIFIED : 9 November 1998

DESCRIPTION :
Functions associated with commands.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "command/command.h"
#include "general/debug.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Global functions
----------------
*/
#if !defined (WINDOWS_DEV_FLAG)
void callback_command(Widget widget,XtPointer string,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1993

DESCRIPTION :
Allows easy execution of command <string>s from menu buttons.
==============================================================================*/
{
	char *command_string;
	struct Execute_command *execute_command;

	ENTER(callback_command);
	if (widget&&(command_string=(char *)string))
	{
		execute_command=(struct Execute_command *)NULL;
		XtVaGetValues(widget,XmNuserData,(XtPointer)(&execute_command),NULL);
		if (execute_command)
		{
			(*(execute_command->function))(command_string,execute_command->data);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"callback_command.  Missing execute_command: %s",command_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"callback_command.  Invalid argument(s)");
	}
	LEAVE;
} /* callback_command */
#endif /* !defined (WINDOWS_DEV_FLAG) */

#if !defined (WINDOWS_DEV_FLAG)
int read_iod_file_via_selection_box(char *file_name,void *execute_command_void)
/*******************************************************************************
LAST MODIFIED : 5 June 1996

DESCRIPTION:
Submits a command to open an iod file.
==============================================================================*/
{
	char *command_string,*file_extension;
	int length,return_code;
	struct Execute_command *execute_command;

	ENTER(read_iod_file_via_selection_box);
	/* check arguments */
	if (file_name&&
		(execute_command=(struct Execute_command *)execute_command_void)&&
		(execute_command->function))
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
			return_code=(*(execute_command->function))(command_string,
				execute_command->data);
			DEALLOCATE(command_string);
		}
		else
		{
			display_message(ERROR_MESSAGE,
"read_iod_file_via_selection_box.  Could not allocate memory for command string"
				);
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
#endif /* !defined (WINDOWS_DEV_FLAG) */
