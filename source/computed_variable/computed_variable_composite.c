/*******************************************************************************
FILE : computed_variable_composite.c

LAST MODIFIED : 21 July 2003

DESCRIPTION :
Implements the composite computed variable - its result is a vector containing
the results of the variables it is made up of.

???DB.  Start with compositing variables whose results can be represented as a
	number of reals

???DB.  Composite of vectors is the "flattened" vector?

???DB.  Combine with composition?

???DB.  How to do derivative with respect to a composite?  Other variable types
	shouldn't have to know about it.  Put composite in computed_variable?  Add
	another method (How could this work?) ?
==============================================================================*/
#include "computed_variable/computed_value_derivative_matrix.h"
#include "computed_variable/computed_value_fe_value.h"
#include "computed_variable/computed_value_matrix.h"
#include "computed_variable/computed_variable_composite.h"
#include "computed_variable/computed_variable_private.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Module constants
----------------
*/
static char Cmiss_variable_composite_type_string[]="Composite";

/*
Module types
------------
*/
struct Cmiss_variable_composite_type_specific_data
/*******************************************************************************
LAST MODIFIED : 15 July 2003

DESCRIPTION :
Combines variables together in a vector.
==============================================================================*/
{
	Cmiss_variable_id *variables;
	int number_of_variables;
}; /* struct Cmiss_variable_composite_type_specific_data */

/*
Module functions
----------------
*/

/*
Module methods
--------------
*/
static START_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(composite)
{
	int i;

	if (data->variables)
	{
		for (i=0;i<data->number_of_variables;i++)
		{
			DEACCESS(Cmiss_variable)(&((data->variables)[i]));
		}
		DEALLOCATE(data->variables);
	}
	return_code=1;
}
END_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(composite)

