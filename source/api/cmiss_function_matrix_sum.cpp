/*******************************************************************************
FILE : api/cmiss_function_matrix_sum.cpp

LAST MODIFIED : 8 September 2004

DESCRIPTION :
The public interface to the Cmiss_function matrix object.
==============================================================================*/

#include <new>
#include "api/cmiss_function_matrix_sum.h"
#include "computed_variable/function_matrix_sum.hpp"

/*
Global functions
----------------
*/

Cmiss_function_id Cmiss_function_matrix_sum_create(
	Cmiss_function_variable_id summand_1,Cmiss_function_variable_id summand_2)
/*******************************************************************************
LAST MODIFIED : 8 September 2004

DESCRIPTION :
Creates a Cmiss_function matrix which is the sum of <summand_1> and <summand_2>.
==============================================================================*/
{
	Cmiss_function_id result;
	Function_variable_handle *summand_1_handle_address,*summand_2_handle_address;

	result=0;
	if ((summand_1_handle_address=reinterpret_cast<Function_variable_handle *>(
		summand_1))&&(summand_2_handle_address=
		reinterpret_cast<Function_variable_handle *>(summand_2)))
	{
		try
		{
			result=reinterpret_cast<Cmiss_function_id>(new Function_handle(
				new Function_matrix_sum<Scalar>(*summand_1_handle_address,
				*summand_2_handle_address)));
		}
		catch (Function_matrix_sum<Scalar>::Invalid_summand)
		{
			result=0;
		}
	}

	return (result);
}
