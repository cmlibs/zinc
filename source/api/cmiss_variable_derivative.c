/*******************************************************************************
FILE : api/cmiss_variable_derivative.c

LAST MODIFIED : 14 August 2003

DESCRIPTION :
The public interface to the Cmiss_variable_derivative object.
==============================================================================*/
#include "api/cmiss_variable_derivative.h"
#include "computed_variable/computed_variable_derivative.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

Cmiss_variable_id CREATE(Cmiss_variable_derivative)(char *name,
	Cmiss_variable_id dependent_variable, int order,
	Cmiss_variable_id *independent_variables)
/*******************************************************************************
LAST MODIFIED : 14 August 2003

DESCRIPTION :
Creates a Cmiss_variable which represents the derivative of the dependent variable
with respect to the independent variables.  There must be <order> variables in
the <independent_variables> array.
==============================================================================*/
{
	Cmiss_variable_id return_variable;

	ENTER(CREATE(Cmiss_variable_derivative));
	return_variable = (Cmiss_variable_id)NULL;
	if (name && dependent_variable && (order > 0) && independent_variables)
	{
		if (return_variable=CREATE(Cmiss_variable)(
			(struct Cmiss_variable_package *)NULL,name))
		{
			ACCESS(Cmiss_variable)(return_variable);
			if (!Cmiss_variable_derivative_set_type(return_variable,
				dependent_variable, order, independent_variables))
			{
				DEACCESS(Cmiss_variable)(&return_variable);
			}
		}
	}
	LEAVE;

	return (return_variable);
} /* CREATE(Cmiss_variable_derivative) */
