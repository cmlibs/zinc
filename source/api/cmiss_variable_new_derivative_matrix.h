/*******************************************************************************
FILE : api/cmiss_variable_new_derivative_matrix.h

LAST MODIFIED : 5 November 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new derivative matrix object.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_NEW_DERIVATIVE_MATRIX_H__
#define __API_CMISS_VARIABLE_NEW_DERIVATIVE_MATRIX_H__

#include "api/cmiss_variable_new.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_variable_new_id Cmiss_variable_new_derivative_matrix_get_matrix(
	Cmiss_variable_new_id variable_matrix,
	Cmiss_variable_new_input_list_id independent_variables);
/*******************************************************************************
LAST MODIFIED : 5 November 2003

DESCRIPTION :
If <variable_matrix> is of type derivative_matrix, this function returns the
specified partial derivative (<independent_variables>).

???DB.  Extend so that can have an independent variable that is a subset of
	one of the independent variables for the derivative matrix.  eg nodal values
	for a particular node as a subset of all nodal values
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_VARIABLE_NEW_DERIVATIVE_MATRIX_H__ */
