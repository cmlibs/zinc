/*******************************************************************************
FILE : api/cmiss_value_matrix.c

LAST MODIFIED : 12 August 2003

DESCRIPTION :
The public interface to the Cmiss_value_matrix object.
==============================================================================*/
#include "api/cmiss_value_matrix.h"
#include "computed_variable/computed_value_matrix.h"
#include "general/debug.h"

/*
Global functions
----------------
*/

Cmiss_value_id CREATE(Cmiss_value_matrix)(int number_of_rows,
	int number_of_columns, double *values)
/*******************************************************************************
LAST MODIFIED : 12 August 2003

DESCRIPTION :
Creates a Cmiss_value which contains a matrix of values.
==============================================================================*/
{
	Cmiss_value_id return_value;
	int i, j, k, number_of_values;
	Matrix_value *value, *matrix_values;
	struct Matrix *matrix;

	ENTER(CREATE(Cmiss_value_matrix));
	if ((number_of_rows > 0) && (number_of_columns > 0) && values)
	{
		if (return_value=CREATE(Cmiss_value)())
		{
			ACCESS(Cmiss_value)(return_value);
			number_of_values = number_of_rows * number_of_columns;
			if (ALLOCATE(matrix_values,Matrix_value,number_of_values))
			{
				/* swap column fastest to row fastest */
				value=matrix_values;
				for (j=0;j<number_of_columns;j++)
				{
					k=j;
					for (i=number_of_rows;i>0;i--)
					{
						*value = values[k];
						value++;
						k += number_of_columns;
					}
				}
				matrix=CREATE(Matrix)("matrix",DENSE,number_of_rows,number_of_columns);
				if (!(matrix && Matrix_set_values(matrix,matrix_values,1,number_of_rows,1,
					number_of_columns) && Cmiss_value_matrix_set_type(return_value,matrix)))
				{
					display_message(ERROR_MESSAGE,"CREATE(Cmiss_value_matrix).  "
						"Unable to set matrix values and matrix type in Cmiss_value.");
					if (matrix)
					{
						DESTROY(Matrix)(&matrix);
					}
					DEACCESS(Cmiss_value)(&return_value);
				}
				DEALLOCATE(matrix_values);
			}
			else
			{
				display_message(ERROR_MESSAGE,"CREATE(Cmiss_value_matrix).  "
					"Unable to allocate Matrix values.");
				DEACCESS(Cmiss_value)(&return_value);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(Cmiss_value_matrix).  "
				"Unable to Create(Cmiss_value).");
			return_value = (Cmiss_value_id)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Cmiss_value_matrix).  "
			"Invalid arguments.");
		return_value = (Cmiss_value_id)NULL;
	}
	LEAVE;

	return (return_value);
} /* CREATE(Cmiss_value_matrix) */
