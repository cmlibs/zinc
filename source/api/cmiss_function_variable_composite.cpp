/*******************************************************************************
FILE : api/cmiss_function_variable_composite.cpp

LAST MODIFIED : 17 March 2004

DESCRIPTION :
The public interface to the Cmiss_function_variable composite object.
==============================================================================*/

#include <new>
#include "api/cmiss_function_variable_composite.h"
#include "computed_variable/function_variable_composite.hpp"

/*
Global functions
----------------
*/
Cmiss_function_variable_id Cmiss_function_variable_composite_create(
	Cmiss_function_variable_list_id variables)
/*******************************************************************************
LAST MODIFIED : 17 March 2004

DESCRIPTION :
Creates a Cmiss_function_variable composite with the supplied <variables>.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	std::list<Function_variable_handle> *variables_address;

	result=0;
	if (variables_address=reinterpret_cast<std::list<Function_variable_handle> *>(
    variables))
	{
		result=reinterpret_cast<Cmiss_function_variable_id>(
			new Function_variable_composite_handle(new Function_variable_composite(
			*variables_address)));
	}

	return (result);
}
