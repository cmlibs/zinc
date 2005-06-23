/*******************************************************************************
FILE : api/cmiss_function_derivative.cpp

LAST MODIFIED : 11 May 2005

DESCRIPTION :
The public interface to the Cmiss_function derivative object.
==============================================================================*/

#include <new>
#include "api/cmiss_function_derivative.h"
#include "computed_variable/function_derivative.hpp"
#include "computed_variable/function_variable.hpp"

/*
Global functions
----------------
*/

Cmiss_function_id Cmiss_function_derivative_create(
	Cmiss_function_variable_id dependent_variable,
	Cmiss_function_variable_list_id independent_variables)
/*******************************************************************************
LAST MODIFIED : 17 January 2005

DESCRIPTION :
Creates a Cmiss_function derivative with the supplied <dependent_variable>
and <independent_variables>.
==============================================================================*/
{
	Cmiss_function_id result;
	Function_variable_handle *dependent_variable_handle_address;
	std::list<Function_variable_handle> *independent_variables_address;

	result=0;
	if ((dependent_variable_handle_address=reinterpret_cast<
		Function_variable_handle *>(dependent_variable))&&
		(independent_variables_address=reinterpret_cast< std::list<
		Function_variable_handle> *>(independent_variables)))
	{
		result=reinterpret_cast<Cmiss_function_id>(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			new Function_derivative_handle(new Function_derivative(
			*dependent_variable_handle_address,*independent_variables_address))
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			new Function_handle((*dependent_variable_handle_address)->derivative(
			*independent_variables_address))
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			);
	}

	return (result);
}
