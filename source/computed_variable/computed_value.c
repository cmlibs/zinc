/*******************************************************************************
FILE : computed_value.c

LAST MODIFIED : 14 February 2003

DESCRIPTION :
A module intended to replace general/value .  Testing and developing in
conjunction with Computed_variables.

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
static char computed_value_string_type_string[]="string";


/*
Module methods
--------------
*/
/*???DB.  Have put here because in general should not know definition of
	struct Computed_variable (may need for special ones like derivative?) */

static char computed_value_FE_value_type_string[]="FE_value";

struct Computed_value_FE_value_type_specific_data
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
==============================================================================*/
{
	FE_value fe_value;
}; /* struct Computed_value_FE_value_type_specific_data */

static START_COMPUTED_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(FE_value)
{
	return_code=1;
}
END_COMPUTED_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(FE_value)

static START_COMPUTED_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(
	FE_value)
{
	struct Computed_value_FE_value_type_specific_data *data_total,*data_1,*data_2;

	data_1=(struct Computed_value_FE_value_type_specific_data *)
		Computed_value_get_type_specific_data(value_1);
	data_2=(struct Computed_value_FE_value_type_specific_data *)
		Computed_value_get_type_specific_data(value_2);
	data_total=(struct Computed_value_FE_value_type_specific_data *)
		Computed_value_get_type_specific_data(total);
	ASSERT_IF(data_1&&data_2&&data_total,return_code,0)
	{
		data_total->fe_value += (data_1->fe_value)*(data_2->fe_value);
		return_code=1;
	}
}
END_COMPUTED_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(FE_value)

static START_COMPUTED_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(FE_value)
{
	return_code=1;
}
END_COMPUTED_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(FE_value)

static char computed_value_FE_value_vector_type_string[]="FE_value_vector";

struct Computed_value_FE_value_vector_type_specific_data
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
==============================================================================*/
{
	FE_value *fe_value_vector;
	int number_of_fe_values;
}; /* struct Computed_value_FE_value_vector_type_specific_data */

static START_COMPUTED_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(FE_value_vector)
{
	DEALLOCATE(data->fe_value_vector);
	return_code=1;
}
END_COMPUTED_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(FE_value_vector)

static START_COMPUTED_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(
	FE_value_vector)
{
	int i,number_of_fe_values;
	struct Computed_value_FE_value_vector_type_specific_data *data_total,*data_1,
		*data_2;

	data_1=(struct Computed_value_FE_value_vector_type_specific_data *)
		Computed_value_get_type_specific_data(value_1);
	data_2=(struct Computed_value_FE_value_vector_type_specific_data *)
		Computed_value_get_type_specific_data(value_2);
	data_total=(struct Computed_value_FE_value_vector_type_specific_data *)
		Computed_value_get_type_specific_data(total);
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
				"Computed_value_FE_value_vector_multiply_and_accumulate_type_specific.  "
				"Vectors are of different lengths");
		}
	}
}
END_COMPUTED_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(
	FE_value_vector)

static START_COMPUTED_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(
	FE_value_vector)
{
	struct Computed_value_FE_value_vector_type_specific_data *data_1,*data_2;

	data_1=(struct Computed_value_FE_value_vector_type_specific_data *)
		Computed_value_get_type_specific_data(value_1);
	data_2=(struct Computed_value_FE_value_vector_type_specific_data *)
		Computed_value_get_type_specific_data(value_2);
	ASSERT_IF(data_1&&data_2,return_code,0)
	{
		if (data_1->number_of_fe_values==data_2->number_of_fe_values)
		{
			return_code=1;
		}
	}
}
END_COMPUTED_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(FE_value_vector)

static char computed_value_FE_value_matrix_type_string[]="FE_value_matrix";

struct Computed_value_FE_value_matrix_type_specific_data
/*******************************************************************************
LAST MODIFIED : 14 February 2003

DESCRIPTION :
==============================================================================*/
{
	/* column number varing faster ie. row-wise */
	FE_value *fe_value_matrix;
	int number_of_columns,number_of_rows;
}; /* struct Computed_value_FE_value_matrix_type_specific_data */

static START_COMPUTED_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(FE_value_matrix)
{
	DEALLOCATE(data->fe_value_matrix);
	return_code=1;
}
END_COMPUTED_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(FE_value_matrix)

