/*******************************************************************************
FILE : api/cmiss_variable_composition.c

LAST MODIFIED : 28 August 2003

DESCRIPTION :
The public interface to the Cmiss_variable_composition object.
==============================================================================*/
#include "api/cmiss_variable_composition.h"
#include "computed_variable/computed_variable_composition.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

Cmiss_variable_id CREATE(Cmiss_variable_composition)(char *name,
	Cmiss_variable_id dependent_variable,int number_of_source_variables,
	Cmiss_variable_id *source_variables,Cmiss_variable_id *independent_variables)
/*******************************************************************************
LAST MODIFIED : 28 August 2003

DESCRIPTION :
Creates a Cmiss_variable which represents a coordinate of the specified <dimension>.
==============================================================================*/
{
	Cmiss_variable_id return_variable;

	ENTER(CREATE(Cmiss_variable_composition));
	return_variable = (Cmiss_variable_id)NULL;
	if (name)
	{
		if (return_variable=CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL,name))
		{
			ACCESS(Cmiss_variable)(return_variable);
			if (!Cmiss_variable_composition_set_type(return_variable, dependent_variable,
				number_of_source_variables, source_variables, independent_variables))
			{
				DEACCESS(Cmiss_variable)(&return_variable);
			}
		}
	}
	LEAVE;

	return (return_variable);
} /* CREATE(Cmiss_variable_composition) */

