/*******************************************************************************
FILE : api/cmiss_variable_composite.c

LAST MODIFIED : 1 September 2003

DESCRIPTION :
The public interface to the Cmiss_variable_composite object.
==============================================================================*/
#include "api/cmiss_variable_composite.h"
#include "computed_variable/computed_variable_composite.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

Cmiss_variable_id CREATE(Cmiss_variable_composite)(char *name,
	int number_of_variables, Cmiss_variable_id *variables_array)
/*******************************************************************************
LAST MODIFIED : 1 September 2003

DESCRIPTION :
Creates a Cmiss_variable which represents a composite concatenation of serveral
other variables.
==============================================================================*/
{
	Cmiss_variable_id return_variable;

	ENTER(CREATE(Cmiss_variable_composite));
	return_variable = (Cmiss_variable_id)NULL;
	if (name)
	{
		if (return_variable=CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL,name))
		{
			ACCESS(Cmiss_variable)(return_variable);
			if (!Cmiss_variable_composite_set_type(return_variable, number_of_variables,
				variables_array))
			{
				DEACCESS(Cmiss_variable)(&return_variable);
			}
		}
	}
	LEAVE;

	return (return_variable);
} /* CREATE(Cmiss_variable_composite) */
