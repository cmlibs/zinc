/*******************************************************************************
FILE : api/cmiss_function_matrix_product.cpp

LAST MODIFIED : 8 September 2004

DESCRIPTION :
The public interface to the Cmiss_function matrix object.
==============================================================================*/

#include <new>
#include "api/cmiss_function_matrix_product.h"
#include "computed_variable/function_matrix_product.hpp"

/*
Global functions
----------------
*/

Cmiss_function_id Cmiss_function_matrix_product_create(
	Cmiss_function_variable_id multiplier,
	Cmiss_function_variable_id multiplicand)
/*******************************************************************************
LAST MODIFIED : 8 September 2004

DESCRIPTION :
Creates a Cmiss_function matrix which is the product of <multiplier> and
<multiplicand>.
==============================================================================*/
{
	Cmiss_function_id result;
	Function_variable_handle *multiplier_handle_address,
		*multiplicand_handle_address;

	result=0;
	if ((multiplier_handle_address=reinterpret_cast<Function_variable_handle *>(
		multiplier))&&(multiplicand_handle_address=
		reinterpret_cast<Function_variable_handle *>(multiplicand)))
	{
		try
		{
			result=reinterpret_cast<Cmiss_function_id>(new Function_handle(
				new Function_matrix_product<Scalar>(*multiplier_handle_address,
				*multiplicand_handle_address)));
		}
		catch (Function_matrix_product<Scalar>::Invalid_multiplier_multiplicand)
		{
			result=0;
		}
	}

	return (result);
}
