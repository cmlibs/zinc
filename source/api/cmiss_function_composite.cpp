/*******************************************************************************
FILE : api/cmiss_function_composite.cpp

LAST MODIFIED : 29 November 2004

DESCRIPTION :
The public interface to the Cmiss_function composite object.
==============================================================================*/

#include <new>
#include "api/cmiss_function_composite.h"
#include "computed_variable/function_composite.hpp"

/*
Global functions
----------------
*/
Cmiss_function_id Cmiss_function_composite_create(
	Cmiss_function_list_id functions)
/*******************************************************************************
LAST MODIFIED : 29 November 2004

DESCRIPTION :
Creates a Cmiss_function composite with the supplied <functions>.
==============================================================================*/
{
	Cmiss_function_id result;
	std::list<Function_handle> *functions_address;

	result=0;
	if (functions_address=reinterpret_cast<std::list<Function_handle> *>(
    functions))
	{
		try
		{
			result=reinterpret_cast<Cmiss_function_id>(
				new Function_composite_handle(new Function_composite(
				*functions_address)));
		}
		catch (Function_composite::Construction_exception)
		{
			result=0;
		}
	}

	return (result);
}
