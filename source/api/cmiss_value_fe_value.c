/*******************************************************************************
FILE : api/cmiss_value_fe_value.c

LAST MODIFIED : 20 August 2003

DESCRIPTION :
The public interface to the Cmiss_value_FE_value object.
==============================================================================*/
#include "api/cmiss_finite_element.h"
#include "api/cmiss_value_fe_value.h"
#include "computed_variable/computed_value_fe_value.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

Cmiss_value_id CREATE(Cmiss_value_FE_value)(FE_value value)
/*******************************************************************************
LAST MODIFIED : 20 August 2003

DESCRIPTION :
Creates a Cmiss_value which contains an FE_value location.
==============================================================================*/
{
	Cmiss_value_id return_value;

	ENTER(CREATE(Cmiss_value_FE_value));
	if (return_value = CREATE(Cmiss_value)())
	{
		ACCESS(Cmiss_value)(return_value);
		if (!Cmiss_value_FE_value_set_type(return_value, value))
		{
			DEACCESS(Cmiss_value)(&return_value);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Cmiss_value_FE_value).  "
			"Invalid arguments.");
		return_value = (Cmiss_value_id)NULL;
	}
	LEAVE;

	return (return_value);
} /* CREATE(Cmiss_value_FE_value) */

Cmiss_value_id CREATE(Cmiss_value_FE_value_vector)(int number_of_values,
	FE_value *values)
/*******************************************************************************
LAST MODIFIED : 20 August 2003

DESCRIPTION :
Creates a Cmiss_value which contains a vector of FE_values.
==============================================================================*/
{
	Cmiss_value_id return_value;

	ENTER(CREATE(Cmiss_value_FE_value_vector));
	if (return_value = CREATE(Cmiss_value)())
	{
		ACCESS(Cmiss_value)(return_value);
		if (!Cmiss_value_FE_value_vector_set_type(return_value, number_of_values,
			values))
		{
			DEACCESS(Cmiss_value)(&return_value);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Cmiss_value_FE_value_vector).  "
			"Invalid arguments.");
		return_value = (Cmiss_value_id)NULL;
	}
	LEAVE;

	return (return_value);
} /* CREATE(Cmiss_value_FE_value_vector) */
