/*******************************************************************************
FILE : api/cmiss_variable.c

LAST MODIFIED : 12 August 2003

DESCRIPTION :
The public interface to the Cmiss_variable object.
==============================================================================*/
#include "api/cmiss_variable.h"
#include "computed_variable/computed_variable.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

Cmiss_variable_id CREATE(Cmiss_variable_API)(char *name)
/*******************************************************************************
LAST MODIFIED : 12 August 2003

DESCRIPTION :
Creates a Cmiss_variable.
==============================================================================*/
{
	Cmiss_variable_id return_value;

	ENTER(CREATE(Cmiss_variable));
	if (name)
	{
		return_value = CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL,
			name);
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Cmiss_variable).  "
			"Invalid arguments.");
		return_value = (Cmiss_variable_id)NULL;
	}
	LEAVE;

	return (return_value);
} /* CREATE(Cmiss_variable) */
