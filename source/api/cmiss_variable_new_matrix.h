/*******************************************************************************
FILE : api/cmiss_variable_new_matrix.h

LAST MODIFIED : 6 November 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new matrix object.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_NEW_MATRIX_H__
#define __API_CMISS_VARIABLE_NEW_MATRIX_H__

#include "api/cmiss_variable_new.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_variable_new_id Cmiss_variable_new_matrix_create(
	unsigned int number_of_rows,unsigned int number_of_columns,Scalar *values);
/*******************************************************************************
LAST MODIFIED : 24 October 2003

DESCRIPTION :
Creates a Cmiss_variable_new matrix with the specified <number_of_rows>,
<number_of_columns> and <values>.  If <values> is NULL then the matrix is
initialized to zero.
==============================================================================*/

int Cmiss_variable_new_matrix_get_dimensions(
	Cmiss_variable_new_id variable_matrix,unsigned int *number_of_rows_address,
	unsigned int *number_of_columns_address);
/*******************************************************************************
LAST MODIFIED : 6 November 2003

DESCRIPTION :
Gets the <*number_of_rows_address> and <*number_of_columns_address> for the
<variable_matrix>.
==============================================================================*/

Cmiss_variable_new_id Cmiss_variable_new_matrix_get_sub_matrix(
	Cmiss_variable_new_id variable_matrix,unsigned int row_low,
	unsigned int row_high,unsigned int column_low,unsigned int column_high);
/*******************************************************************************
LAST MODIFIED : 6 November 2003

DESCRIPTION :
Returns a Cmiss_variable_new matrix which is the specified sub-matrix of
<variable_matrix>.
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_matrix_values(
	Cmiss_variable_new_id variable_matrix,unsigned int number_of_indices,
	unsigned int *row_indices,unsigned int *column_indices);
/*******************************************************************************
LAST MODIFIED : 24 October 2003

DESCRIPTION :
Returns the values input made up of the specified indices for the
<variable_matrix>.  If <number_of_indices> is zero or <row_indices> is NULL or
<column_indices> is NULL then the input refers to all values.
==============================================================================*/

Cmiss_variable_new_id Cmiss_variable_new_matrix_solve(
	Cmiss_variable_new_id variable_matrix,Cmiss_variable_new_id variable_rhs);
/*******************************************************************************
LAST MODIFIED : 11 December 2003

DESCRIPTION :
Returns the solution of the linear system <variable_matrix>*x=<variable_rhs>.
<variable_rhs> should be a matrix or a vector.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_VARIABLE_NEW_MATRIX_H__ */