static START_COMPUTED_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(
	FE_value_matrix)
{
	FE_value *matrix_total,*matrix_1,*matrix_2,sum,*fe_value_1,*fe_value_2;
	int i,j,k,number_in_sum,total_number_of_columns,total_number_of_rows;
	struct Computed_value_FE_value_matrix_type_specific_data *data_total,*data_1,
		*data_2;

	data_1=(struct Computed_value_FE_value_matrix_type_specific_data *)
		Computed_value_get_type_specific_data(value_1);
	data_2=(struct Computed_value_FE_value_matrix_type_specific_data *)
		Computed_value_get_type_specific_data(value_2);
	data_total=(struct Computed_value_FE_value_matrix_type_specific_data *)
		Computed_value_get_type_specific_data(total);
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
				"Computed_value_FE_value_matrix_multiply_and_accumulate_type_specific.  "
				"Matrices are of inconsistent sizes");
		}
	}
}
END_COMPUTED_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(
	FE_value_matrix)

static START_COMPUTED_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(
	FE_value_matrix)
{
	struct Computed_value_FE_value_matrix_type_specific_data *data_1,*data_2;

	data_1=(struct Computed_value_FE_value_matrix_type_specific_data *)
		Computed_value_get_type_specific_data(value_1);
	data_2=(struct Computed_value_FE_value_matrix_type_specific_data *)
		Computed_value_get_type_specific_data(value_2);
	ASSERT_IF(data_1&&data_2,return_code,0)
	{
		if ((data_1->number_of_rows==data_2->number_of_rows)&&
			(data_1->number_of_columns==data_2->number_of_columns))
		{
			return_code=1;
		}
	}
}
END_COMPUTED_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(FE_value_matrix)

/*
Module types
------------
*/
struct Computed_value
/*******************************************************************************
LAST MODIFIED : 13 February 2003

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
		- a field here in struct Computed_value
		- a function type declaration in computed_value_private.h
		- an argument and an assigment to Computed_value_establish_methods
		The compiler will force the other changes because
		Computed_value_establish_methods is used in
		- Computed_value_clear_type
		- CREATE(Computed_value) */
	Computed_value_clear_type_specific_function
		computed_value_clear_type_specific_function;
	Computed_value_multiply_and_accumulate_type_specific_function
		computed_value_multiply_and_accumulate_type_specific_function;
	Computed_value_same_sub_type_type_specific_function
		computed_value_same_sub_type_type_specific_function;
		/*???DB.  More than just same type_string.  How to indicate this? */

	/*???DB.  Sufficiently light weight that access_count not needed? */
	int access_count;
};  /* struct Computed_value */

/*
Friend functions
----------------
*/
int Computed_value_establish_methods(struct Computed_value *value,
	Computed_value_clear_type_specific_function
	computed_value_clear_type_specific_function,
	Computed_value_multiply_and_accumulate_type_specific_function
	computed_value_multiply_and_accumulate_type_specific_function,
	Computed_value_same_sub_type_type_specific_function
	computed_value_same_sub_type_type_specific_function)
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Sets the methods for the <value>.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_value_establish_methods);
	return_code=0;
	/* check arguments */
	if (value)
	{
		value->computed_value_clear_type_specific_function=
			computed_value_clear_type_specific_function;
		value->computed_value_multiply_and_accumulate_type_specific_function=
			computed_value_multiply_and_accumulate_type_specific_function;
		value->computed_value_same_sub_type_type_specific_function=
			computed_value_same_sub_type_type_specific_function;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_value_establish_methods.  "
			"Missing value");
	}
	LEAVE;

	return (return_code);
} /* Computed_value_establish_methods */

Computed_value_type_specific_data *Computed_value_get_type_specific_data(
	struct Computed_value *value)
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Returns the type specific data for the <value>.
==============================================================================*/
{
	Computed_value_type_specific_data *data;

	ENTER(Computed_value_get_type_specific_data);
	data=(Computed_value_type_specific_data *)NULL;
	ASSERT_IF(value,data,(Computed_value_type_specific_data *)NULL)
	{
		data=value->type_specific_data;
	}
	LEAVE;

	return (data);
} /* Computed_value_get_type_specific_data */

