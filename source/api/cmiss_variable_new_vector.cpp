/*******************************************************************************
FILE : api/cmiss_variable_new_vector.cpp

LAST MODIFIED : 24 October 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new vector object.
==============================================================================*/

#include <new>
#include "api/cmiss_variable_new_vector.h"
#include "computed_variable/variable_vector.hpp"

/*
Global functions
----------------
*/

Cmiss_variable_new_id Cmiss_variable_new_vector_create(
	unsigned int number_of_values,Scalar *values)
/*******************************************************************************
LAST MODIFIED : 24 October 2003

DESCRIPTION :
Creates a Cmiss_variable_new vector with the specified <number_of_values> and
<values>.  If <values> is NULL then the vector is initialized to zero.
==============================================================================*/
{
	Cmiss_variable_new_id result;

	result=0;
	if (0<number_of_values)
	{
		Vector values_vector(number_of_values);
		unsigned int i;

		if (values)
		{
			for (i=0;i<number_of_values;i++)
			{
				values_vector[i]=values[i];
			}
		}
		else
		{
			for (i=0;i<number_of_values;i++)
			{
				values_vector[i]=(Scalar)0;
			}
		}
#if defined (USE_SMART_POINTER)
		result=reinterpret_cast<Cmiss_variable_new_id>(new Variable_handle(
			new Variable_vector(values_vector)));
#else /* defined (USE_SMART_POINTER) */
		result=reinterpret_cast<Cmiss_variable_new_id>(Variable_handle(
			new Variable_vector(values_vector)));
#endif /* defined (USE_SMART_POINTER) */
	}

	return (result);
}

Cmiss_variable_new_input_id Cmiss_variable_new_input_vector_values(
	Cmiss_variable_new_id variable_vector,unsigned int number_of_indices,
	unsigned int *indices)
/*******************************************************************************
LAST MODIFIED : 24 October 2003

DESCRIPTION :
Returns the values input made up of the specified <indices> for the
<variable_vector>.  If <number_of_indices> is zero or <indices> is NULL then
the input refers to all values.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	Variable_input_handle input_values;
	Variable_vector_handle variable_vector_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_vector))&&
		(variable_vector_handle=boost::dynamic_pointer_cast<Variable_vector,
		Variable>(*variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(variable_vector))&&
		(variable_vector_handle=dynamic_cast<Variable_vector *>(variable_address))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		if ((0<number_of_indices)&&indices)
		{
			boost::numeric::ublas::vector<Variable_size_type> indices_vector(
				number_of_indices);
			unsigned int i;

			for (i=0;i<number_of_indices;i++)
			{
				indices_vector[i]=indices[i];
			}
			input_values=variable_vector_handle->input_values(indices_vector);
		}
		else
		{
			input_values=variable_vector_handle->input_values();
		}
		result=reinterpret_cast<Cmiss_variable_new_input_id>(
#if defined (USE_SMART_POINTER)
			new Variable_input_handle(input_values)
#else /* defined (USE_SMART_POINTER) */
			input_values
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}
