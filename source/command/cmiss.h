/*******************************************************************************
FILE : cmiss.h

LAST MODIFIED : 12 August 2002

DESCRIPTION :
Functions and types for executing cmiss commands.

This should only be included in cmgui.c and command/cmiss.c
==============================================================================*/
#if !defined (COMMAND_CMISS_H)
#define COMMAND_CMISS_H

#include "command/command.h"
#include "finite_element/finite_element.h"
#include "graphics/graphics_object.h"

/*
Global types
------------
*/
struct Cmiss_command_data;
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Shifted the Cmiss_command_data to be internal to cmiss.c
==============================================================================*/

/*
Global functions
----------------
*/

int cmiss_execute_command(char *command_string,void *command_data);
/*******************************************************************************
LAST MODIFIED : 9 November 1998

DESCRIPTION :
Execute a <command_string>.
==============================================================================*/

#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
void execute_command(char *command_string,void *command_data_void, int *quit,
  int *error);
/*******************************************************************************
LAST MODIFIED : 28 March 2000

DESCRIPTION:
==============================================================================*/
#endif /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */

int cmiss_set_command(char *command_string,void *command_data_void);
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION:
Sets the <command_string> in the command box of the CMISS command_window, ready
for editing or entering. If there is no command_window, does nothing.
==============================================================================*/

struct Cmiss_command_data *CREATE(Cmiss_command_data)(int argc,char *argv[],
	char *version_string);
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Initialise all the subcomponents of cmgui and create the Cmiss_command_data
==============================================================================*/

int Cmiss_command_data_main_loop(struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Process events until some events request the program to finish.
==============================================================================*/

int DESTROY(Cmiss_command_data)(struct Cmiss_command_data **command_data_address);
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Clean up the command_data, deallocating all the associated memory and resources.
==============================================================================*/

#endif /* !defined (COMMAND_CMISS_H) */
