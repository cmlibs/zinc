/*******************************************************************************
FILE : comfile_window.h

LAST MODIFIED : 3 September 2004

DESCRIPTION :
Definitions for the comfile window and structures.
==============================================================================*/
#if !defined (COMFILE_WINDOW_H)
#define COMFILE_WINDOW_H

#include <stddef.h>
#include "command/command.h"
#include "command/parser.h"
#include "general/io_stream.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "user_interface/user_interface.h"

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

/*
Global functions
----------------
*/

struct Comfile_window *CREATE(Comfile_window)(char *name,
	char *file_name, struct IO_stream_package *io_stream_package,
	struct Execute_command *execute_command,
	struct Execute_command *set_command,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 3 September 2004

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
#endif /* !defined (COMFILE_WINDOW_H) */
