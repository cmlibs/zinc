/*******************************************************************************
FILE : computed_value_fe_value.c

LAST MODIFIED : 25 July 2003

DESCRIPTION :
computed_value types for FE_value, FE_value vector and FE_value_matrix
==============================================================================*/
#include "computed_variable/computed_value.h"
#include "computed_variable/computed_value_private.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Module methods
--------------
*/

static char Cmiss_value_FE_value_type_string[]="FE_value";

struct Cmiss_value_FE_value_type_specific_data
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
==============================================================================*/
{
	FE_value fe_value;
}; /* struct Cmiss_value_FE_value_type_specific_data */

static START_CMISS_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(FE_value)
{
	return_code=1;
}
END_CMISS_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(FE_value)

static START_CMISS_VALUE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(FE_value)
{
	destination->fe_value=source->fe_value;
}
END_CMISS_VALUE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(FE_value)

static START_CMISS_VALUE_GET_REALS_TYPE_SPECIFIC_FUNCTION(FE_value)
{
	FE_value *destination_real;

	if (!reals_address||ALLOCATE(destination_real,FE_value,1))
	{
		*number_of_reals_address=1;
		if (reals_address)
		{
			*reals_address=destination_real;
			*destination_real=data->fe_value;
		}
		return_code=1;
	}
}
END_CMISS_VALUE_GET_REALS_TYPE_SPECIFIC_FUNCTION(FE_value)

#define Cmiss_value_FE_value_get_string_type_specific Cmiss_value_default_get_string

static START_CMISS_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(
	FE_value)
{
	struct Cmiss_value_FE_value_type_specific_data *data_total,*data_1,*data_2;

	data_1=(struct Cmiss_value_FE_value_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_1);
	data_2=(struct Cmiss_value_FE_value_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_2);
	data_total=(struct Cmiss_value_FE_value_type_specific_data *)
		Cmiss_value_get_type_specific_data(total);
	ASSERT_IF(data_1&&data_2&&data_total,return_code,0)
	{
		data_total->fe_value += (data_1->fe_value)*(data_2->fe_value);
		return_code=1;
	}
}
END_CMISS_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(FE_value)

static START_CMISS_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(FE_value)
{
	return_code=1;
}
END_CMISS_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(FE_value)

static char Cmiss_value_FE_value_matrix_type_string[]="FE_value_matrix";

struct Cmiss_value_FE_value_matrix_type_specific_data
/*******************************************************************************
LAST MODIFIED : 14 February 2003

DESCRIPTION :
==============================================================================*/
{
	/* column number varying faster ie. row-wise */
	FE_value *fe_value_matrix;
	int number_of_columns,number_of_rows;
}; /* struct Cmiss_value_FE_value_matrix_type_specific_data */

static START_CMISS_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(FE_value_matrix)
{
	DEALLOCATE(data->fe_value_matrix);
	return_code=1;
}
END_CMISS_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(FE_value_matrix)

static START_CMISS_VALUE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(
	FE_value_matrix)
{
	FE_value *destination_value,*source_value;
	int number_of_values;

	if ((0<(number_of_values=(source->number_of_rows)*
		(source->number_of_columns)))&&(source_value=source->fe_value_matrix))
	{
		if (ALLOCATE(destination_value,FE_value,number_of_values))
		{
			destination->fe_value_matrix=destination_value;
			while (number_of_values>0)
			{
				*destination_value= *source_value;
				destination_value++;
				source_value++;
				number_of_values--;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_value_FE_value_matrix_duplicate_data_type_specific.  "
				"Could not allocate <fe_value_matrix>");
			DEALLOCATE(destination);
		}
	}
	else
	{
		destination->fe_value_matrix=(FE_value *)NULL;
	}
	if (destination)
	{
		destination->number_of_rows=source->number_of_rows;
		destination->number_of_columns=source->number_of_columns;
	}
}
END_CMISS_VALUE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(FE_value_matrix)

