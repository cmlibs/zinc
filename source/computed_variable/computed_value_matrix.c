/*******************************************************************************
FILE : computed_value_matrix.c

LAST MODIFIED : 30 July 2003

DESCRIPTION :
Implements a matrix computed value.
==============================================================================*/
#include "matrix/matrix.h"
#include "computed_variable/computed_value_matrix.h"
#include "computed_variable/computed_value_private.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Module methods
--------------
*/
static char Cmiss_value_matrix_type_string[]="Matrix";

struct Cmiss_value_matrix_type_specific_data
/*******************************************************************************
LAST MODIFIED : 4 May 2003

DESCRIPTION :
==============================================================================*/
{
	struct Matrix *matrix;
}; /* struct Cmiss_value_matrix_type_specific_data */

static START_CMISS_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(matrix)
{
	if (data->matrix)
	{
		DESTROY(Matrix)(&(data->matrix));
	}
	return_code=1;
}
END_CMISS_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(matrix)

static START_CMISS_VALUE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(matrix)
{
	int number_of_columns,number_of_rows;
	struct Matrix *destination_matrix,*source_matrix;

	if ((source_matrix=source->matrix)&&Matrix_get_dimensions(source_matrix,
		&number_of_rows,&number_of_columns))
	{
		if (destination_matrix=CREATE(Matrix)("matrix",DENSE,number_of_rows,
			number_of_columns))
		{
			if (Matrix_copy(source_matrix,destination_matrix))
			{
				destination->matrix=destination_matrix;
			}
			else
			{
				DESTROY(Matrix)(&destination_matrix);
				DEALLOCATE(destination);
			}
		}
		else
		{
			DEALLOCATE(destination);
		}
	}
	else
	{
		DEALLOCATE(destination);
	}
}
END_CMISS_VALUE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(matrix)

static START_CMISS_VALUE_GET_REALS_TYPE_SPECIFIC_FUNCTION(matrix)
{
	FE_value *destination_real,*destination_reals;
	int column_number,number_of_columns,number_of_reals,number_of_rows,row_number;
	Matrix_value matrix_value;
	struct Matrix *matrix;

	if ((matrix=data->matrix)&&Matrix_get_dimensions(matrix,&number_of_rows,
		&number_of_columns))
	{
		number_of_reals=number_of_rows*number_of_columns;
		if (!reals_address||ALLOCATE(destination_reals,FE_value,number_of_reals))
		{
			if (reals_address)
			{
				destination_real=destination_reals;
				row_number=1;
				while (return_code&&(row_number<=number_of_rows))
				{
					column_number=1;
					while ((column_number<=number_of_columns)&&(return_code=
						Matrix_get_value(matrix,row_number,column_number,&matrix_value)))
					{
						*destination_real=(FE_value)matrix_value;
						destination_real++;
						column_number++;
					}
					row_number++;
				}
				if (return_code)
				{
					*reals_address=destination_reals;
				}
				else
				{
					DEALLOCATE(destination_real);
				}
			}
			else
			{
				return_code=1;
			}
			if (return_code)
			{
				*number_of_reals_address=number_of_reals;
			}
		}
	}
}
END_CMISS_VALUE_GET_REALS_TYPE_SPECIFIC_FUNCTION(matrix)

#define Cmiss_value_matrix_get_string_type_specific \
	Cmiss_value_default_get_string

