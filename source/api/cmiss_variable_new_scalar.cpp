/*******************************************************************************
FILE : api/cmiss_variable_new_scalar.cpp

LAST MODIFIED : 24 October 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new scalar object.
==============================================================================*/

#include <new>
#include "api/cmiss_variable_new_scalar.h"
#include "computed_variable/variable_scalar.hpp"

/*
Global functions
----------------
*/
Cmiss_variable_new_id Cmiss_variable_new_scalar_create(Scalar value)
/*******************************************************************************
LAST MODIFIED : 11 October 2003

DESCRIPTION :
Creates a Cmiss_variable_new scalar with the supplied <value>.
==============================================================================*/
{
#if defined (USE_SMART_POINTER)
	Variable_scalar *variable_scalar;

	variable_scalar=new Variable_scalar(value);

	return (reinterpret_cast<Cmiss_variable_new_id>(new Variable_handle(
		variable_scalar)));
#else /* defined (USE_SMART_POINTER) */
	return (reinterpret_cast<Cmiss_variable_new_id>(new Variable_scalar(value)));
#endif /* defined (USE_SMART_POINTER) */
}

Cmiss_variable_new_input_id Cmiss_variable_new_input_scalar_value(
	Cmiss_variable_new_id variable_scalar)
/*******************************************************************************
LAST MODIFIED : 24 October 2003

DESCRIPTION :
Returns the value input for the <variable_scalar>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	Variable_scalar_handle variable_scalar_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_scalar))&&
		(variable_scalar_handle=boost::dynamic_pointer_cast<Variable_scalar,
		Variable>(*variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(variable_scalar))&&
		(variable_scalar_handle=dynamic_cast<Variable_scalar *>(variable_address))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		result=reinterpret_cast<Cmiss_variable_new_input_id>(
#if defined (USE_SMART_POINTER)
			new Variable_input_handle(variable_scalar_handle->input_value())
#else /* defined (USE_SMART_POINTER) */
			variable_scalar_handle->input_value()
#endif /* defined (USE_SMART_POINTER) */
			);
	}
	else
	{
		result=0;
	}

	return (result);
}