static START_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(composite)
{
	int i;

	if (0<source->number_of_variables)
	{
		ALLOCATE(destination->variables,Cmiss_variable_id,
			source->number_of_variables);
		if (destination->variables)
		{
			destination->number_of_variables=source->number_of_variables;
			for (i=0;i<source->number_of_variables;i++)
			{
				(destination->variables)[i]=
					ACCESS(Cmiss_variable)((source->variables)[i]);
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
END_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(composite)

static START_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	composite)
{
	Cmiss_value_id *derivative_matrices,*matrices,**sub_matrices;
	Cmiss_variable_id *local_independent_variables;
	int i,j,number_of_columns,number_of_matrices,number_of_rows,
		total_number_of_rows;
	struct Cmiss_variable_composite_type_specific_data *data;
	struct Matrix *matrix_temp;

	data=(struct Cmiss_variable_composite_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(0<data->number_of_variables)&&(data->variables),return_code,
		0)
	{
		ALLOCATE(derivative_matrices,Cmiss_value_id,data->number_of_variables);
		ALLOCATE(sub_matrices,Cmiss_value_id *,data->number_of_variables);
		if (derivative_matrices&&sub_matrices)
		{
			/* evaluate derivatives for individual variables */
			return_code=1;
			i=0;
			total_number_of_rows=0;
			while (return_code&&(i<data->number_of_variables))
			{
				if (derivative_matrices[i]=CREATE(Cmiss_value)())
				{
					ACCESS(Cmiss_value)(derivative_matrices[i]);
					if (return_code=Cmiss_variable_evaluate_derivative(
						(data->variables)[i],order,independent_variables,
						(struct LIST(Cmiss_variable_value) *)NULL,derivative_matrices[i]))
					{
						if (return_code=Cmiss_value_derivative_matrix_get_type(
							derivative_matrices[i],(Cmiss_variable_id *)NULL,(int *)NULL,
							(Cmiss_variable_id **)NULL,sub_matrices+i))
						{
							if (return_code=Cmiss_value_matrix_get_dimensions(
								sub_matrices[i][0],&number_of_rows,(int *)NULL))
							{
								total_number_of_rows += number_of_rows;
							}
						}
					}
					i++;
				}
				else
				{
					return_code=0;
				}
			}
			if (return_code)
			{
				number_of_matrices=1;
				for (i=0;i<order;i++)
				{
					number_of_matrices *= 2;
				}
				number_of_matrices -= 1;
				if (ALLOCATE(matrices,Cmiss_value_id,number_of_matrices))
				{
					i=0;
					while (return_code&&(i<number_of_matrices))
					{
						if (return_code=Cmiss_value_matrix_get_dimensions(
							sub_matrices[0][i],(int *)NULL,&number_of_columns)&&(matrix_temp=
							CREATE(Matrix)("matrix",DENSE,total_number_of_rows,
							number_of_columns))&&(matrices[i]=CREATE(Cmiss_value)())&&
							Cmiss_value_matrix_set_type(matrices[i],matrix_temp))
						{
							ACCESS(Cmiss_value)(matrices[i]);
							j=0;
							total_number_of_rows=0;
							while (return_code&&(j<data->number_of_variables))
							{
								if (return_code=Cmiss_value_matrix_get_dimensions(
									sub_matrices[j][i],&number_of_rows,(int *)NULL))
								{
									return_code=Cmiss_value_matrix_set_submatrix(matrices[i],
										total_number_of_rows+1,total_number_of_rows+number_of_rows,
										1,number_of_columns,sub_matrices[j][i]);
									total_number_of_rows += number_of_rows;
								}
								j++;
							}
							i++;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_variable_composite_evaluate_derivative_type_specific.  "
								"Could not set up matrices[i]. %d %p %p",return_code,
								matrix_temp,matrices[i]);
							if (return_code)
							{
								if (matrix_temp)
								{
									if (matrices[i])
									{
										DESTROY(Cmiss_value)(matrices+i);
									}
									DESTROY(Matrix)(&matrix_temp);
								}
								return_code=0;
							}
						}
					}
					if (return_code)
					{
						/* duplicate independent variables */
						if (ALLOCATE(local_independent_variables,Cmiss_variable_id,order))
						{
							for (i=0;i<order;i++)
							{
								local_independent_variables[i]=independent_variables[i];
							}
							if (return_code=Cmiss_value_derivative_matrix_set_type(value,
								variable,order,local_independent_variables,matrices))
							{
								/* local_independent_variables and matrices are now managed by
									value */
								local_independent_variables=(Cmiss_variable_id *)NULL;
								matrices=(Cmiss_value_id *)NULL;
							}
							if (local_independent_variables)
							{
								DEALLOCATE(local_independent_variables);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_variable_composite_evaluate_derivative_type_specific.  "
								"Could not allocate local_independent_variables");
							return_code=0;
						}
						if (matrices)
						{
							for (i=0;i<number_of_matrices;i++)
							{
								DEACCESS(Cmiss_value)(matrices+i);
							}
						}
					}
					else
					{
						while (i>0)
						{
							i--;
							DEACCESS(Cmiss_value)(matrices+i);
						}
					}
					if (matrices)
					{
						DEALLOCATE(matrices);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_variable_composite_evaluate_derivative_type_specific.  "
						"Could not allocate matrices");
					return_code=0;
				}
				for (i=0;i<data->number_of_variables;i++)
				{
					DEACCESS(Cmiss_value)(derivative_matrices+i);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_composite_evaluate_derivative_type_specific.  "
					"Could not evaluate derivative_matrices");
				while (i>0)
				{
					i--;
					DEACCESS(Cmiss_value)(derivative_matrices+i);
				}
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_composite_evaluate_derivative_type_specific.  "
				"Could not allocate derivative_matrices (%p) or sub_matrices (%p)",
				derivative_matrices,sub_matrices);
			return_code=0;
		}
		if (sub_matrices)
		{
			DEALLOCATE(sub_matrices);
		}
		if (derivative_matrices)
		{
			DEALLOCATE(derivative_matrices);
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(composite)

static START_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(composite)
{
	Cmiss_value_id temp_value;
	Cmiss_variable_id *variable_address;
	FE_value *destination_fe_value,*fe_value_vector,*reals,*source_fe_value;
	int i,number_of_fe_values,number_of_reals;
	struct Cmiss_variable_composite_type_specific_data *data;
	struct Cmiss_variable_value *temp_variable_value;

	data=(struct Cmiss_variable_composite_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(0<data->number_of_variables)&&(data->variables),return_code,
		0)
	{
		if (temp_value=CREATE(Cmiss_value)())
		{
			ACCESS(Cmiss_value)(temp_value);
			i=data->number_of_variables;
			variable_address=data->variables;
			return_code=1;
			number_of_fe_values=0;
			fe_value_vector=(FE_value *)NULL;
			while (return_code&&(i>0))
			{
				if (temp_variable_value=CREATE(Cmiss_variable_value)(
					*variable_address,temp_value))
				{
					ACCESS(Cmiss_variable_value)(temp_variable_value);
					if (return_code=Cmiss_variable_evaluate(temp_variable_value,
						(struct LIST(Cmiss_variable_value) *)NULL))
					{
						if (return_code=Cmiss_value_get_reals(temp_value,&number_of_reals,
							&reals))
						{
							if (REALLOCATE(destination_fe_value,fe_value_vector,FE_value,
								number_of_fe_values+number_of_reals))
							{
								fe_value_vector=destination_fe_value;
								destination_fe_value += number_of_fe_values;
								number_of_fe_values += number_of_reals;
								source_fe_value=reals;
								while (number_of_reals>0)
								{
									*destination_fe_value= *source_fe_value;
									source_fe_value++;
									destination_fe_value++;
									number_of_reals--;
								}
							}
							else
							{
								return_code=0;
							}
							DEALLOCATE(reals);
						}
					}
					DEACCESS(Cmiss_variable_value)(&temp_variable_value);
				}
				else
				{
					return_code=0;
				}
				variable_address++;
				i--;
			}
			if (return_code)
			{
				return_code=Cmiss_value_FE_value_vector_set_type(value,
					number_of_fe_values,fe_value_vector);
			}
			if (!return_code)
			{
				DEALLOCATE(fe_value_vector);
			}
			DEACCESS(Cmiss_value)(&temp_value);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_composite_evaluate_type_specific.  "
				"Could not create temp_value");
			return_code=0;
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(composite)

static
	START_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	composite)
{
	Cmiss_variable_id *variable_address;
	int i;
	struct Cmiss_variable_composite_type_specific_data *data;

	data=(struct Cmiss_variable_composite_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(0<data->number_of_variables)&&(data->variables),return_code,
		0)
	{
		i=data->number_of_variables;
		variable_address=data->variables;
		while ((i>0)&&!(return_code=Cmiss_variable_get_independent_variable_value(
			*variable_address,independent_variable,value)))
		{
			i--;
			variable_address++;
		}
	}
}
END_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	composite)

static
	START_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	composite)
{
	Cmiss_value_id temp_value;
	Cmiss_variable_id *variable_address;
	int i;
	struct Cmiss_variable_composite_type_specific_data *data;

	data=(struct Cmiss_variable_composite_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(0<data->number_of_variables)&&(data->variables),return_code,
		0)
	{
		if (temp_value=CREATE(Cmiss_value)())
		{
			ACCESS(Cmiss_value)(temp_value);
			if (Cmiss_value_copy(temp_value,value))
			{
				i=data->number_of_variables;
				variable_address=data->variables;
				return_code=0;
				while ((i>0)&&!return_code)
				{
					return_code=Cmiss_variable_get_set_independent_variable_value(
						*variable_address,independent_variable,value);
					i--;
					variable_address++;
				}
				while (i>0)
				{
					Cmiss_variable_set_independent_variable_value(*variable_address,
						independent_variable,temp_value);
					i--;
					variable_address++;
				}
			}
			DEACCESS(Cmiss_value)(&temp_value);
		}
	}
}
END_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	composite)

static START_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(composite)
{
	Cmiss_value_id temp_value;
	Cmiss_variable_id *variable_address;
	int i,number_of_fe_values,number_of_reals;
	struct Cmiss_variable_composite_type_specific_data *data;

	data=(struct Cmiss_variable_composite_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(0<data->number_of_variables)&&(data->variables),return_code,
		0)
	{
		if (temp_value=CREATE(Cmiss_value)())
		{
			ACCESS(Cmiss_value)(temp_value);
			i=data->number_of_variables;
			variable_address=data->variables;
			return_code=1;
			number_of_fe_values=0;
			while (return_code&&(i>0))
			{
				if (return_code=Cmiss_variable_get_value_type(*variable_address,
					temp_value))
				{
					if (return_code=Cmiss_value_get_reals(temp_value,&number_of_reals,
						(FE_value **)NULL))
					{
						number_of_fe_values += number_of_reals;
					}
				}
				variable_address++;
				i--;
			}
			if (return_code)
			{
				return_code=Cmiss_value_FE_value_vector_set_type(type,
					number_of_fe_values,(FE_value *)NULL);
			}
			DEACCESS(Cmiss_value)(&temp_value);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_composite_evaluate_type_specific.  "
				"Could not create temp_value");
			return_code=0;
		}
	}
}
END_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(composite)

static START_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(composite)
{
	Cmiss_variable_id *variable_address;
	int i;
	struct Cmiss_variable_composite_type_specific_data *data;

	data=(struct Cmiss_variable_composite_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(0<data->number_of_variables)&&(data->variables),return_code,
		0)
	{
		i=data->number_of_variables;
		variable_address=data->variables;
		while ((i>0)&&(return_code=Cmiss_variable_is_defined(*variable_address,
			values)))
		{
			i--;
			variable_address++;
		}
	}
}
END_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(composite)

static START_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	composite)
{
	Cmiss_variable_id *variable_address;
	int i;
	struct Cmiss_variable_composite_type_specific_data *data;

	data=(struct Cmiss_variable_composite_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(0<data->number_of_variables)&&(data->variables),return_code,
		0)
	{
		i=data->number_of_variables;
		variable_address=data->variables;
		while ((i>0)&&!(return_code=
			Cmiss_variable_is_independent_variable_of(*variable_address,
			independent_variable)))
		{
			i--;
			variable_address++;
		}
	}
}
END_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(composite)

static START_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(composite)
{
	Cmiss_variable_id *variable_address;
	int i;
	struct Cmiss_variable_composite_type_specific_data *data;

	data=(struct Cmiss_variable_composite_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(0<data->number_of_variables)&&(data->variables),return_code,
		0)
	{
		i=data->number_of_variables;
		variable_address=data->variables;
		while ((i>0)&&(return_code=Cmiss_variable_not_in_use(*variable_address)))
		{
			i--;
			variable_address++;
		}
	}
}
END_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(composite)

static START_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(composite)
{
	Cmiss_variable_id *variable_address_1,*variable_address_2;
	int i,j;
	struct Cmiss_variable_composite_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_variable_composite_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_1);
	data_2=(struct Cmiss_variable_composite_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&(0<data_1->number_of_variables)&&(data_1->variables)&&
		data_2&&(0<data_2->number_of_variables)&&(data_2->variables),return_code,0)
	{
		variable_address_1=data_1->variables;
		i=data_1->number_of_variables;
		while ((i>0)&&!return_code)
		{
			variable_address_2=data_2->variables;
			j=data_2->number_of_variables;
			while ((j>0)&&!return_code)
			{
				return_code=Cmiss_variable_overlap(*variable_address_1,
					*variable_address_2);
				j--;
				variable_address_2++;
			}
			i--;
			variable_address_1++;
		}
	}
}
END_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(composite)

