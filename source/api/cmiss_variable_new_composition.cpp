/*******************************************************************************
FILE : api/cmiss_variable_new_composition.cpp

LAST MODIFIED : 20 November 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new composition object.
==============================================================================*/

#include <new>
#include "api/cmiss_variable_new_composition.h"
#include "computed_variable/variable_composition.hpp"

/*
Global functions
----------------
*/
Cmiss_variable_new_id Cmiss_variable_new_composition_create(
	Cmiss_variable_new_id dependent_variable,
	Cmiss_variable_new_input_value_list_id input_source_list)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Creates a Cmiss_variable_new composite with the supplied <dependent_variable>
and inputs <input_source_list>.
==============================================================================*/
{
	Cmiss_variable_new_id result;
#if defined (USE_SMART_POINTER)
	Variable_handle *dependent_variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable_handle dependent_variable_handle;
#endif /* defined (USE_SMART_POINTER) */
	std::list<Variable_input_value_handle> *input_source_list_address;

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(dependent_variable_handle_address=reinterpret_cast<Variable_handle *>(
		dependent_variable))&&
#else /* defined (USE_SMART_POINTER) */
		(dependent_variable_handle=reinterpret_cast<Variable_handle>(
		dependent_variable))&&
#endif /* defined (USE_SMART_POINTER) */
		(input_source_list_address=reinterpret_cast<
		std::list<Variable_input_value_handle> *>(input_source_list)))
	{
		result=reinterpret_cast<Cmiss_variable_new_id>(
#if defined (USE_SMART_POINTER)
			new Variable_composition_handle(new Variable_composition(
			*dependent_variable_handle_address,*input_source_list_address))
#else /* defined (USE_SMART_POINTER) */
			new Variable_composition(*dependent_variable_handle_address,
			*input_source_list_address)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}
