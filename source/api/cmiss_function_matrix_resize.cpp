/*******************************************************************************
FILE : api/cmiss_function_matrix_resize.cpp

LAST MODIFIED : 7 October 2004

DESCRIPTION :
The public interface to the Cmiss_function matrix resize object.
==============================================================================*/

#include <new>
#include "api/cmiss_function_matrix_resize.h"
#include "computed_variable/function_matrix_resize.hpp"

/*
Global functions
----------------
*/

Cmiss_function_id Cmiss_function_matrix_resize_create(
	Cmiss_function_variable_id matrix,unsigned int number_of_columns)
/*******************************************************************************
LAST MODIFIED : 7 October 2004

DESCRIPTION :
Creates a Cmiss_function matrix which is the resize of <matrix>.
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
				new Function_matrix_resize<Scalar>(*matrix_handle_address,
				(Function_size_type)number_of_columns)));
		}
		catch (Function_matrix_resize<Scalar>::Invalid_argument)
		{
			result=0;
		}
	}

	return (result);
}