static START_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(composite)
{
	Cmiss_variable_id *variable_address_1,*variable_address_2;
	int i;
	struct Cmiss_variable_composite_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_variable_composite_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_1);
	data_2=(struct Cmiss_variable_composite_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&(0<data_1->number_of_variables)&&(data_1->variables)&&
		data_2&&(0<data_2->number_of_variables)&&(data_2->variables),return_code,0)
	{
		if (data_1->number_of_variables==data_2->number_of_variables)
		{
			i=data_1->number_of_variables;
			variable_address_1=data_1->variables;
			variable_address_2=data_2->variables;
			while ((i>0)&&(return_code=Cmiss_variable_same_variable(
				*variable_address_1,*variable_address_2)))
			{
				i--;
				variable_address_1++;
				variable_address_2++;
			}
		}
	}
}
END_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(composite)

static
	START_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	composite)
{
	Cmiss_variable_id *variable_address;
	int i;
	struct Cmiss_variable_composite_type_specific_data *data;

	data=(struct Cmiss_variable_composite_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(0<data->number_of_variables)&&(data->variables),return_code,
		0)
	{
		i=data->number_of_variables;
		variable_address=data->variables;
		return_code=0;
		while ((i>0)&&!return_code)
		{
			return_code=Cmiss_variable_set_independent_variable_value(
				*variable_address,independent_variable,value);
			i--;
			variable_address++;
		}
		while (i>0)
		{
			Cmiss_variable_set_independent_variable_value(*variable_address,
				independent_variable,value);
			i--;
			variable_address++;
		}
	}
}
END_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	composite)

