/*******************************************************************************
FILE : api/cmiss_value_matrix.c

LAST MODIFIED : 12 August 2003

DESCRIPTION :
The public interface to the Cmiss_value_matrix object.
==============================================================================*/
#include "api/cmiss_value_derivative_matrix.h"
#include "computed_variable/computed_value_derivative_matrix.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

Cmiss_value_id CREATE(Cmiss_value_derivative_matrix)(Cmiss_variable_id dependent_variable,
	int order, Cmiss_variable_id *independent_variables,
	int number_of_matrices, Cmiss_value_id *matrices)
/*******************************************************************************
LAST MODIFIED : 12 August 2003

DESCRIPTION :
Creates a Cmiss_value which contains a set of derivative matrices.
==============================================================================*/
{
	Cmiss_value_id return_value;
	int expected_number_of_matrices, i;

	ENTER(CREATE(Cmiss_value_derivative_matrix));
	expected_number_of_matrices=0;
	for (i = 0 ; i < order ; i++)
	{
		expected_number_of_matrices=2*expected_number_of_matrices+1;
	}
	if (dependent_variable && (order > 0) && independent_variables
		 && (number_of_matrices == expected_number_of_matrices) && matrices)
	{
		if (return_value=CREATE(Cmiss_value)())
		{
			ACCESS(Cmiss_value)(return_value);
			if (!Cmiss_value_derivative_matrix_set_type(return_value,
				dependent_variable,order,independent_variables,matrices))
			{
				DEACCESS(Cmiss_value)(&return_value);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(Cmiss_value_derivative_matrix).  "
				"Unable to Create(Cmiss_value).");
			return_value = (Cmiss_value_id)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Cmiss_value_derivative_matrix).  "
			"Invalid arguments.");
		return_value = (Cmiss_value_id)NULL;
	}
	LEAVE;

	return (return_value);
} /* CREATE(Cmiss_value_derivative_matrix) */
