/*******************************************************************************
FILE : command.h

LAST MODIFIED : 5 October 2001

DESCRIPTION :
Functions and types associated with commands.
==============================================================================*/
#if !defined (COMMAND_H)
#define COMMAND_H

#include "general/object.h"
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

struct Execute_command;

/*
Global functions
----------------
*/
#if defined (MOTIF)
void callback_command(Widget widget_id,XtPointer command,XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 11 June 1993

DESCRIPTION :
Allows easy execution of <command>s from menu buttons.
==============================================================================*/
#endif /* defined (MOTIF) */

int read_iod_file_via_selection_box(char *file_name,void *dummy);
/*******************************************************************************
LAST MODIFIED : 15 June 1993

DESCRIPTION :
==============================================================================*/

struct Execute_command *CREATE(Execute_command)(
	Execute_command_function *execute_command_function,
	void *command_function_data);
/*******************************************************************************
LAST MODIFIED : 8 December 1999

DESCRIPTION :
==============================================================================*/

int Execute_command_execute_string(struct Execute_command *execute_command,
	char *string);
/*******************************************************************************
LAST MODIFIED : 8 December 1999

DESCRIPTION :
Executes the given string using the Execute_command stucture
==============================================================================*/

int DESTROY(Execute_command)(struct Execute_command **execute_command_address);
/*******************************************************************************
LAST MODIFIED : 8 December 1999

DESCRIPTION :
==============================================================================*/

int execute_comfile(char *file_name,struct Execute_command *execute_command);
/******************************************************************************
LAST MODIFIED : 16 June 1996

DESCRIPTION :
Opens, executes and then closes a com file.  No window is created.
=============================================================================*/
#endif /* !defined (COMMAND_H) */
