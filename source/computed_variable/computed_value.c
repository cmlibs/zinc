/*******************************************************************************
FILE : computed_value.c

LAST MODIFIED : 27 April 2003

DESCRIPTION :
A module intended to replace general/value .  Testing and developing in
conjunction with Cmiss_variables.

???DB.  Should _get_s try and get a representation if not specified type?
==============================================================================*/
#include "computed_variable/computed_value.h"
#include "computed_variable/computed_value_private.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Module constants
----------------
*/
static char Cmiss_value_string_type_string[]="String";


/*
Module methods
--------------
*/
/*???DB.  Have put here because in general should not know definition of
	struct Cmiss_value */

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
Module types
------------
*/
struct Cmiss_value
/*******************************************************************************
LAST MODIFIED : 22 April 2003

DESCRIPTION :
A value that knows what type it is.

???DB.  An update of Value_type (general/value).  Want to be able to add new
	value types (in the same way that Computed_field types can be added).  Will
	need a package?  Will replace Value_type?
???DB.  At present just extend?
==============================================================================*/
{
	/* the type string identifies the value's type.  It points to a string that
		is shared by all variables of the same type and should not be copied */
	char *type_string;
	/* information that is specific to the type */
	void *type_specific_data;
	/* methods.   To add a new method, add
		- a field here in struct Cmiss_value
		- a function type declaration in computed_value_private.h
		- an argument and an assigment to Cmiss_value_establish_methods
		The compiler will force the other changes because
		Cmiss_value_establish_methods is used in
		- Cmiss_value_clear_type
		- CREATE(Cmiss_value) */
	Cmiss_value_clear_type_specific_function clear_type_specific_function;
	Cmiss_value_duplicate_data_type_specific_function
		duplicate_data_type_specific_function;
	Cmiss_value_multiply_and_accumulate_type_specific_function
		multiply_and_accumulate_type_specific_function;
	Cmiss_value_same_sub_type_type_specific_function
		same_sub_type_type_specific_function;
		/*???DB.  More than just same type_string.  How to indicate this? */
	int access_count;
};  /* struct Cmiss_value */

/*
Friend functions
----------------
*/
int Cmiss_value_establish_methods(Cmiss_value_id value,
	Cmiss_value_clear_type_specific_function clear_type_specific_function,
	Cmiss_value_duplicate_data_type_specific_function
	duplicate_data_type_specific_function,
	Cmiss_value_multiply_and_accumulate_type_specific_function
	multiply_and_accumulate_type_specific_function,
	Cmiss_value_same_sub_type_type_specific_function
	same_sub_type_type_specific_function)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Sets the methods for the <value>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_value_establish_methods);
	return_code=0;
	/* check arguments */
	if (value)
	{
		value->clear_type_specific_function=clear_type_specific_function;
		value->duplicate_data_type_specific_function=
			duplicate_data_type_specific_function;
		value->multiply_and_accumulate_type_specific_function=
			multiply_and_accumulate_type_specific_function;
		value->same_sub_type_type_specific_function=
			same_sub_type_type_specific_function;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_establish_methods.  "
			"Missing value");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_establish_methods */

Cmiss_value_type_specific_data *Cmiss_value_get_type_specific_data(
	Cmiss_value_id value)
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Returns the type specific data for the <value>.
==============================================================================*/
{
	Cmiss_value_type_specific_data *data;

	ENTER(Cmiss_value_get_type_specific_data);
	data=(Cmiss_value_type_specific_data *)NULL;
	ASSERT_IF(value,data,(Cmiss_value_type_specific_data *)NULL)
	{
		data=value->type_specific_data;
	}
	LEAVE;

	return (data);
} /* Cmiss_value_get_type_specific_data */

int Cmiss_value_set_type_specific_information(Cmiss_value_id value,
	char *type_string,void *type_specific_data)
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
Sets the type specific information for the <value>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_value_set_type_specific_information);
	return_code=0;
	/* check arguments */
	if (value&&type_string&&type_specific_data)
	{
		value->type_specific_data=type_specific_data;
		value->type_string=type_string;
		return_code=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_value_set_type_specific_information.  "
			"Invalid argument(s).  %p %p %p",value,type_string,type_specific_data);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_set_type_specific_information */

int Cmiss_value_clear_type(Cmiss_value_id value)
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
Used internally by DESTROY and Cmiss_value_set_type_*() functions to
deallocate or deaccess data specific to any Cmiss_value_type.  Functions
changing the type of the Cmiss_value should
- allocate any dynamic data needed for the type
- call this function to clear what is currently in the value
- then set values
to ensure that the value is not left in an invalid state.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_value_clear_type);
	return_code=0;
	/* check arguments */
	if (value)
	{
		if (value->type_specific_data)
		{
			if (value->clear_type_specific_function)
			{
				return_code=(value->clear_type_specific_function)(value);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_value_clear_type.  "
					"Type specific data but no function to clear it");
				return_code=0;
			}
			DEALLOCATE(value->type_specific_data);
		}
		/* clear all methods */
		Cmiss_value_establish_methods(value,
			(Cmiss_value_clear_type_specific_function)NULL,
			(Cmiss_value_duplicate_data_type_specific_function)NULL,
			(Cmiss_value_multiply_and_accumulate_type_specific_function)NULL,
			(Cmiss_value_same_sub_type_type_specific_function)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_establish_methods.  "
			"Missing value");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_clear_type */

