/*******************************************************************************
FILE : api/cmiss_function_matrix_divide_by_scalar.cpp

LAST MODIFIED : 8 March 2005

DESCRIPTION :
The public interface to the Cmiss_function matrix divide by scalar object.
==============================================================================*/

#include <new>
#include "api/cmiss_function_matrix_divide_by_scalar.h"
#include "computed_variable/function_matrix_divide_by_scalar.hpp"

/*
Global functions
----------------
*/

Cmiss_function_id Cmiss_function_matrix_divide_by_scalar_create(
	Cmiss_function_variable_id dividend,Cmiss_function_variable_id divisor)
/*******************************************************************************
LAST MODIFIED : 8 March 2005

DESCRIPTION :
Creates a Cmiss_function matrix which is the <dividend> (has matrix value)
divided by <divisor> (has a 1x1 matrix value).
==============================================================================*/
{
	Cmiss_function_id result;
	Function_variable_handle *dividend_handle_address,*divisor_handle_address;

	result=0;
	if ((dividend_handle_address=reinterpret_cast<Function_variable_handle *>(
		dividend))&&(divisor_handle_address=
		reinterpret_cast<Function_variable_handle *>(divisor)))
	{
		try
		{
			result=reinterpret_cast<Cmiss_function_id>(new Function_handle(
				new Function_matrix_divide_by_scalar<Scalar>(*dividend_handle_address,
				*divisor_handle_address)));
		}
		catch (Function_matrix_divide_by_scalar<Scalar>::Invalid_dividend_divisor)
		{
			result=0;
		}
	}

	return (result);
}