int Computed_value_clear_type(struct Computed_value *value)
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Used internally by DESTROY and Computed_value_set_type_*() functions to
deallocate or deaccess data specific to any Computed_value_type.  Functions
changing the type of the Computed_value should
- allocate any dynamic data needed for the type
- call this function to clear what is currently in the value
- then set values
to ensure that the value is not left in an invalid state.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_value_clear_type);
	return_code=0;
	/* check arguments */
	if (value)
	{
		if (value->type_specific_data)
		{
			if (value->computed_value_clear_type_specific_function)
			{
				return_code=(value->computed_value_clear_type_specific_function)(value);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_value_clear_type.  "
					"Type specific data but no function to clear it");
				return_code=0;
			}
			DEALLOCATE(value->type_specific_data);
		}
		/* clear all methods */
		Computed_value_establish_methods(value,
			(Computed_value_clear_type_specific_function)NULL,
			(Computed_value_multiply_and_accumulate_type_specific_function)NULL,
			(Computed_value_same_sub_type_type_specific_function)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_value_establish_methods.  "
			"Missing value");
	}
	LEAVE;

	return (return_code);
} /* Computed_value_clear_type */

/*
Global functions
----------------
*/
struct Computed_value *CREATE(Computed_value)(void)
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Creates an empty value with no type.  Each type of value has its own "set_type"
function.
==============================================================================*/
{
	struct Computed_value *value;

	ENTER(CREATE(Computed_value));
	if (ALLOCATE(value,struct Computed_value,1))
	{
		/* initialise data */
		value->type_string=(char *)NULL;
		value->type_specific_data=NULL;
		/* initialise methods */
		Computed_value_establish_methods(value,
			(Computed_value_clear_type_specific_function)NULL,
			(Computed_value_multiply_and_accumulate_type_specific_function)NULL,
			(Computed_value_same_sub_type_type_specific_function)NULL);
		/* initialise access_count */
		value->access_count=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Computed_value).  "
			"Insufficient memory");
	}
	LEAVE;

	return (value);
} /* CREATE(Computed_value) */

int DESTROY(Computed_value)(struct Computed_value **value_address)
/*******************************************************************************
LAST MODIFIED : 2 February 2003

DESCRIPTION :
Frees memory/deaccess objects for Computed_value at <*value_address>.
==============================================================================*/
{
	int return_code;
	struct Computed_value *value;

	ENTER(DESTROY(Computed_value));
	return_code=0;
	if (value_address&&(value= *value_address))
	{
		Computed_value_clear_type(value);
		DEALLOCATE(*value_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Computed_value).  Missing value");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Computed_value) */

int Computed_value_same_sub_type(struct Computed_value *value_1,
	struct Computed_value *value_2)
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Returns nonzero if <value_1> and <value_2> have the same sub-type and zero
otherwise.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_value_same_sub_type);
	return_code=0;
	if (value_1&&value_2)
	{
		if (value_1->computed_value_same_sub_type_type_specific_function)
		{
			return_code=
				(value_1->computed_value_same_sub_type_type_specific_function)(value_1,
				value_2);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_value_same_sub_type.  "
				"Type specific data but no function to clear it");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_value_same_sub_type.  "
			"Invalid argument(s).  %p %p",value_1,value_2);
	}
	LEAVE;

	return (return_code);
} /* Computed_value_same_sub_type */

char *Computed_value_get_type_id_string(struct Computed_value *value)
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Returns the string which identifies the type.  The calling function must not
DEALLOCATE the returned string.
==============================================================================*/
{
	char *return_string;

	ENTER(Computed_value_get_type_id_string);
	return_string=(char *)NULL;
	if (value)
	{
		return_string=value->type_string;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_value_get_type_id_string.  "
			"Missing value");
	}
	LEAVE;

	return (return_string);
} /* Computed_value_get_type_id_string */

int Computed_value_set_type_FE_value(struct Computed_value *value,
	FE_value fe_value)
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Makes <value> of type FE_value and sets its <fe_value>.
==============================================================================*/
{
	int return_code;
	struct Computed_value_FE_value_type_specific_data *data;

	ENTER(Computed_value_set_type_FE_value);
	return_code=0;
	/* check arguments */
	if (value)
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,struct Computed_value_FE_value_type_specific_data,1))
		{
			/* 2.  Clear current type-specific data */
			Computed_value_clear_type(value);
			/* 3.  Establish the new type */
			value->type_string=computed_value_FE_value_type_string;
			value->type_specific_data=(void *)data;
			data->fe_value=fe_value;
			/* set all the methods */
			return_code=COMPUTED_VALUE_ESTABLISH_METHODS(value,FE_value);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_value_set_type_FE_value.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_value_set_type_FE_value.  "
			"Missing value");
	}
	LEAVE;

	return (return_code);
} /* Computed_value_set_type_FE_value */

