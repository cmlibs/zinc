/*******************************************************************************
FILE : computed_value_matrix.h

LAST MODIFIED : 11 May 2003

DESCRIPTION :
Implements a matrix computed value.
==============================================================================*/
#if !defined (__CMISS_VALUE_MATRIX_H__)
#define __CMISS_VALUE_MATRIX_H__

#include "computed_variable/computed_value.h"
#include "matrix/matrix.h"

/*
Global functions
----------------
*/
int Cmiss_value_matrix_set_type(Cmiss_value_id value,struct Matrix *matrix);
/*******************************************************************************
LAST MODIFIED : 4 May 2003

DESCRIPTION :
Makes <value> of type matrix and sets its <matrix>.  After success, the <value>
is responsible for DESTROYing <matrix>.
==============================================================================*/

PROTOTYPE_CMISS_VALUE_IS_TYPE_FUNCTION(matrix);

int Cmiss_value_matrix_get_type(Cmiss_value_id value,
	struct Matrix **matrix_address);
/*******************************************************************************
LAST MODIFIED : 4 May 2003

DESCRIPTION :
If <value> is of type matrix, gets its <*matrix_address>.

The calling program must not DESTROY the returned <*matrix_address>.
==============================================================================*/

int Cmiss_value_matrix_get_dimensions(Cmiss_value_id matrix,
	int *number_of_rows_address,int *number_of_columns_address);
/*******************************************************************************
LAST MODIFIED : 7 May 2003

DESCRIPTION :
Gets the <*number_of_rows_address> and <*number_of_columns_address> for the
<matrix>.
==============================================================================*/

int Cmiss_value_matrix_get_value(Cmiss_value_id matrix,int row_number,
	int column_number,Matrix_value *value_address);
/*******************************************************************************
LAST MODIFIED : 7 May 2003

DESCRIPTION :
Gets the <*value_address> at the <row_number> and <column_number> of the
<matrix>.
==============================================================================*/

int Cmiss_value_matrix_set_value(Cmiss_value_id matrix,int row_number,
	int column_number,Matrix_value value);
/*******************************************************************************
LAST MODIFIED : 7 May 2003

DESCRIPTION :
Sets the <value> at the <row_number> and <column_number> of the <matrix>.
==============================================================================*/

Cmiss_value_id Cmiss_value_matrix_get_submatrix(Cmiss_value_id matrix,
	int row_low,int row_high,int column_low,int column_high);
/*******************************************************************************
LAST MODIFIED : 11 May 2003

DESCRIPTION :
Returns a Cmiss value which is the specified sub-matrix of <matrix>.
==============================================================================*/
#endif /* !defined (__CMISS_VALUE_MATRIX_H__) */