static START_CMISS_VALUE_GET_REALS_TYPE_SPECIFIC_FUNCTION(FE_value_matrix)
{
	FE_value *destination_real,*source_real;
	int number_of_reals;

	number_of_reals=(data->number_of_rows)*(data->number_of_columns);
	if (!reals_address||!(source_real=data->fe_value_matrix)||
		ALLOCATE(destination_real,FE_value,number_of_reals))
	{
		*number_of_reals_address=number_of_reals;
		if (reals_address)
		{
			if (source_real)
			{
				*reals_address=destination_real;
				while (number_of_reals>0)
				{
					*destination_real= *source_real;
					source_real++;
					destination_real++;
					number_of_reals--;
				}
			}
			else
			{
				*reals_address=(FE_value *)NULL;
			}
		}
		return_code=1;
	}
}
END_CMISS_VALUE_GET_REALS_TYPE_SPECIFIC_FUNCTION(FE_value_matrix)

#define Cmiss_value_FE_value_matrix_get_string_type_specific Cmiss_value_default_get_string

static START_CMISS_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(
	FE_value_matrix)
{
	FE_value *fe_value_1,*fe_value_2,*matrix_total,*matrix_1,*matrix_2,sum;
	int i,j,k,number_in_sum,total_number_of_columns,total_number_of_rows;
	struct Cmiss_value_FE_value_matrix_type_specific_data *data_total,*data_1,
		*data_2;

	data_1=(struct Cmiss_value_FE_value_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_1);
	data_2=(struct Cmiss_value_FE_value_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_2);
	data_total=(struct Cmiss_value_FE_value_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(total);
	ASSERT_IF(data_1&&data_2&&data_total,return_code,0)
	{
		total_number_of_rows=data_total->number_of_rows;
		total_number_of_columns=data_total->number_of_columns;
		number_in_sum=data_1->number_of_columns;
		if ((total_number_of_rows==data_1->number_of_rows)&&
			(number_in_sum==data_2->number_of_rows)&&
			(total_number_of_columns==data_2->number_of_columns))
		{
			matrix_total=data_total->fe_value_matrix;
			matrix_1=data_1->fe_value_matrix;
			matrix_2=data_2->fe_value_matrix;
			ASSERT_IF(matrix_total&&matrix_1&&matrix_2,return_code,0)
			{
				for (i=total_number_of_rows;i>0;i--)
				{
					fe_value_2=matrix_2;
					for (j=total_number_of_columns;j>0;j--)
					{
						fe_value_1=matrix_1;
						sum=(FE_value)0;
						for (k=number_in_sum;k>0;k--)
						{
							sum += (*fe_value_1)*(*fe_value_2);
							fe_value_1++;
							fe_value_2 += total_number_of_columns;
						}
						fe_value_2 -= (number_in_sum*total_number_of_columns)-1;
						*matrix_total += sum;
						matrix_total++;
					}
					matrix_1 += number_in_sum;
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_value_FE_value_matrix_multiply_and_accumulate_type_specific.  "
				"Matrices are of inconsistent sizes");
		}
	}
}
END_CMISS_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(
	FE_value_matrix)

static START_CMISS_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(
	FE_value_matrix)
{
	struct Cmiss_value_FE_value_matrix_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_value_FE_value_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_1);
	data_2=(struct Cmiss_value_FE_value_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_2);
	ASSERT_IF(data_1&&data_2,return_code,0)
	{
		if ((data_1->number_of_rows==data_2->number_of_rows)&&
			(data_1->number_of_columns==data_2->number_of_columns))
		{
			return_code=1;
		}
	}
}
END_CMISS_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(FE_value_matrix)

static char Cmiss_value_FE_value_vector_type_string[]="FE_value_vector";

struct Cmiss_value_FE_value_vector_type_specific_data
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
==============================================================================*/
{
	FE_value *fe_value_vector;
	int number_of_fe_values;
}; /* struct Cmiss_value_FE_value_vector_type_specific_data */

static START_CMISS_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(FE_value_vector)
{
	DEALLOCATE(data->fe_value_vector);
	return_code=1;
}
END_CMISS_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(FE_value_vector)

static START_CMISS_VALUE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(
	FE_value_vector)
{
	FE_value *destination_value,*source_value;
	int number_of_values;

	if ((0<(number_of_values=source->number_of_fe_values))&&
		(source_value=source->fe_value_vector))
	{
		if (ALLOCATE(destination_value,FE_value,number_of_values))
		{
			destination->fe_value_vector=destination_value;
			while (number_of_values>0)
			{
				*destination_value= *source_value;
				destination_value++;
				source_value++;
				number_of_values--;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_value_FE_value_vector_duplicate_data_type_specific.  "
				"Could not allocate <fe_value_vector>");
			DEALLOCATE(destination);
		}
	}
	else
	{
		destination->fe_value_vector=(FE_value *)NULL;
	}
	if (destination)
	{
		destination->number_of_fe_values=source->number_of_fe_values;
	}
}
END_CMISS_VALUE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(FE_value_vector)

static START_CMISS_VALUE_GET_REALS_TYPE_SPECIFIC_FUNCTION(FE_value_vector)
{
	FE_value *destination_real,*source_real;
	int number_of_reals;

	number_of_reals=data->number_of_fe_values;
	if (!reals_address||!(source_real=data->fe_value_vector)||
		ALLOCATE(destination_real,FE_value,number_of_reals))
	{
		*number_of_reals_address=number_of_reals;
		if (reals_address)
		{
			if (source_real)
			{
				*reals_address=destination_real;
				while (number_of_reals>0)
				{
					*destination_real= *source_real;
					source_real++;
					destination_real++;
					number_of_reals--;
				}
			}
			else
			{
				*reals_address=(FE_value *)NULL;
			}
		}
		return_code=1;
	}
}
END_CMISS_VALUE_GET_REALS_TYPE_SPECIFIC_FUNCTION(FE_value_vector)

#define Cmiss_value_FE_value_vector_get_string_type_specific Cmiss_value_default_get_string

static START_CMISS_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(
	FE_value_vector)
{
	int i,number_of_fe_values;
	struct Cmiss_value_FE_value_vector_type_specific_data *data_total,*data_1,
		*data_2;

	data_1=(struct Cmiss_value_FE_value_vector_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_1);
	data_2=(struct Cmiss_value_FE_value_vector_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_2);
	data_total=(struct Cmiss_value_FE_value_vector_type_specific_data *)
		Cmiss_value_get_type_specific_data(total);
	ASSERT_IF(data_1&&data_2&&data_total,return_code,0)
	{
		number_of_fe_values=data_1->number_of_fe_values;
		if ((number_of_fe_values==data_1->number_of_fe_values)&&
			(number_of_fe_values==data_2->number_of_fe_values))
		{
			ASSERT_IF((data_total->fe_value_vector)&&(data_1->fe_value_vector)&&
				(data_2->fe_value_vector),return_code,0)
			{
				for (i=0;i<number_of_fe_values;i++)
				{
					(data_total->fe_value_vector)[i] +=
						(data_1->fe_value_vector)[i]*(data_2->fe_value_vector)[i];
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_value_FE_value_vector_multiply_and_accumulate_type_specific.  "
				"Vectors are of different lengths");
		}
	}
}
END_CMISS_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(
	FE_value_vector)

static START_CMISS_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(
	FE_value_vector)
{
	struct Cmiss_value_FE_value_vector_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_value_FE_value_vector_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_1);
	data_2=(struct Cmiss_value_FE_value_vector_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_2);
	ASSERT_IF(data_1&&data_2,return_code,0)
	{
		if (data_1->number_of_fe_values==data_2->number_of_fe_values)
		{
			return_code=1;
		}
	}
}
END_CMISS_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(FE_value_vector)

/*
Global functions
----------------
*/

int Cmiss_value_FE_value_set_type(Cmiss_value_id value,
	FE_value fe_value)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Makes <value> of type FE_value and sets its <fe_value>.
==============================================================================*/
{
	int return_code;
	struct Cmiss_value_FE_value_type_specific_data *data;

	ENTER(Cmiss_value_FE_value_set_type);
	return_code=0;
	/* check arguments */
	if (value)
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,struct Cmiss_value_FE_value_type_specific_data,1))
		{
			/* 2.  Clear current type-specific data */
			Cmiss_value_clear_type(value);
			/* 3.  Establish the new type */
			Cmiss_value_set_type_specific_information(value,
				Cmiss_value_FE_value_type_string,(void *)data);
			data->fe_value=fe_value;
			/* set all the methods */
			return_code=CMISS_VALUE_ESTABLISH_METHODS(value,FE_value);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_value_FE_value_set_type.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_FE_value_set_type.  "
			"Missing value");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_FE_value_set_type */

