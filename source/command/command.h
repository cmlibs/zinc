/*******************************************************************************
FILE : command.h

LAST MODIFIED : 9 November 1998

DESCRIPTION :
Functions and types associated with commands.
==============================================================================*/
#if !defined (COMMAND_H)
#define COMMAND_H

#if defined (MOTIF)
#include <X11/Intrinsic.h>
#endif /* defined (MOTIF) */

/*
Global constants
----------------
*/
/*???DB.  Shouldn't be here ? */
/*#define CMGUI_EXAMPLE_DIRECTORY_SYMBOL "doc"*/
#define CMGUI_EXAMPLE_DIRECTORY_SYMBOL "example"

/*
Global types
------------
*/
typedef int (Execute_command_function)(char *command,void *user_data);

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
#if !defined (WINDOWS_DEV_FLAG)
void callback_command(Widget widget_id,XtPointer command,XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 11 June 1993

DESCRIPTION :
Allows easy execution of <command>s from menu buttons.
==============================================================================*/

int read_iod_file_via_selection_box(char *file_name, void *dummy);
/*******************************************************************************
LAST MODIFIED : 15 June 1993

DESCRIPTION :
==============================================================================*/
#endif /* !defined (WINDOWS_DEV_FLAG) */
#endif /* !defined (COMMAND_H) */
