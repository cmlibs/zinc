/*******************************************************************************
FILE : api/cmiss_variable_identity.c

LAST MODIFIED : 1 September 2003

DESCRIPTION :
The public interface to the Cmiss_variable_identity object.
==============================================================================*/
#include "api/cmiss_variable_identity.h"
#include "computed_variable/computed_variable_identity.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

Cmiss_variable_id CREATE(Cmiss_variable_identity)(char *name,
	Cmiss_variable_id variable)
/*******************************************************************************
LAST MODIFIED : 1 September 2003

DESCRIPTION :
Creates a Cmiss_variable which represents an identity.
==============================================================================*/
{
	Cmiss_variable_id return_variable;

	ENTER(CREATE(Cmiss_variable_identity));
	return_variable = (Cmiss_variable_id)NULL;
	if (name)
	{
		if (return_variable=CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL,name))
		{
			ACCESS(Cmiss_variable)(return_variable);
			if (!Cmiss_variable_identity_set_type(return_variable, variable))
			{
				DEACCESS(Cmiss_variable)(&return_variable);
			}
		}
	}
	LEAVE;

	return (return_variable);
} /* CREATE(Cmiss_variable_identity) */
