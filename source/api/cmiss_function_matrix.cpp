/*******************************************************************************
FILE : api/cmiss_function_matrix.cpp

LAST MODIFIED : 2 September 2004

DESCRIPTION :
The public interface to the Cmiss_function matrix object.
==============================================================================*/

#include <new>
#include "api/cmiss_function_matrix.h"
#include "computed_variable/function_matrix.hpp"

/*
Module typedefs
---------------
*/

typedef boost::intrusive_ptr< Function_matrix<Scalar> > Function_matrix_handle;


/*
Global functions
----------------
*/

Cmiss_function_id Cmiss_function_matrix_create(unsigned int number_of_rows,
	unsigned int number_of_columns,Scalar *values)
/*******************************************************************************
LAST MODIFIED : 6 August 2004

DESCRIPTION :
Creates a Cmiss_function matrix with the specified <number_of_rows>,
<number_of_columns> and <values>.  If <values> is NULL then the matrix is
initialized to zero.
==============================================================================*/
{
	Cmiss_function_id result;

	result=0;
	if (0<number_of_rows*number_of_columns)
	{
		Matrix values_matrix(number_of_rows,number_of_columns);
		Function_size_type i,j,k;

		if (values)
		{
			k=0;
			for (i=0;i<number_of_rows;i++)
			{
				for (j=0;j<number_of_columns;j++)
				{
					values_matrix(i,j)=values[k];
					k++;
				}
			}
		}
		else
		{
			for (i=0;i<number_of_rows;i++)
			{
				for (j=0;j<number_of_columns;j++)
				{
					values_matrix(i,j)=(Scalar)0;
				}
			}
		}
		result=reinterpret_cast<Cmiss_function_id>(new Function_handle(
			new Function_matrix<Scalar>(values_matrix)));
	}

	return (result);
}

Scalar Cmiss_function_matrix_value(Cmiss_function_id function_matrix,
	unsigned int row,unsigned int column)
/*******************************************************************************
LAST MODIFIED : 3 February 2005

DESCRIPTION :
Returns the scalar value at the specified <row> and <column>.
==============================================================================*/
{
	Scalar result;
	Function_matrix_handle *function_matrix_handle_address;

	if ((function_matrix_handle_address=
		reinterpret_cast<Function_matrix_handle *>(function_matrix))&&
		(*function_matrix_handle_address))
	{
		result=(**function_matrix_handle_address)(
			row,column);
	}
	else
	{
		result=0;
	}

	return (result);
}

Cmiss_function_variable_id Cmiss_function_matrix_entry(
	Cmiss_function_id function_matrix,unsigned int row,unsigned int column)
/*******************************************************************************
LAST MODIFIED : 6 August 2004

DESCRIPTION :
Returns a variable that refers to the entry at the specified <row> and <column>.
If <row> is zero, then the variable refers to the <column>.  If <column> is
zero, then the variable refers to the <row>.  If <row> and <column> are both
zero, the variable refers to the whole matrix.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	Function_matrix_handle *function_matrix_handle_address;

	if ((function_matrix_handle_address=
		reinterpret_cast<Function_matrix_handle *>(function_matrix))&&
		(*function_matrix_handle_address))
	{
		result=reinterpret_cast<Cmiss_function_variable_id>(
			new Function_variable_handle(((*function_matrix_handle_address)->entry)(
			row,column)));
	}
	else
	{
		result=0;
	}

	return (result);
}

int Cmiss_function_matrix_get_dimensions(Cmiss_function_id function_matrix,
	unsigned int *number_of_rows_address,unsigned int *number_of_columns_address)
/*******************************************************************************
LAST MODIFIED : 24 February 2004

DESCRIPTION :
Gets the <*number_of_rows_address> and <*number_of_columns_address> for the
<function_matrix>.
==============================================================================*/
{
	int return_code;
	Function_matrix_handle *function_matrix_handle_address;

	return_code=0;
	if (function_matrix_handle_address=
		reinterpret_cast<Function_matrix_handle *>(function_matrix))
	{
		return_code=1;
		if (number_of_rows_address)
		{
			*number_of_rows_address=
				((*function_matrix_handle_address)->number_of_rows)();
		}
		if (number_of_columns_address)
		{
			*number_of_columns_address=
				((*function_matrix_handle_address)->number_of_columns)();
		}
	}

	return (return_code);
}

Cmiss_function_id Cmiss_function_matrix_get_sub_matrix(
	Cmiss_function_id function_matrix,unsigned int row_low,unsigned int row_high,
	unsigned int column_low,unsigned int column_high)
/*******************************************************************************
LAST MODIFIED : 24 February 2004

DESCRIPTION :
Returns a Cmiss_function matrix which is the specified sub-matrix of
<function_matrix>.
==============================================================================*/
{
	Cmiss_function_id result;
	Function_matrix_handle *function_matrix_handle_address;

	result=0;
	if (function_matrix_handle_address=
		reinterpret_cast<Function_matrix_handle *>(function_matrix))
	{
		result=reinterpret_cast<Cmiss_function_id>(new Function_handle(
			((*function_matrix_handle_address)->sub_matrix)(row_low,row_high,
			column_low,column_high)));
	}

	return (result);
}

Cmiss_function_id Cmiss_function_matrix_solve(Cmiss_function_id function_matrix,
	Cmiss_function_id function_rhs)
/*******************************************************************************
LAST MODIFIED : 6 August 2004

DESCRIPTION :
Returns the solution of the linear system <function_matrix>*x=<function_rhs>.
<function_rhs> should be a matrix or a vector.
==============================================================================*/
{
	Cmiss_function_id result;
	Function_handle *function_matrix_handle_address;
	Function_handle *function_rhs_handle_address;
	Function_matrix_handle function_matrix_handle;
	Function_handle function_rhs_handle;

	result=0;
	if ((function_matrix_handle_address=
		reinterpret_cast<Function_handle *>(function_matrix))&&
		(*function_matrix_handle_address)&&
		(function_matrix_handle=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
		Function>(*function_matrix_handle_address))&&
		(function_rhs_handle_address=
		reinterpret_cast<Function_handle *>(function_rhs))&&
		(function_rhs_handle= *function_rhs_handle_address))
	{
		Function_matrix_handle function_matrix_rhs_handle;

		if (function_matrix_rhs_handle=
			boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
			function_rhs_handle))
		{
			result=reinterpret_cast<Cmiss_function_id>(new Function_handle(
				function_matrix_handle->solve(function_matrix_rhs_handle)));
		}
	}

	return (result);
}
