/*******************************************************************************
FILE : api/cmiss_function_variable_exclusion.cpp

LAST MODIFIED : 19 March 2004

DESCRIPTION :
The public interface to the Cmiss_function_variable exclusion object.
==============================================================================*/

#include <new>
#include "api/cmiss_function_variable_exclusion.h"
#include "computed_variable/function_variable_exclusion.hpp"

/*
Global functions
----------------
*/
Cmiss_function_variable_id Cmiss_function_variable_exclusion_create(
	Cmiss_function_variable_id universe,Cmiss_function_variable_id exclusion)
/*******************************************************************************
LAST MODIFIED : 19 March 2004

DESCRIPTION :
Creates a Cmiss_function_variable exclusion which is <universe>-<exclusion>.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	Function_variable_handle *exclusion_address,*universe_address;

	result=0;
	if (
		(universe_address=reinterpret_cast<Function_variable_handle *>(universe))&&
		(exclusion_address=reinterpret_cast<Function_variable_handle *>(exclusion)))
	{
		result=reinterpret_cast<Cmiss_function_variable_id>(
			new Function_variable_exclusion_handle(new Function_variable_exclusion(
			*universe_address,*exclusion_address)));
	}

	return (result);
}
