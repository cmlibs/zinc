/*******************************************************************************
FILE : comfile_window.h

LAST MODIFIED : 27 April 1999

DESCRIPTION :
Definitions for the comfile window and structures.
==============================================================================*/
#if !defined (COMFILE_WINDOW_H)
#define COMFILE_WINDOW_H

#include <stddef.h>
#include "command/command.h"
#include "command/parser.h"

/*
Global types
------------
*/
struct Comfile_window
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
Information required for a comfile window.
???DB.  Should there be a "window_structure" which contains shell information
etc ?
==============================================================================*/
{
	char *file_name;
	int number_of_commands;
	char **commands;
	Widget command_list;
	Widget window;
	Widget shell;
	struct User_interface *user_interface;
	/* for executing commands */
	struct Execute_command *execute_command;
	/* for setting command text ready to edit and enter */
	struct Execute_command *set_command;
}; /* struct Comfile_window */

struct Open_comfile_data
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
==============================================================================*/
{
	char example_flag,*examples_directory,*example_symbol,*file_extension,
		*file_name;
	int execute_count;
	struct Execute_command *execute_command,*set_command;
	struct User_interface *user_interface;
}; /* struct Open_comfile_data */

/*
Global functions
----------------
*/
struct Comfile_window *create_Comfile_window(char *file_name,
	struct Execute_command *execute_command,
	struct Execute_command *set_command,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
Creates the structures and retrieves a comfile window widget from the Motif
resource manager hierarchy.
==============================================================================*/

int execute_comfile(char *file_name,struct Execute_command *execute_command);
/******************************************************************************
LAST MODIFIED : 16 June 1996

DESCRIPTION :
Opens, executes and then closes a com file.  No window is created.
=============================================================================*/

int open_comfile(struct Parse_state *state,void *dummy_to_be_modified,
	void *open_comfile_data_void);
/*******************************************************************************
LAST MODIFIED : 25 February 1997

DESCRIPTION :
Opens a comfile, and a window if it is to be executed.  If a comfile is not
specified on the command line, a file selection box is presented to the user.
==============================================================================*/

#if defined (OLD_CODE)
int open_comfile(struct Parse_state *state,
	struct Execute_command *execute_command,
	struct Modifier_entry *set_file_name_option_table,
	struct User_interface *user_interface);
/******************************************************************************
LAST MODIFIED : 24 September 1996

DESCRIPTION :
Opens a comfile, and a window if it is to be executed.  If a comfile is not
specified on the command line, a file selection box is presented to the user.
=============================================================================*/

int open_comfile_via_selection_box(char *file_name,void *execute_command_void);
/*******************************************************************************
LAST MODIFIED : 5 June 1996

DESCRIPTION :
Submits a command to open a comfile.
==============================================================================*/
#endif /* defined (OLD_CODE) */
#endif /* !defined (COMFILE_WINDOW_H) */
