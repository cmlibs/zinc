/*******************************************************************************
FILE : cmiss_command.c

LAST MODIFIED : 13 August 2003

DESCRIPTION :
The public interface to the some of the internal functions of cmiss.
==============================================================================*/
#include <stdlib.h>
#include "api/cmiss_command.h"
#include "command/cmiss.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

int Cmiss_command_data_execute(struct Cmiss_command_data *command_data,
	char *command)
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
Parses the supplied <command> using cmiss on command parser, therefore the
string should only contain valid "gfx" or "fem" syntax.
==============================================================================*/
{
	int quit, return_code;

	ENTER(Cmiss_command_data_execute);
	if (command)
	{
		quit = 0;
		return_code = 1;
		execute_command(command, (void *)command_data, &quit, &return_code);
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_command_data_execute */