static START_CMISS_VALUE_INCREMENT_TYPE_SPECIFIC_FUNCTION(matrix)
/*******************************************************************************
LAST MODIFIED : 30 July 2003

DESCRIPTION :
==============================================================================*/
{
	int i,j,number_of_columns,number_of_rows;
	Matrix_value value_increment,value_value;
	struct Cmiss_value_matrix_type_specific_data *data_increment;
	struct Matrix *matrix_increment,*matrix_value;

	/* check that <value> and <increment> "match" */
	if (CMISS_VALUE_IS_TYPE(matrix)(increment)&&(data_increment=
		(struct Cmiss_value_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(increment))&&
		(matrix_value=data_value->matrix)&&
		(matrix_increment=data_increment->matrix)&&
		Matrix_get_dimensions(matrix_value,&number_of_rows,&number_of_columns)&&
		Matrix_get_dimensions(matrix_increment,&i,&j)&&
		(number_of_rows==i)&&(number_of_columns==j))
	{
		return_code=1;
		i=1;
		while (return_code&&(i<=number_of_rows))
		{
			j=1;
			while (return_code&&(j<=number_of_columns))
			{
				if ((return_code=Matrix_get_value(matrix_value,i,j,&value_value))&&
					(return_code=Matrix_get_value(matrix_increment,i,j,&value_increment)))
				{
					value_value += value_increment;
					return_code=Matrix_set_value(matrix_value,i,j,value_value);
				}
				j++;
			}
			i++;
		}
	}
}
END_CMISS_VALUE_INCREMENT_TYPE_SPECIFIC_FUNCTION(matrix)

static START_CMISS_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(
	matrix)
{
	struct Cmiss_value_matrix_type_specific_data *data_total,*data_1,*data_2;
	struct Matrix *matrix_total,*matrix_1,*matrix_2;

	data_1=(struct Cmiss_value_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_1);
	data_2=(struct Cmiss_value_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_2);
	data_total=(struct Cmiss_value_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(total);
	ASSERT_IF(data_1&&data_2&&data_total,return_code,0)
	{
		matrix_1=data_1->matrix;
		matrix_2=data_2->matrix;
		matrix_total=data_total->matrix;
		ASSERT_IF(matrix_1&&matrix_2&&matrix_total,return_code,0)
		{
			return_code=Matrix_matmul(matrix_1,matrix_2,matrix_total,(Matrix_value)1,
				(Matrix_value)1);
		}
	}
}
END_CMISS_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(matrix)

static START_CMISS_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(matrix)
{
	int number_of_columns_1,number_of_columns_2,number_of_rows_1,number_of_rows_2;
	struct Cmiss_value_matrix_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_value_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_1);
	data_2=(struct Cmiss_value_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_2);
	ASSERT_IF(data_1&&data_2,return_code,0)
	{
		if (Matrix_get_dimensions(data_1->matrix,&number_of_rows_1,
			&number_of_columns_1)&&Matrix_get_dimensions(data_2->matrix,
			&number_of_rows_2,&number_of_columns_2)&&
			(number_of_rows_1==number_of_rows_2)&&
			(number_of_columns_1==number_of_columns_2))
		{
			return_code=1;
		}
	}
}
END_CMISS_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(matrix)

static START_CMISS_VALUE_SCALAR_MULTIPLY_TYPE_SPECIFIC_FUNCTION(matrix)
/*******************************************************************************
LAST MODIFIED : 30 July 2003

DESCRIPTION :
==============================================================================*/
{
	int i,j,number_of_columns,number_of_rows;
	Matrix_value value_value;
	struct Matrix *matrix_value;

	if ((matrix_value=data->matrix)&&Matrix_get_dimensions(matrix_value,
		&number_of_rows,&number_of_columns)&&(0<number_of_rows)&&
		(0<number_of_columns))
	{
		return_code=1;
		i=1;
		while (return_code&&(i<=number_of_rows))
		{
			j=1;
			while (return_code&&(j<=number_of_columns))
			{
				if (return_code=Matrix_get_value(matrix_value,i,j,&value_value))
				{
					value_value *= scalar;
					return_code=Matrix_set_value(matrix_value,i,j,value_value);
				}
				j++;
			}
			i++;
		}
	}
}
END_CMISS_VALUE_SCALAR_MULTIPLY_TYPE_SPECIFIC_FUNCTION(matrix)

/*
Global functions
----------------
*/
int Cmiss_value_matrix_set_type(Cmiss_value_id value,struct Matrix *matrix)
/*******************************************************************************
LAST MODIFIED : 4 May 2003

DESCRIPTION :
Makes <value> of type matrix and sets its <matrix>.  After success, the <value>
is responsible for DESTROYing <matrix>.
==============================================================================*/
{
	int return_code;
	struct Cmiss_value_matrix_type_specific_data *data;

	ENTER(Cmiss_value_matrix_set_type);
	return_code=0;
	/* check arguments */
	if (value)
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,struct Cmiss_value_matrix_type_specific_data,1))
		{
			/* 2.  Clear current type-specific data */
			Cmiss_value_clear_type(value);
			/* 3.  Establish the new type */
			Cmiss_value_set_type_specific_information(value,
				Cmiss_value_matrix_type_string,(void *)data);
			if (matrix)
			{
				data->matrix=matrix;
			}
			else
			{
				data->matrix=(struct Matrix *)NULL;
			}
			/* set all the methods */
			return_code=CMISS_VALUE_ESTABLISH_METHODS(value,matrix);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_value_matrix_set_type.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_matrix_set_type.  "
			"Missing value");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_matrix_set_type */