DECLARE_COMPUTED_VALUE_IS_TYPE_FUNCTION(FE_value)

int Computed_value_get_type_FE_value(struct Computed_value *value,
	FE_value *fe_value_address)
/*******************************************************************************
LAST MODIFIED : 14 February 2003

DESCRIPTION :
If <value> is of type FE_value, gets its <*fe_value_address>.
==============================================================================*/
{
	int return_code;
	struct Computed_value_FE_value_type_specific_data *data;

	ENTER(Computed_value_get_type_FE_value);
	return_code=0;
	/* check arguments */
	if (value&&fe_value_address)
	{
		data=(struct Computed_value_FE_value_type_specific_data *)
			Computed_value_get_type_specific_data(value);
		ASSERT_IF(data,return_code,0)
		{
			*fe_value_address=data->fe_value;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_value_get_type_FE_value.  "
			"Invalid argument(s).  %p %p",value,fe_value_address);
	}
	LEAVE;

	return (return_code);
} /* Computed_value_get_type_FE_value */

int Computed_value_set_type_FE_value_vector(struct Computed_value *value,
	int number_of_fe_values,FE_value *fe_value_vector)
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Makes <value> of type FE_value_vector and sets its <number_of_fe_values> and
<fe_value_vector>.  After success, the <value> is responsible for DEALLOCATEing
<fe_value_vector>.
==============================================================================*/
{
	int return_code;
	struct Computed_value_FE_value_vector_type_specific_data *data;

	ENTER(Computed_value_set_type_FE_value_vector);
	return_code=0;
	/* check arguments */
	if (value&&(((0<number_of_fe_values)&&fe_value_vector)||
		((0==number_of_fe_values)&&!fe_value_vector)))
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,struct Computed_value_FE_value_vector_type_specific_data,
			1))
		{
			/* 2.  Clear current type-specific data */
			Computed_value_clear_type(value);
			/* 3.  Establish the new type */
			value->type_string=computed_value_FE_value_vector_type_string;
			value->type_specific_data=(void *)data;
			data->number_of_fe_values=number_of_fe_values;
			data->fe_value_vector=fe_value_vector;
			/* set all the methods */
			return_code=COMPUTED_VALUE_ESTABLISH_METHODS(value,FE_value_vector);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_value_set_type_FE_value_vector.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_value_set_type_FE_value_vector.  "
			"Invalid argument(s).  %p %d %p",value,number_of_fe_values,
			fe_value_vector);
	}
	LEAVE;

	return (return_code);
} /* Computed_value_set_type_FE_value_vector */

DECLARE_COMPUTED_VALUE_IS_TYPE_FUNCTION(FE_value_vector)

int Computed_value_get_type_FE_value_vector(struct Computed_value *value,
	int *number_of_fe_values_address,FE_value **fe_value_vector_address)
/*******************************************************************************
LAST MODIFIED : 14 February 2003

DESCRIPTION :
If <value> is of type FE_value_vector, gets its <*number_of_fe_values_address>
and <*fe_value_vector_address>.

The calling program must not DEALLOCATE the returned <*fe_value_vector_address>.
==============================================================================*/
{
	int return_code;
	struct Computed_value_FE_value_vector_type_specific_data *data;

	ENTER(Computed_value_get_type_FE_value_vector);
	return_code=0;
	/* check arguments */
	if (value&&(number_of_fe_values_address||fe_value_vector_address))
	{
		data=(struct Computed_value_FE_value_vector_type_specific_data *)
			Computed_value_get_type_specific_data(value);
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
		display_message(ERROR_MESSAGE,"Computed_value_get_type_FE_value_vector.  "
			"Invalid argument(s).  %p %p %p",value,number_of_fe_values_address,
			fe_value_vector_address);
	}
	LEAVE;

	return (return_code);
} /* Computed_value_get_type_FE_value_vector */

int Computed_value_set_type_FE_value_matrix(struct Computed_value *value,
	int number_of_rows,int number_of_columns,FE_value *fe_value_matrix)
