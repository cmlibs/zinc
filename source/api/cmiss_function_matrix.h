/*******************************************************************************
FILE : api/cmiss_function_matrix.h

LAST MODIFIED : 26 April 2004

DESCRIPTION :
The public interface to the Cmiss_function matrix object.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_MATRIX_H__
#define __API_CMISS_FUNCTION_MATRIX_H__

#include "api/cmiss_function.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_id Cmiss_function_matrix_create(unsigned int number_of_rows,
	unsigned int number_of_columns,Scalar *values);
/*******************************************************************************
LAST MODIFIED : 24 February 2004

DESCRIPTION :
Creates a Cmiss_function matrix with the specified <number_of_rows>,
<number_of_columns> and <values>.  If <values> is NULL then the matrix is
initialized to zero.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_matrix_entry(
	Cmiss_function_id function_matrix,unsigned int row,unsigned int column);
/*******************************************************************************
LAST MODIFIED : 17 March 2004

DESCRIPTION :
Returns a variable that refers to the entry at the specified <row> and <column>.
If <row> is zero, then the variable refers to the <column>.  If <column> is
zero, then the variable refers to the <row>.  If <row> and <column> are both
zero, the variable refers to the whole matrix.
==============================================================================*/

int Cmiss_function_matrix_get_dimensions(Cmiss_function_id function_matrix,
	unsigned int *number_of_rows_address,unsigned int *number_of_columns_address);
/*******************************************************************************
LAST MODIFIED : 24 February 2004

DESCRIPTION :
Gets the <*number_of_rows_address> and <*number_of_columns_address> for the
<function_matrix>.  Returns a non-zero for success.
==============================================================================*/

Cmiss_function_id Cmiss_function_matrix_get_sub_matrix(
	Cmiss_function_id function_matrix,unsigned int row_low,unsigned int row_high,
	unsigned int column_low,unsigned int column_high);
/*******************************************************************************
LAST MODIFIED : 24 February 2004

DESCRIPTION :
Returns a Cmiss_function matrix which is the specified sub-matrix of
<function_matrix>.
==============================================================================*/

Cmiss_function_id Cmiss_function_matrix_solve(Cmiss_function_id function_matrix,
	Cmiss_function_id function_rhs);
/*******************************************************************************
LAST MODIFIED : 24 February 2004

DESCRIPTION :
Returns the solution of the linear system <function_matrix>*x=<function_rhs>.
<function_rhs> should be a matrix or a vector.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_MATRIX_H__ */
