/*******************************************************************************
FILE : api/cmiss_variable_new_derivative.cpp

LAST MODIFIED : 14 November 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new derivative object.
==============================================================================*/

#include <new>
#include "api/cmiss_variable_new_derivative.h"
#include "computed_variable/variable_derivative.hpp"

/*
Global functions
----------------
*/
Cmiss_variable_new_id Cmiss_variable_new_derivative_create(
	Cmiss_variable_new_id dependent_variable,
	Cmiss_variable_new_input_list_id independent_variables)
/*******************************************************************************
LAST MODIFIED : 14 November 2003

DESCRIPTION :
Creates a Cmiss_variable_new derivative with the supplied <dependent_variable>
and <independent_variables>.
==============================================================================*/
{
	Cmiss_variable_new_id result;
#if defined (USE_SMART_POINTER)
	Variable_handle *dependent_variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable_handle dependent_variable_handle;
#endif /* defined (USE_SMART_POINTER) */
	std::list<Variable_input_handle> *independent_variables_address;

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(dependent_variable_handle_address=reinterpret_cast<Variable_handle *>(
		dependent_variable))&&
#else /* defined (USE_SMART_POINTER) */
		(dependent_variable_handle=reinterpret_cast<Variable_handle>(
		dependent_variable))&&
#endif /* defined (USE_SMART_POINTER) */
		(independent_variables_address=
		reinterpret_cast<std::list<Variable_input_handle> *>(
    independent_variables)))
	{
		result=reinterpret_cast<Cmiss_variable_new_id>(
#if defined (USE_SMART_POINTER)
			new Variable_derivative_handle(new Variable_derivative(
			*dependent_variable_handle_address,*independent_variables_address))
#else /* defined (USE_SMART_POINTER) */
			new Variable_derivative(dependent_variable_handle,
			*independent_variables_address)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}