DECLARE_CMISS_VALUE_IS_TYPE_FUNCTION(FE_value)

int Cmiss_value_FE_value_get_type(Cmiss_value_id value,
	FE_value *fe_value_address)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
If <value> is of type FE_value, gets its <*fe_value_address>.
==============================================================================*/
{
	int return_code;
	struct Cmiss_value_FE_value_type_specific_data *data;

	ENTER(Cmiss_value_FE_value_get_type);
	return_code=0;
	/* check arguments */
	if (value&&CMISS_VALUE_IS_TYPE(FE_value)(value)&&fe_value_address)
	{
		data=(struct Cmiss_value_FE_value_type_specific_data *)
			Cmiss_value_get_type_specific_data(value);
		ASSERT_IF(data,return_code,0)
		{
			*fe_value_address=data->fe_value;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_FE_value_get_type.  "
			"Invalid argument(s).  %p %p",value,fe_value_address);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_FE_value_get_type */

int Cmiss_value_FE_value_vector_set_type(Cmiss_value_id value,
	int number_of_fe_values,FE_value *fe_value_vector)
/*******************************************************************************
LAST MODIFIED : 27 April 2003

DESCRIPTION :
Makes <value> of type FE_value_vector and sets its <number_of_fe_values> and
<fe_value_vector>.  After success, the <value> is responsible for DEALLOCATEing
<fe_value_vector>.
==============================================================================*/
{
	int return_code;
	struct Cmiss_value_FE_value_vector_type_specific_data *data;

	ENTER(Cmiss_value_FE_value_vector_set_type);
	return_code=0;
	/* check arguments */
	if (value&&(0<=number_of_fe_values))
#if defined (OLD_CODE)
	if (value&&(((0<number_of_fe_values)&&fe_value_vector)||
		((0==number_of_fe_values)&&!fe_value_vector)))
#endif /* defined (OLD_CODE) */
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,struct Cmiss_value_FE_value_vector_type_specific_data,
			1))
		{
			/* 2.  Clear current type-specific data */
			Cmiss_value_clear_type(value);
			/* 3.  Establish the new type */
			Cmiss_value_set_type_specific_information(value,
				Cmiss_value_FE_value_vector_type_string,(void *)data);
			data->number_of_fe_values=number_of_fe_values;
			data->fe_value_vector=fe_value_vector;
			/* set all the methods */
			return_code=CMISS_VALUE_ESTABLISH_METHODS(value,FE_value_vector);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_value_FE_value_vector_set_type.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_FE_value_vector_set_type.  "
			"Invalid argument(s).  %p %d",value,number_of_fe_values);
#if defined (OLD_CODE)
		display_message(ERROR_MESSAGE,"Cmiss_value_FE_value_vector_set_type.  "
			"Invalid argument(s).  %p %d %p",value,number_of_fe_values,
			fe_value_vector);
#endif /* defined (OLD_CODE) */
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_FE_value_vector_set_type */

DECLARE_CMISS_VALUE_IS_TYPE_FUNCTION(FE_value_vector)

int Cmiss_value_FE_value_vector_get_type(Cmiss_value_id value,
	int *number_of_fe_values_address,FE_value **fe_value_vector_address)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
If <value> is of type FE_value_vector, gets its <*number_of_fe_values_address>
and <*fe_value_vector_address>.

The calling program must not DEALLOCATE the returned <*fe_value_vector_address>.
==============================================================================*/
{
	int return_code;
	struct Cmiss_value_FE_value_vector_type_specific_data *data;

	ENTER(Cmiss_value_FE_value_vector_get_type);
	return_code=0;
	/* check arguments */
	if (value&&CMISS_VALUE_IS_TYPE(FE_value_vector)(value)&&
		(number_of_fe_values_address||fe_value_vector_address))
	{
		data=(struct Cmiss_value_FE_value_vector_type_specific_data *)
			Cmiss_value_get_type_specific_data(value);
		ASSERT_IF(data,return_code,0)
		{
			if (number_of_fe_values_address)
			{
				*number_of_fe_values_address=data->number_of_fe_values;
			}
			if (fe_value_vector_address)
			{
				*fe_value_vector_address=data->fe_value_vector;
			}
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_FE_value_vector_get_type.  "
			"Invalid argument(s).  %p %p %p",value,number_of_fe_values_address,
			fe_value_vector_address);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_FE_value_vector_get_type */

int Cmiss_value_FE_value_matrix_set_type(Cmiss_value_id value,
	int number_of_rows,int number_of_columns,FE_value *fe_value_matrix)
/*******************************************************************************
LAST MODIFIED : 27 April 2003

DESCRIPTION :
Makes <value> of type FE_value_matrix and sets its <number_of_rows>,
<number_of_columns> and <fe_value_matrix> (column number varying fastest).
After success, the <value> is responsible for DEALLOCATEing <fe_value_matrix>.
==============================================================================*/
{
	int return_code;
	struct Cmiss_value_FE_value_matrix_type_specific_data *data;

	ENTER(Cmiss_value_FE_value_matrix_set_type);
	return_code=0;
	/* check arguments */
	if (value&&(0<=number_of_rows)&&(0<=number_of_columns))
#if defined (OLD_CODE)
	if (value&&(((0<number_of_rows)&&(0<number_of_columns)&&fe_value_matrix)||
		((0==number_of_rows)&&(0==number_of_columns)&&!fe_value_matrix)))
#endif /* defined (OLD_CODE) */
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,struct Cmiss_value_FE_value_matrix_type_specific_data,
			1))
		{
			/* 2.  Clear current type-specific data */
			Cmiss_value_clear_type(value);
			/* 3.  Establish the new type */
			Cmiss_value_set_type_specific_information(value,
				Cmiss_value_FE_value_matrix_type_string,(void *)data);
			data->number_of_rows=number_of_rows;
			data->number_of_columns=number_of_columns;
			data->fe_value_matrix=fe_value_matrix;
			/* set all the methods */
			return_code=CMISS_VALUE_ESTABLISH_METHODS(value,FE_value_matrix);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_value_FE_value_matrix_set_type.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_FE_value_matrix_set_type.  "
			"Invalid argument(s).  %p %d %d",value,number_of_rows,number_of_columns);
#if defined (OLD_CODE)
		display_message(ERROR_MESSAGE,"Cmiss_value_FE_value_matrix_set_type.  "
			"Invalid argument(s).  %p %d %d %p",value,number_of_rows,
			number_of_columns,fe_value_matrix);
#endif /* defined (OLD_CODE) */
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_FE_value_matrix_set_type */

DECLARE_CMISS_VALUE_IS_TYPE_FUNCTION(FE_value_matrix)

int Cmiss_value_FE_value_matrix_get_type(Cmiss_value_id value,
	int *number_of_rows_address,int *number_of_columns_address,
	FE_value **fe_value_matrix_address)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
If <value> is of type FE_value_matrix, gets its <*number_of_rows_address>,
<*number_of_columns_address> and <*fe_value_matrix_address>.

The calling program must not DEALLOCATE the returned <*fe_value_matrix_address>.
==============================================================================*/
{
	int return_code;
	struct Cmiss_value_FE_value_matrix_type_specific_data *data;

	ENTER(Cmiss_value_FE_value_matrix_get_type);
	return_code=0;
	/* check arguments */
	if (value&&CMISS_VALUE_IS_TYPE(FE_value_matrix)(value)&&
		(number_of_rows_address||number_of_columns_address||
		fe_value_matrix_address))
	{
		data=(struct Cmiss_value_FE_value_matrix_type_specific_data *)
			Cmiss_value_get_type_specific_data(value);
		ASSERT_IF(data,return_code,0)
		{
			if (number_of_rows_address)
			{
				*number_of_rows_address=data->number_of_rows;
			}
			if (number_of_columns_address)
			{
				*number_of_columns_address=data->number_of_columns;
			}
			if (fe_value_matrix_address)
			{
				*fe_value_matrix_address=data->fe_value_matrix;
			}
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_FE_value_matrix_get_type.  "
			"Invalid argument(s).  %p %p %p %p",value,number_of_rows_address,
			number_of_columns_address,fe_value_matrix_address);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_FE_value_matrix_get_type */