/*******************************************************************************
LAST MODIFIED : 14 February 2003

DESCRIPTION :
Makes <value> of type FE_value_matrix and sets its <number_of_rows>,
<number_of_columns> and <fe_value_matrix> (column number varying fastest).
After success, the <value> is responsible for DEALLOCATEing <fe_value_matrix>.
==============================================================================*/
{
	int return_code;
	struct Computed_value_FE_value_matrix_type_specific_data *data;

	ENTER(Computed_value_set_type_FE_value_matrix);
	return_code=0;
	/* check arguments */
	if (value&&(((0<number_of_rows)&&(0<number_of_columns)&&fe_value_matrix)||
		((0==number_of_rows)&&(0==number_of_columns)&&!fe_value_matrix)))
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,struct Computed_value_FE_value_matrix_type_specific_data,
			1))
		{
			/* 2.  Clear current type-specific data */
			Computed_value_clear_type(value);
			/* 3.  Establish the new type */
			value->type_string=computed_value_FE_value_matrix_type_string;
			value->type_specific_data=(void *)data;
			data->number_of_rows=number_of_rows;
			data->number_of_columns=number_of_columns;
			data->fe_value_matrix=fe_value_matrix;
			/* set all the methods */
			return_code=COMPUTED_VALUE_ESTABLISH_METHODS(value,FE_value_matrix);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_value_set_type_FE_value_matrix.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_value_set_type_FE_value_matrix.  "
			"Invalid argument(s).  %p %d %d %p",value,number_of_rows,
			number_of_columns,fe_value_matrix);
	}
	LEAVE;

	return (return_code);
} /* Computed_value_set_type_FE_value_matrix */

DECLARE_COMPUTED_VALUE_IS_TYPE_FUNCTION(FE_value_matrix)

int Computed_value_get_type_FE_value_matrix(struct Computed_value *value,
	int *number_of_rows_address,int *number_of_columns_address,
	FE_value **fe_value_matrix_address)
/*******************************************************************************
LAST MODIFIED : 14 February 2003

DESCRIPTION :
If <value> is of type FE_value_matrix, gets its <*number_of_rows_address>,
<*number_of_columns_address> and <*fe_value_matrix_address>.

The calling program must not DEALLOCATE the returned <*fe_value_matrix_address>.
==============================================================================*/
{
	int return_code;
	struct Computed_value_FE_value_matrix_type_specific_data *data;

	ENTER(Computed_value_get_type_FE_value_matrix);
	return_code=0;
	/* check arguments */
	if (value&&(number_of_rows_address||number_of_columns_address||
		fe_value_matrix_address))
	{
		data=(struct Computed_value_FE_value_matrix_type_specific_data *)
			Computed_value_get_type_specific_data(value);
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
		display_message(ERROR_MESSAGE,"Computed_value_get_type_FE_value_matrix.  "
			"Invalid argument(s).  %p %p %p %p",value,number_of_rows_address,
			number_of_columns_address,fe_value_matrix_address);
	}
	LEAVE;

	return (return_code);
} /* Computed_value_get_type_FE_value_matrix */

int Computed_value_set_type_string(struct Computed_value *value,char *string);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Makes <value> of type string and sets its <string>.  After success, the <value>
is responsible for DEALLOCATEing <string>.
==============================================================================*/

DECLARE_COMPUTED_VALUE_IS_TYPE_FUNCTION(string)

int Computed_value_get_type_string(struct Computed_value *value,
	char **string_address);
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
If <value> is of type string, gets its <*string_address>.

The calling program must not DEALLOCATE the returned <*string_address>.
==============================================================================*/

int Computed_value_multiply_and_accumulate(struct Computed_value *value_1,
	struct Computed_value *value_2,struct Computed_value *total)
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Calculates <total>+<value_1>*<value_2> and puts in <total>.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_value_multiply_and_accumulate);
	return_code=0;
	/* check arguments */
	if (value_1&&value_2&&total)
	{
		if (total->computed_value_multiply_and_accumulate_type_specific_function)
		{
			return_code=(total->
				computed_value_multiply_and_accumulate_type_specific_function)(value_1,
				value_2,total);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_value_multiply_and_accumulate.  "
				"Type specific data but no function to clear it");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_value_multiply_and_accumulate.  "
			"Invalid argument(s).  %p %p",value_1,value_2);
	}
	LEAVE;

	return (return_code);
} /* Computed_value_multiply_and_accumulate */
