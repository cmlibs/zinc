/*******************************************************************************
FILE : comfile.h

LAST MODIFIED : 29 June 2002

DESCRIPTION :
Commands and functions for comfiles.
==============================================================================*/
#if !defined (COMFILE_H)
#define COMFILE_H

#if defined (MOTIF)
#include "comfile/comfile_window.h"
#endif /* defined (MOTIF) */
#include "command/parser.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/

struct Open_comfile_data
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION :
==============================================================================*/
{
	char example_flag,*examples_directory,*example_symbol,*file_extension,
		*file_name;
	int execute_count;
	struct Execute_command *execute_command,*set_command;
#if defined (MOTIF)
	struct MANAGER(Comfile_window) *comfile_window_manager;
#endif /* defined (MOTIF) */
	struct User_interface *user_interface;
}; /* struct Open_comfile_data */

/*
Global functions
----------------
*/

int open_comfile(struct Parse_state *state,void *dummy_to_be_modified,
	void *open_comfile_data_void);
/*******************************************************************************
LAST MODIFIED : 29 June 2002

DESCRIPTION :
Opens a comfile, and a window if it is to be executed.  If a comfile is not
specified on the command line, a file selection box is presented to the user.
==============================================================================*/
#endif /* !defined (COMFILE_H) */
