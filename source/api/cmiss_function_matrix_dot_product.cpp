/*******************************************************************************
FILE : api/cmiss_function_matrix_dot_product.cpp

LAST MODIFIED : 20 October 2004

DESCRIPTION :
The public interface to the Cmiss_function matrix dot product object.
==============================================================================*/

#include <new>
#include "api/cmiss_function_matrix_dot_product.h"
#include "computed_variable/function_matrix_dot_product.hpp"

/*
Global functions
----------------
*/

Cmiss_function_id Cmiss_function_matrix_dot_product_create(
	Cmiss_function_variable_id variable_1,Cmiss_function_variable_id variable_2)
/*******************************************************************************
LAST MODIFIED : 20 October 2004

DESCRIPTION :
Creates a Cmiss_function matrix which is the dot product of <variable_1> and
<variable_2>.
==============================================================================*/
{
	Cmiss_function_id result;
	Function_variable_handle *variable_1_handle_address,
		*variable_2_handle_address;

	result=0;
	if ((variable_1_handle_address=reinterpret_cast<Function_variable_handle *>(
		variable_1))&&(variable_2_handle_address=
		reinterpret_cast<Function_variable_handle *>(variable_2)))
	{
		try
		{
			result=reinterpret_cast<Cmiss_function_id>(new Function_handle(
				new Function_matrix_dot_product<Scalar>(*variable_1_handle_address,
				*variable_2_handle_address)));
		}
		catch (Function_matrix_dot_product<Scalar>::Invalid_argument)
		{
			result=0;
		}
	}

	return (result);
}