/*
Global functions
----------------
*/
int Cmiss_variable_composite_set_type(Cmiss_variable_id composite,
	int number_of_variables,Cmiss_variable_id *variables)
/*******************************************************************************
LAST MODIFIED : 14 July 2003

DESCRIPTION :
Sets <composite> to be a variable whose result is a vector containing the
results of the <variables>.

This function ACCESSes the <variables>.  After success, the <composite> is
responsible for DEACCESS/DEALLOCATEing <variables>.
==============================================================================*/
{
	Cmiss_value_id temp_value;
	int i,number_of_reals,return_code;
	struct Cmiss_variable_composite_type_specific_data *data;

	ENTER(Cmiss_variable_composite_set_type);
	return_code=0;
	/* check arguments */
	if (composite&&(0<number_of_variables)&&variables)
	{
		if (temp_value=CREATE(Cmiss_value)())
		{
			ACCESS(Cmiss_value)(temp_value);
			i=0;
			while ((i<number_of_variables)&&variables[i]&&
				Cmiss_variable_get_value_type(variables[i],temp_value)&&
				Cmiss_value_get_reals(temp_value,&number_of_reals,(FE_value **)NULL))
			{
				i++;
			}
			DEACCESS(Cmiss_value)(&temp_value);
			if (i==number_of_variables)
			{
				/* 1.  Make dynamic allocations for any new type-specific data */
				if (ALLOCATE(data,struct Cmiss_variable_composite_type_specific_data,1))
				{
					/* 2.  Clear current type-specific data */
					Cmiss_variable_clear_type(composite);
					/* 3.  Establish the new type */
					if (return_code=Cmiss_variable_set_type_specific_information(
						composite,Cmiss_variable_composite_type_string,(void *)data))
					{
						data->number_of_variables=number_of_variables;
						data->variables=variables;
						for (i=0;i<number_of_variables;i++)
						{
							ACCESS(Cmiss_variable)(variables[i]);
						}
						/* set all the methods */
						return_code=CMISS_VARIABLE_ESTABLISH_METHODS(composite,composite);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Cmiss_variable_composite_set_type.  "
							"Could not set type specific information");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Cmiss_variable_composite_set_type.  "
						"Could not ALLOCATE type specific data");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Cmiss_variable_composite_set_type.  "
					"Missing/invalid variable(s)");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_variable_composite_set_type.  "
				"Could not create temp_value");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_composite_set_type.  "
			"Invalid argument(s).  %p %d %p\n",composite,number_of_variables,
			variables);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_composite_set_type */

DECLARE_CMISS_VARIABLE_IS_TYPE_FUNCTION(composite)

int Cmiss_variable_composite_get_type(Cmiss_variable_id composite,
	int *number_of_variables_address,Cmiss_variable_id **variables_address)
/*******************************************************************************
LAST MODIFIED : 17 July 2003

DESCRIPTION :
Sets <composite> to be a variable whose result is a vector containing the
results of the <variables>.

This function ACCESSes the <variables>.  After success, the <composite> is
responsible for DEACCESS/DEALLOCATEing <variables>.
==============================================================================*/
{
	int return_code;
	struct Cmiss_variable_composite_type_specific_data *data;

	ENTER(Cmiss_variable_composite_get_type);
	return_code=0;
	/* check arguments */
	if (composite&&CMISS_VARIABLE_IS_TYPE(composite)(composite)&&
		(number_of_variables_address||variables_address))
	{
		data=(struct Cmiss_variable_composite_type_specific_data *)
			Cmiss_variable_get_type_specific_data(composite);
		ASSERT_IF(data,return_code,0)
		{
			if (number_of_variables_address)
			{
				*number_of_variables_address=data->number_of_variables;
			}
			if (variables_address)
			{
				*variables_address=data->variables;
			}
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_composite_get_type.  "
			"Invalid argument(s).  %p %p %p",composite,number_of_variables_address,
			variables_address);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_composite_get_type */
