/*******************************************************************************
FILE : cmiss_command.c

LAST MODIFIED : 5 April 2004

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
LAST MODIFIED : 5 April 2004

DESCRIPTION :
Parses the supplied <command> using the command parser interpreter.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_command_data_execute);
	if (command)
	{
		return_code = cmiss_execute_command(command, (void *)command_data);
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_command_data_execute */

