/*******************************************************************************
FILE : api/cmiss_variable_new_vector.h

LAST MODIFIED : 24 October 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new vector object.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_NEW_VECTOR_H__
#define __API_CMISS_VARIABLE_NEW_VECTOR_H__

#include "api/cmiss_variable_new.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_variable_new_id Cmiss_variable_new_vector_create(
	unsigned int number_of_values,Scalar *values);
/*******************************************************************************
LAST MODIFIED : 23 October 2003

DESCRIPTION :
Creates a Cmiss_variable_new vector with the specified <number_of_values> and
<values>.  If <values> is NULL then the vector is initialized to zero.
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_vector_values(
	Cmiss_variable_new_id variable_vector,unsigned int number_of_indices,
	unsigned int *indices);
/*******************************************************************************
LAST MODIFIED : 24 October 2003

DESCRIPTION :
Returns the values input made up of the specified <indices> for the
<variable_vector>.  If <number_of_indices> is zero or <indices> is NULL then
the input refers to all values.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_VARIABLE_NEW_VECTOR_H__ */