/*
Global functions
----------------
*/
Cmiss_value_id CREATE(Cmiss_value)(void)
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
Creates an empty value with no type.  Each type of value has its own "set_type"
function.
==============================================================================*/
{
	Cmiss_value_id value;

	ENTER(CREATE(Cmiss_value));
	if (ALLOCATE(value,struct Cmiss_value,1))
	{
		/* initialise data */
		value->type_string=(char *)NULL;
		value->type_specific_data=NULL;
		/* initialise methods */
		Cmiss_value_establish_methods(value,
			(Cmiss_value_clear_type_specific_function)NULL,
			(Cmiss_value_duplicate_data_type_specific_function)NULL,
			(Cmiss_value_multiply_and_accumulate_type_specific_function)NULL,
			(Cmiss_value_same_sub_type_type_specific_function)NULL);
		/* initialise access_count */
		value->access_count=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Cmiss_value).  "
			"Insufficient memory");
	}
	LEAVE;

	return (value);
} /* CREATE(Cmiss_value) */

int DESTROY(Cmiss_value)(Cmiss_value_id *value_address)
/*******************************************************************************
LAST MODIFIED : 2 February 2003

DESCRIPTION :
Frees memory/deaccess objects for Cmiss_value at <*value_address>.
==============================================================================*/
{
	int return_code;
	Cmiss_value_id value;

	ENTER(DESTROY(Cmiss_value));
	return_code=0;
	if (value_address&&(value= *value_address))
	{
		Cmiss_value_clear_type(value);
		DEALLOCATE(*value_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Cmiss_value).  Missing value");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Cmiss_value) */

int Cmiss_value_copy(Cmiss_value_id destination,
	Cmiss_value_id source)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Copies the type and contents from <source> to <destination>.

???DB.  What if the access_count>0?  Put in Cmiss_value_clear_type?
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_value_copy);
	return_code=0;
	/* check arguments */
	if (destination&&source)
	{
		/* clear destination */
		if (Cmiss_value_clear_type(destination))
		{
			/* initialise data */
			destination->type_string=source->type_string;
			if (source->duplicate_data_type_specific_function)
			{
				destination->type_specific_data=
					(source->duplicate_data_type_specific_function)(source);
			}
			/* initialise methods */
			Cmiss_value_establish_methods(destination,
				source->clear_type_specific_function,
				source->duplicate_data_type_specific_function,
				source->multiply_and_accumulate_type_specific_function,
				source->same_sub_type_type_specific_function);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_copy.  "
			"Invalid argument(s).  %p %p",destination,source);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_copy */

DECLARE_OBJECT_FUNCTIONS(Cmiss_value)

int Cmiss_value_same_sub_type(Cmiss_value_id value_1,
	Cmiss_value_id value_2)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Returns nonzero if <value_1> and <value_2> have the same sub-type and zero
otherwise.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_value_same_sub_type);
	return_code=0;
	if (value_1&&value_2)
	{
		if (value_1->same_sub_type_type_specific_function)
		{
			return_code=(value_1->same_sub_type_type_specific_function)(value_1,
				value_2);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_value_same_sub_type.  "
				"Type specific data but no function to clear it");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_same_sub_type.  "
			"Invalid argument(s).  %p %p",value_1,value_2);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_same_sub_type */

char *Cmiss_value_get_type_id_string(Cmiss_value_id value)
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Returns the string which identifies the type.  The calling function must not
DEALLOCATE the returned string.
==============================================================================*/
{
	char *return_string;

	ENTER(Cmiss_value_get_type_id_string);
	return_string=(char *)NULL;
	if (value)
	{
		return_string=value->type_string;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_get_type_id_string.  "
			"Missing value");
	}
	LEAVE;

	return (return_string);
} /* Cmiss_value_get_type_id_string */

int Cmiss_value_multiply_and_accumulate(Cmiss_value_id total,
	Cmiss_value_id value_1,Cmiss_value_id value_2)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Calculates <total>+<value_1>*<value_2> and puts in <total>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_value_multiply_and_accumulate);
	return_code=0;
	/* check arguments */
	if (value_1&&value_2&&total)
	{
		if (total->multiply_and_accumulate_type_specific_function)
		{
			return_code=(total->multiply_and_accumulate_type_specific_function)(
				total,value_1,value_2);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_value_multiply_and_accumulate.  "
				"Type specific data but no function to clear it");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_multiply_and_accumulate.  "
			"Invalid argument(s).  %p %p",value_1,value_2);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_multiply_and_accumulate */

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

int Cmiss_value_string_set_type(Cmiss_value_id value,char *string);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Makes <value> of type string and sets its <string>.  After success, the <value>
is responsible for DEALLOCATEing <string>.
==============================================================================*/

DECLARE_CMISS_VALUE_IS_TYPE_FUNCTION(string)

int Cmiss_value_string_get_type(Cmiss_value_id value,char **string_address);
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
If <value> is of type string, gets its <*string_address>.

The calling program must not DEALLOCATE the returned <*string_address>.
==============================================================================*/
