/*******************************************************************************
FILE : comfile_window.h

LAST MODIFIED : 18 April 2002

DESCRIPTION :
Definitions for the comfile window and structures.
==============================================================================*/
#if !defined (COMFILE_WINDOW_H)
#define COMFILE_WINDOW_H

#include <stddef.h>
#include "command/command.h"
#include "command/parser.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"

/*
Global types
------------
*/

struct Comfile_window;
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION :
Command file or "comfile" window structure which lists a cmiss command file and
by user selection passes commands to the command window.
The contents of this structure are private.
==============================================================================*/

DECLARE_LIST_TYPES(Comfile_window);

DECLARE_MANAGER_TYPES(Comfile_window);

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
	struct MANAGER(Comfile_window) *comfile_window_manager;
	struct User_interface *user_interface;
}; /* struct Open_comfile_data */

/*
Global functions
----------------
*/

struct Comfile_window *CREATE(Comfile_window)(char *name,
	char *file_name,
	struct Execute_command *execute_command,
	struct Execute_command *set_command,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION :
Creates the structures and retrieves a comfile window widget from the Motif
resource manager hierarchy.
==============================================================================*/

int DESTROY(Comfile_window)(struct Comfile_window **comfile_window_address);
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION:
Frees the contents of the Comfile_window structure and then the object itself,
then closes down the window shell and widgets it uses. Note that responsibility
for removing the comfile_window from a global list of windows is left with the
calling routine. See also Comfile_window_close_CB and
Comfile_window_destroy_CB.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Comfile_window);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Comfile_window);

PROTOTYPE_LIST_FUNCTIONS(Comfile_window);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Comfile_window,name,char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Comfile_window,name,char *);
PROTOTYPE_MANAGER_FUNCTIONS(Comfile_window);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Comfile_window,name,char *);

char *Comfile_window_manager_make_unique_name(
	struct MANAGER(Comfile_window) *comfile_window_manager, char *file_name);
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION :
Allocates and returns a name based on <file_name> that is not currently in use
in the <comfile_window_manager>. Does so by appending a number in angle
brackets.
Up to the calling routine to deallocate the returned string.
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
