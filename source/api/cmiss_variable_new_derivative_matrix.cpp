/*******************************************************************************
FILE : api/cmiss_variable_new_derivative_matrix.cpp

LAST MODIFIED : 2 February 2004

DESCRIPTION :
The public interface to the Cmiss_variable_new derivative matrix object.
==============================================================================*/

#include <new>
#include "api/cmiss_variable_new_derivative_matrix.h"
#include "computed_variable/variable_derivative_matrix.hpp"

/*
Global functions
----------------
*/

Cmiss_variable_new_id Cmiss_variable_new_derivative_matrix_get_matrix(
	Cmiss_variable_new_id variable_derivative_matrix,
	Cmiss_variable_new_input_list_id independent_variables)
/*******************************************************************************
LAST MODIFIED : 2 February 2004

DESCRIPTION :
If <variable_derivative_matrix> is of type derivative_matrix, this function
returns the specified partial derivative (<independent_variables>).

???DB.  Extend so that can have an independent variable that is a subset of
	one of the independent variables for the derivative matrix.  eg nodal values
	for a particular node as a subset of all nodal values
==============================================================================*/
{
	Cmiss_variable_new_id result;
	std::list<
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		> *independent_variables_address;
#if defined (USE_SMART_POINTER)
	Variable_derivative_matrix_handle *variable_derivative_matrix_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable_derivative_matrix *variable_derivative_matrix_address;
#endif /* defined (USE_SMART_POINTER) */

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(variable_derivative_matrix_handle_address=
		reinterpret_cast<Variable_derivative_matrix_handle *>(
		variable_derivative_matrix))&&
#else /* defined (USE_SMART_POINTER) */
		(variable_derivative_matrix_address=
		reinterpret_cast<Variable_derivative_matrix *>(
		variable_derivative_matrix))&&
#endif /* defined (USE_SMART_POINTER) */
		(independent_variables_address=reinterpret_cast<std::list<
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		> *>(independent_variables)))
	{
		result=reinterpret_cast<Cmiss_variable_new_id>(
#if defined (USE_SMART_POINTER)
			new Variable_handle(((*variable_derivative_matrix_handle_address)->
			matrix)(*independent_variables_address))
#else /* defined (USE_SMART_POINTER) */
			(variable_derivative_matrix_address->matrix)(
			*independent_variables_address)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}
