/*******************************************************************************
FILE : help_interface.h

LAST MODIFIED : 19 November 1998

DESCRIPTION :
Interface routines for CMISS commands to work with the help window.
==============================================================================*/
#if !defined (HELP_INTERFACE_H)
#define HELP_INTERFACE_H

#include "command/command.h"
#include "user_interface/user_interface.h"

/*
Global functions
----------------
*/
void do_help(char *help_string,char *help_examples_directory,
	struct Execute_command *execute_command,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
Gets help for help_string.
If the help window hasn't been created,it creates it.
If the help window is popped down it pops it up.
Then it sets the help topic edit field and gets the help on the string.
==============================================================================*/

void pop_down_help(void);
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Pops down the help window.
==============================================================================*/

void destroy_help(void);
/*******************************************************************************
LAST MODIFIED : 19 February 1995

DESCRIPTION :
Destroys the help window.
==============================================================================*/
#endif /* !defined (HELP_INTERFACE_H) */
