/*******************************************************************************
FILE : api/cmiss_value_matrix.h

LAST MODIFIED : 30 July 2003

DESCRIPTION :
The public interface to the Cmiss_value_matrix object.
==============================================================================*/
#ifndef __API_CMISS_VALUE_MATRIX_H__
#define __API_CMISS_VALUE_MATRIX_H__

#include "api/cmiss_value.h"
#include "general/object.h"

Cmiss_value_id CREATE(Cmiss_value_matrix)(int number_of_rows,
	int number_of_columns, double *values);
/*******************************************************************************
LAST MODIFIED : 30 July 2003

DESCRIPTION :
Creates a Cmiss_value which contains a matrix of values.
==============================================================================*/

int Cmiss_value_matrix_get_dimensions(Cmiss_value_id matrix,
	int *number_of_rows_address,int *number_of_columns_address);
/*******************************************************************************
LAST MODIFIED : 7 May 2003

DESCRIPTION :
Gets the <*number_of_rows_address> and <*number_of_columns_address> for the
<matrix>.
==============================================================================*/

Cmiss_value_id Cmiss_value_matrix_get_submatrix(Cmiss_value_id matrix,
	int row_low,int row_high,int column_low,int column_high);
/*******************************************************************************
LAST MODIFIED : 11 May 2003

DESCRIPTION :
Returns a Cmiss value which is the specified sub-matrix of <matrix>.
==============================================================================*/


#endif /* __API_CMISS_VALUE_MATRIX_H__ */
