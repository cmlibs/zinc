/*******************************************************************************
FILE : computed_value_fe_value.h

LAST MODIFIED : 25 July 2003

DESCRIPTION :
computed_value types for FE_value, FE_value vector and FE_value_matrix
==============================================================================*/
#if !defined (__CMISS_VALUE_FE_VALUE_H__)
#define __CMISS_VALUE_FE_VALUE_H__

/*
Global functions
----------------
*/
int Cmiss_value_FE_value_set_type(Cmiss_value_id value,
	FE_value fe_value);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Makes <value> of type FE_value and sets its <fe_value>.
==============================================================================*/

PROTOTYPE_CMISS_VALUE_IS_TYPE_FUNCTION(FE_value);

int Cmiss_value_FE_value_get_type(Cmiss_value_id value,
	FE_value *fe_value_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <value> is of type FE_value, gets its <*fe_value_address>.
==============================================================================*/

int Cmiss_value_FE_value_vector_set_type(Cmiss_value_id value,
	int number_of_fe_values,FE_value *fe_value_vector);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Makes <value> of type FE_value_vector and sets its <number_of_fe_values> and
<fe_value_vector>.  After success, the <value> is responsible for DEALLOCATEing
<fe_value_vector>.
==============================================================================*/

PROTOTYPE_CMISS_VALUE_IS_TYPE_FUNCTION(FE_value_vector);

int Cmiss_value_FE_value_vector_get_type(Cmiss_value_id value,
	int *number_of_fe_values_address,FE_value **fe_value_vector_address);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
If <value> is of type FE_value_vector, gets its <*number_of_fe_values_address>
and <*fe_value_vector_address>.

The calling program must not DEALLOCATE the returned <*fe_value_vector_address>.
==============================================================================*/

int Cmiss_value_FE_value_matrix_set_type(Cmiss_value_id value,
	int number_of_rows,int number_of_columns,FE_value *fe_value_matrix);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Makes <value> of type FE_value_matrix and sets its <number_of_rows>,
<number_of_columns> and <fe_value_matrix> (column number varying fastest).
After success, the <value> is responsible for DEALLOCATEing <fe_value_matrix>.
==============================================================================*/

PROTOTYPE_CMISS_VALUE_IS_TYPE_FUNCTION(FE_value_matrix);

int Cmiss_value_FE_value_matrix_get_type(Cmiss_value_id value,
	int *number_of_rows_address,int *number_of_columns_address,
	FE_value **fe_value_matrix_address);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
If <value> is of type FE_value_matrix, gets its <*number_of_rows_address>,
<*number_of_columns_address> and <*fe_value_matrix_address>.

The calling program must not DEALLOCATE the returned <*fe_value_matrix_address>.
==============================================================================*/
#endif /* !defined (__CMISS_VALUE_FE_VALUE_H__) */
