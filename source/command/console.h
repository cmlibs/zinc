/*******************************************************************************
FILE : console.h

LAST MODIFIED : 27 June 2002

DESCRIPTION :
Definitions for command window structure, and associated functions
==============================================================================*/
#if !defined (CONSOLE_H)
#define CONSOLE_H

#include "command/command.h"
#include "general/object.h"
#include "user_interface/event_dispatcher.h"

/*
Global types
------------
*/
struct Console;

/*
Global functions
----------------
*/
struct Console *CREATE(Console)(struct Execute_command *execute_command, 
	struct Event_dispatcher *event_dispatcher, int file_descriptor);
/*******************************************************************************
LAST MODIFIED : 27 June 2002

DESCRIPTION :
==============================================================================*/

int DESTROY(Console)(struct Console **console_pointer);
/*******************************************************************************
LAST MODIFIED : 27 June 2002

DESCRIPTION:
==============================================================================*/

int Console_set_command_prompt(struct Console *console,
	char *prompt);
/*******************************************************************************
LAST MODIFIED : 27 June 2002

DESCRIPTION :
Sets the value of the <prompt> for the <console>.
==============================================================================*/
#endif /* !defined (CONSOLE_H) */
