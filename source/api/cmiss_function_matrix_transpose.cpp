/*******************************************************************************
FILE : api/cmiss_function_matrix_transpose.cpp

LAST MODIFIED : 7 October 2004

DESCRIPTION :
The public interface to the Cmiss_function matrix transpose object.
==============================================================================*/

#include <new>
#include "api/cmiss_function_matrix_transpose.h"
#include "computed_variable/function_matrix_transpose.hpp"

/*
Global functions
----------------
*/

Cmiss_function_id Cmiss_function_matrix_transpose_create(
	Cmiss_function_variable_id matrix)
/*******************************************************************************
LAST MODIFIED : 7 October 2004

DESCRIPTION :
Creates a Cmiss_function matrix which is the transpose of <matrix>.
==============================================================================*/
{
	Cmiss_function_id result;
	Function_variable_handle *matrix_handle_address;

	result=0;
	if (matrix_handle_address=reinterpret_cast<Function_variable_handle *>(
		matrix))
	{
		try
		{
			result=reinterpret_cast<Cmiss_function_id>(new Function_handle(
				new Function_matrix_transpose<Scalar>(*matrix_handle_address)));
		}
		catch (Function_matrix_transpose<Scalar>::Invalid_matrix)
		{
			result=0;
		}
	}

	return (result);
}
