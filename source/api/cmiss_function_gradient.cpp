/*******************************************************************************
FILE : api/cmiss_function_gradient.cpp

LAST MODIFIED : 26 August 2004

DESCRIPTION :
The public interface to the Cmiss_function gradient object.
==============================================================================*/

#include <new>
#include "api/cmiss_function_gradient.h"
#include "computed_variable/function_gradient.hpp"

/*
Global functions
----------------
*/

Cmiss_function_id Cmiss_function_gradient_create(
	Cmiss_function_variable_id dependent_variable,
	Cmiss_function_variable_id independent_variable)
/*******************************************************************************
LAST MODIFIED : 26 August 2004

DESCRIPTION :
Creates a Cmiss_function gradient with the supplied <dependent_variable> and
<independent_variable>.
==============================================================================*/
{
	Cmiss_function_id result;
	Function_variable_handle *dependent_variable_handle_address,
		*independent_variable_handle_address;

	result=0;
	if ((dependent_variable_handle_address=reinterpret_cast<
		Function_variable_handle *>(dependent_variable))&&
		(independent_variable_handle_address=reinterpret_cast<
		Function_variable_handle *>(independent_variable)))
	{
		result=reinterpret_cast<Cmiss_function_id>(
			new Function_gradient_handle(new Function_gradient(
			*dependent_variable_handle_address,
			*independent_variable_handle_address)));
	}

	return (result);
}
