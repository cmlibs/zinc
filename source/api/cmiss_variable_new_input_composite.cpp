/*******************************************************************************
FILE : api/cmiss_variable_new_input_composite.cpp

LAST MODIFIED : 21 October 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new basic objects.
==============================================================================*/

#include <new>
#include "api/cmiss_variable_new_input_composite.h"
#include "computed_variable/variable_input_composite.hpp"

/*
Global functions
----------------
*/
Cmiss_variable_new_input_id Cmiss_variable_new_input_composite_create(
	Cmiss_variable_new_input_value_list_id inputs)
/*******************************************************************************
LAST MODIFIED : 20 October 2003

DESCRIPTION :
Creates a Cmiss_variable_new_input composite with the supplied <inputs>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	std::list<Variable_input_handle> *inputs_address;

	result=0;
	if (inputs_address=reinterpret_cast<std::list<Variable_input_handle> *>(
    inputs))
	{
		result=reinterpret_cast<Cmiss_variable_new_input_id>(
#if defined (USE_SMART_POINTER)
			new Variable_input_composite_handle(new Variable_input_composite(
			*inputs_address))
#else /* defined (USE_SMART_POINTER) */
			new Variable_input_composite(*inputs_address)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}