DECLARE_CMISS_VALUE_IS_TYPE_FUNCTION(matrix)

int Cmiss_value_matrix_get_type(Cmiss_value_id value,
	struct Matrix **matrix_address)
/*******************************************************************************
LAST MODIFIED : 4 May 2003

DESCRIPTION :
If <value> is of type matrix, gets its <*matrix_address>.

The calling program must not DESTROY the returned <*matrix_address>.
==============================================================================*/
{
	int return_code;
	struct Cmiss_value_matrix_type_specific_data *data;

	ENTER(Cmiss_value_matrix_get_type);
	return_code=0;
	/* check arguments */
	if (value&&CMISS_VALUE_IS_TYPE(matrix)&&matrix_address)
	{
		data=(struct Cmiss_value_matrix_type_specific_data *)
			Cmiss_value_get_type_specific_data(value);
		ASSERT_IF(data,return_code,0)
		{
			if (matrix_address)
			{
				*matrix_address=data->matrix;
			}
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_matrix_get_type.  "
			"Invalid argument(s).  %p %p",value,matrix_address);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_matrix_get_type */

int Cmiss_value_matrix_get_dimensions(Cmiss_value_id matrix,
	int *number_of_rows_address,int *number_of_columns_address)
/*******************************************************************************
LAST MODIFIED : 7 May 2003

DESCRIPTION :
Gets the <*number_of_rows_address> and <*number_of_columns_address> for the
<matrix>.
==============================================================================*/
{
	int number_of_columns,number_of_rows,return_code;
	struct Matrix *local_matrix;

	ENTER(Cmiss_value_matrix_get_dimensions);
	return_code=0;
	if (matrix&&Cmiss_value_matrix_get_type(matrix,&local_matrix)&&
		(number_of_rows_address||number_of_columns_address))
	{
		if (Matrix_get_dimensions(local_matrix,&number_of_rows,&number_of_columns))
		{
			if (number_of_rows_address)
			{
				*number_of_rows_address=number_of_rows;
			}
			if (number_of_columns_address)
			{
				*number_of_columns_address=number_of_columns;
			}
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_matrix_get_dimensions.  "
			"Invalid argument(s).  %p %p %p",matrix,number_of_rows_address,
			number_of_columns_address);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_matrix_get_dimensions */

int Cmiss_value_matrix_get_value(Cmiss_value_id matrix,int row_number,
	int column_number,Matrix_value *value_address)
/*******************************************************************************
LAST MODIFIED : 7 May 2003

DESCRIPTION :
Gets the <*value_address> at the <row_number> and <column_number> of the
<matrix>.
==============================================================================*/
{
	int return_code;
	struct Matrix *local_matrix;

	ENTER(Cmiss_value_matrix_get_value);
	return_code=0;
	if (matrix&&Cmiss_value_matrix_get_type(matrix,&local_matrix)&&value_address)
	{
		return_code=Matrix_get_value(local_matrix,row_number,column_number,
			value_address);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_matrix_get_value.  "
			"Invalid argument(s).  %p %p",matrix,value_address);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_matrix_get_value */

int Cmiss_value_matrix_set_value(Cmiss_value_id matrix,int row_number,
	int column_number,Matrix_value value)
/*******************************************************************************
LAST MODIFIED : 7 May 2003

DESCRIPTION :
Sets the <value> at the <row_number> and <column_number> of the <matrix>.
==============================================================================*/
{
	int return_code;
	struct Matrix *local_matrix;

	ENTER(Cmiss_value_matrix_set_value);
	return_code=0;
	if (matrix&&Cmiss_value_matrix_get_type(matrix,&local_matrix))
	{
		return_code=Matrix_set_value(local_matrix,row_number,column_number,value);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_matrix_set_value.  "
			"Invalid argument(s).  %p",matrix);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_matrix_set_value */

Cmiss_value_id Cmiss_value_matrix_get_submatrix(Cmiss_value_id matrix,
	int row_low,int row_high,int column_low,int column_high)
/*******************************************************************************
LAST MODIFIED : 18 July 2003

DESCRIPTION :
Returns a Cmiss value which is the specified sub-matrix of <matrix>.
==============================================================================*/
{
	Cmiss_value_id sub_matrix;
	enum Matrix_type matrix_type;
	int number_of_columns,number_of_rows;
	Matrix_value *values;
	struct Matrix *local_matrix;

	ENTER(Cmiss_value_matrix_get_submatrix);
	sub_matrix=(Cmiss_value_id)NULL;
	/* check args */
	if (matrix&&Cmiss_value_matrix_get_type(matrix,&local_matrix)&&
		Matrix_get_dimensions(local_matrix,&number_of_rows,&number_of_columns)&&
		Matrix_get_type(local_matrix,&matrix_type)&&(1<=row_low)&&
		(row_low<=row_high)&&(row_high<=number_of_rows)&&(1<=column_low)&&
		(column_low<=column_high)&&(column_high<=number_of_columns))
	{
		if (ALLOCATE(values,Matrix_value,(row_high-row_low+1)*(column_high-
			column_low+1)))
		{
			if (Matrix_get_values(local_matrix,values,row_low,row_high,column_low,
				column_high))
			{
				if (!((local_matrix=CREATE(Matrix)("matrix",matrix_type,
					row_high-row_low+1,column_high-column_low+1))&&Matrix_set_values(
					local_matrix,values,1,row_high-row_low+1,1,column_high-column_low+1)&&
					(sub_matrix=CREATE(Cmiss_value)())&&Cmiss_value_matrix_set_type(
					sub_matrix,local_matrix)))
				{
					if (local_matrix)
					{
						DESTROY(Matrix)(&local_matrix);
						if (sub_matrix)
						{
							DESTROY(Cmiss_value)(&sub_matrix);
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Cmiss_value_matrix_get_submatrix.  "
					"Could not get values");
			}
			DEALLOCATE(values);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_value_matrix_get_submatrix.  "
				"Could not allocate values");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_matrix_get_submatrix.  "
			"Invalid argument(s).  %p %d %d %d %d %d %d",matrix,row_low,row_high,
			column_low,column_high,number_of_rows,number_of_columns);
	}
	LEAVE;

	return (sub_matrix);
} /* Cmiss_value_matrix_get_submatrix */

int Cmiss_value_matrix_set_submatrix(Cmiss_value_id matrix,int row_low,
	int row_high,int column_low,int column_high,Cmiss_value_id submatrix)
/*******************************************************************************
LAST MODIFIED : 16 July 2003

DESCRIPTION :
Copies the <submatrix> into the specified sub-matrix of <matrix>.
==============================================================================*/
{
	int number_of_columns,number_of_rows,return_code;
	Matrix_value *values;
	struct Matrix *local_matrix,*local_submatrix;

	ENTER(Cmiss_value_matrix_get_submatrix);
	return_code=0;
	/* check args */
	if (matrix&&Cmiss_value_matrix_get_type(matrix,&local_matrix)&&
		Matrix_get_dimensions(local_matrix,&number_of_rows,&number_of_columns)&&
		(1<=row_low)&&(row_low<=row_high)&&(row_high<=number_of_rows)&&
		(1<=column_low)&&(column_low<=column_high)&&
		(column_high<=number_of_columns)&&submatrix&&Cmiss_value_matrix_get_type(
		submatrix,&local_submatrix)&&Matrix_get_dimensions(local_submatrix,
		&number_of_rows,&number_of_columns)&&(number_of_rows==row_high-row_low+1)&&
		(number_of_columns==column_high-column_low+1))
	{
		if (ALLOCATE(values,Matrix_value,number_of_rows*number_of_columns))
		{
			if (Matrix_get_values(local_submatrix,values,1,number_of_rows,1,
				number_of_columns))
			{
				if (Matrix_set_values(local_matrix,values,row_low,row_high,column_low,
					column_high))
				{
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,"Cmiss_value_matrix_set_submatrix.  "
						"Could not set values");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Cmiss_value_matrix_set_submatrix.  "
					"Could not get values");
			}
			DEALLOCATE(values);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_value_matrix_set_submatrix.  "
				"Could not allocate values");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_matrix_set_submatrix.  "
			"Invalid argument(s).  %p %p %d %d %d %d %p %p %d %d",matrix,local_matrix,
			row_low,row_high,column_low,column_high,submatrix,local_submatrix,
			number_of_rows,number_of_columns);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_matrix_set_submatrix */
