/*******************************************************************************
FILE : api/cmiss_value_element_xi.c

LAST MODIFIED : 12 August 2003

DESCRIPTION :
The public interface to the Cmiss_value_element_xi object.
==============================================================================*/
#include "api/cmiss_finite_element.h"
#include "api/cmiss_value_element_xi.h"
#include "computed_variable/computed_value_finite_element.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

Cmiss_value_id CREATE(Cmiss_value_element_xi)(int dimension,
	struct Cmiss_element *element, float *xi_values)
/*******************************************************************************
LAST MODIFIED : 13 August 2003

DESCRIPTION :
Creates a Cmiss_value which contains an element_xi location.
==============================================================================*/
{
	Cmiss_value_id return_value;
	int local_dimension;

	ENTER(CREATE(Cmiss_value_element_xi));
	if ((element || (0<dimension)) && (return_value = CREATE(Cmiss_value)()))
	{
		ACCESS(Cmiss_value)(return_value);
		if (xi_values)
		{
			if (element)
			{
				local_dimension=get_FE_element_dimension(element);
				if ((0<dimension) && (dimension!=local_dimension))
				{
					/* error */
					local_dimension=0;
				}
			}
			else
			{
				local_dimension=dimension;
			}
			if (!Cmiss_value_element_xi_set_type(return_value, local_dimension, element,
				xi_values))
			{
				DEACCESS(Cmiss_value)(&return_value);
			}
		}
		else
		{
			if (!Cmiss_value_element_xi_set_type(return_value,dimension,element,
					 (FE_value *)NULL))
			{
				DEACCESS(Cmiss_value)(&return_value);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Cmiss_value_element_xi).  "
			"Invalid arguments.");
		return_value = (Cmiss_value_id)NULL;
	}
	LEAVE;

	return (return_value);
} /* CREATE(Cmiss_value_element_xi) */
