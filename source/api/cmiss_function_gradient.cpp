/*******************************************************************************
FILE : api/cmiss_function_gradient.cpp

LAST MODIFIED : 21 October 2004

DESCRIPTION :
The public interface to the Cmiss_function gradient object - composition of
derivative and transpose.
==============================================================================*/

#include <new>
#include "api/cmiss_function_derivative.h"
#include "api/cmiss_function_gradient.h"
#include "api/cmiss_function_matrix_transpose.h"

/*
Global functions
----------------
*/

Cmiss_function_id Cmiss_function_gradient_create(
	Cmiss_function_variable_id dependent_variable,
	Cmiss_function_variable_id independent_variable)
/*******************************************************************************
LAST MODIFIED : 21 October 2004

DESCRIPTION :
Creates a Cmiss_function gradient with the supplied <dependent_variable> and
<independent_variable>.
==============================================================================*/
{
	Cmiss_function_id derivative,result;
	Cmiss_function_variable_list_id independent_variables;

	result=0;
	independent_variables=Cmiss_function_variable_list_create();
	if (independent_variables&&Cmiss_function_variable_list_add(
		independent_variables,independent_variable))
	{
		if (derivative=Cmiss_function_derivative_create(dependent_variable,
			independent_variables))
		{
			if (!(result=Cmiss_function_matrix_transpose_create(Cmiss_function_output(
				derivative))))
			{
				Cmiss_function_destroy(&derivative);
			}
		}
	}
	Cmiss_function_variable_list_destroy(&independent_variables);

	return (result);
}
