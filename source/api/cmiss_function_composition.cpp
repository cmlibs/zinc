/*******************************************************************************
FILE : api/cmiss_function_composition.cpp

LAST MODIFIED : 11 June 2004

DESCRIPTION :
The public interface to the Cmiss_function composition object.
==============================================================================*/

#include <new>
#include "api/cmiss_function_composition.h"
#include "computed_variable/function_composition.hpp"

/*
Global functions
----------------
*/
Cmiss_function_id Cmiss_function_composition_create(
	Cmiss_function_variable_id output_variable,
	Cmiss_function_variable_id input_variable,
	Cmiss_function_variable_id value_variable)
/*******************************************************************************
LAST MODIFIED : 11 June 2004

DESCRIPTION :
Creates a Cmiss_function composition
output_variable(input_variable=value_variable).
==============================================================================*/
{
	Cmiss_function_id result;
	Function_variable_handle *input_variable_handle_address,
		*output_variable_handle_address,*value_variable_handle_address;

	result=0;
	if ((output_variable_handle_address=
		reinterpret_cast<Function_variable_handle *>(output_variable))&&
		(input_variable_handle_address=
		reinterpret_cast<Function_variable_handle *>(input_variable))&&
		(value_variable_handle_address=
		reinterpret_cast<Function_variable_handle *>(value_variable)))
	{
		result=reinterpret_cast<Cmiss_function_id>(
			new Function_composition_handle(new Function_composition(
			*output_variable_handle_address,*input_variable_handle_address,
			*value_variable_handle_address)));
	}

	return (result);
}
