/*******************************************************************************
FILE : computed_variable_derivative.c

LAST MODIFIED : 20 July 2003

DESCRIPTION :
Implements the derivative computed variable.
==============================================================================*/
#include "computed_variable/computed_value_derivative_matrix.h"
#include "computed_variable/computed_value_fe_value.h"
#include "computed_variable/computed_variable_derivative.h"
#include "computed_variable/computed_variable_private.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Module constants
----------------
*/

/*
Module methods
--------------
*/
static char Cmiss_variable_derivative_type_string[]="Derivative";

struct Cmiss_variable_derivative_type_specific_data
/*******************************************************************************
LAST MODIFIED : 21 March 2003

DESCRIPTION :
==============================================================================*/
{
	/* differentiate */
	Cmiss_variable_id variable;
	/* with respect to */
	int order;
	Cmiss_variable_id *independent_variables;
}; /* struct Cmiss_variable_derivative_type_specific_data */

static START_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(derivative)
{
	int i;

	DEACCESS(Cmiss_variable)(&(data->variable));
	if (data->independent_variables)
	{
		for (i=0;i<data->order;i++)
		{
			DEACCESS(Cmiss_variable)(&((data->independent_variables)[i]));
		}
		DEALLOCATE(data->independent_variables);
	}
	return_code=1;
}
END_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(derivative)
{
	int i;

	if ((0<source->order)&&ALLOCATE(destination->independent_variables,
		Cmiss_variable_id ,source->order))
	{
		destination->variable=ACCESS(Cmiss_variable)(source->variable);
		destination->order=source->order;
		for (i=0;i<source->order;i++)
		{
			(destination->independent_variables)[i]=
				ACCESS(Cmiss_variable)((source->independent_variables)[i]);
		}
	}
	else
	{
		DEALLOCATE(destination);
	}
}
END_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	derivative)
{
	int i;
	Cmiss_variable_id *augmented_independent_variables;
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->variable)&&(0<data->order)&&
		(data->independent_variables),return_code,0)
	{
		if (ALLOCATE(augmented_independent_variables,Cmiss_variable_id ,
			order+(data->order)))
		{
			/*???DB.  Should be ACCESSing? */
			for (i=0;i<order;i++)
			{
				augmented_independent_variables[i]=independent_variables[i];
			}
			for (i=0;i<data->order;i++)
			{
				augmented_independent_variables[order+i]=
					(data->independent_variables)[i];
			}
			return_code=Cmiss_variable_evaluate_derivative(data->variable,
				order+(data->order),augmented_independent_variables,
				(struct LIST(Cmiss_variable_value) *)NULL,value);
			DEALLOCATE(augmented_independent_variables);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_derivative_evaluate_derivative_type_specific.  "
				"Could not allocate augmented_independent_variables");
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(derivative)
{
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->variable)&&(0<data->order)&&
		(data->independent_variables),return_code,0)
	{
		return_code=Cmiss_variable_evaluate_derivative(data->variable,data->order,
			data->independent_variables,(struct LIST(Cmiss_variable_value) *)NULL,
			value);
	}
}
END_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(derivative)
{
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->variable),return_code,0)
	{
		return_code=Cmiss_variable_get_independent_variable_value(
			data->variable,independent_variable,value);
	}
}
END_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(derivative)
{
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->variable),return_code,0)
	{
		return_code=Cmiss_variable_get_set_independent_variable_value(
			data->variable,independent_variable,value);
	}
}
END_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(derivative)
{
	int i,dependent_order;
	Cmiss_value_id temp_type_1;
	Cmiss_variable_id *dependent_independent_variables,dependent_variable,
		*independent_variables;
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->variable)&&(0<data->order)&&
		(data->independent_variables),return_code,0)
	{
		if (temp_type_1=CREATE(Cmiss_value)())
		{
			dependent_independent_variables=(Cmiss_variable_id *)NULL;
			dependent_variable=(Cmiss_variable_id)NULL;
			dependent_order=0;
			if (return_code=Cmiss_variable_get_value_type(data->variable,
				temp_type_1))
			{
				/*???DB.  Not general, but hopefully OK for a while */
				if (CMISS_VALUE_IS_TYPE(FE_value)(temp_type_1))
				{
					dependent_variable=data->variable;
				}
				else if (CMISS_VALUE_IS_TYPE(FE_value_vector)(temp_type_1))
				{
					dependent_variable=data->variable;
				}
				else if (CMISS_VALUE_IS_TYPE(derivative_matrix)(temp_type_1))
				{
					Cmiss_value_derivative_matrix_get_type(temp_type_1,
						&dependent_variable,&dependent_order,
						&dependent_independent_variables,(Cmiss_value_id **)NULL);
				}
				if (dependent_variable)
				{
					i=0;
					do
					{
						if (return_code=Cmiss_variable_get_value_type(
							(data->independent_variables)[i],temp_type_1))
						{
							if (!CMISS_VALUE_IS_TYPE(FE_value)(temp_type_1)&&
								!CMISS_VALUE_IS_TYPE(FE_value_vector)(temp_type_1))
							{
								return_code=0;
							}
						}
						i++;
					} while (return_code&&(i<data->order));
					if (return_code)
					{
						if (ALLOCATE(independent_variables,Cmiss_variable_id,
							dependent_order+(data->order)))
						{
							for (i=0;i<dependent_order;i++)
							{
								independent_variables[i]=dependent_independent_variables[i];
							}
							for (i=0;i<data->order;i++)
							{
								independent_variables[dependent_order+i]=
									(data->independent_variables)[i];
							}
							if (return_code=Cmiss_value_derivative_matrix_set_type(type,
								dependent_variable,dependent_order+(data->order),
								independent_variables,(Cmiss_value_id *)NULL))
							{
								/* independent_variables is now managed by type */
								independent_variables=(Cmiss_variable_id *)NULL;
							}
							if (independent_variables)
							{
								DEALLOCATE(independent_variables);
							}
						}
					}
					else
					{
						return_code=0;
					}
				}
				else
				{
					return_code=0;
				}
			}
			DESTROY(Cmiss_value)(&temp_type_1);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_derivative_get_value_type_type_specific.  "
				"Could not create <temp_type_1>");
		}
	}
}
END_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(derivative)
{
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->variable),return_code,0)
	{
		return_code=Cmiss_variable_is_defined(data->variable,
			(struct LIST(Cmiss_variable_value) *)NULL);
	}
}
END_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(derivative)
{
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->variable),return_code,0)
	{
		return_code=Cmiss_variable_is_independent_variable_of(
			data->variable,independent_variable);
	}
}
END_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(derivative)
{
	int i;
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->variable)&&(0<data->order)&&
		(data->independent_variables),return_code,0)
	{
		return_code=Cmiss_variable_not_in_use(data->variable);
		i=0;
		while (return_code&&(i<data->order))
		{
			return_code=Cmiss_variable_not_in_use(
				(data->independent_variables)[i]);
			i++;
		}
	}
}
END_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(derivative)
{
	int i,j;
	struct Cmiss_variable_derivative_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_1);
	data_2=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&(data_1->variable)&&(0<data_1->order)&&
		(data_1->independent_variables)&&data_2&&(data_2->variable)&&
		(0<data_2->order)&&(data_2->independent_variables),return_code,0)
	{
		if (return_code=Cmiss_variable_overlap(data_1->variable,
			data_2->variable))
		{
			i=0;
			do
			{
				j=0;
				do
				{
					return_code=Cmiss_variable_overlap(
						(data_1->independent_variables)[i],
						(data_2->independent_variables)[j]);
					j++;
				} while (!return_code&&(j<data_2->order));
				i++;
			} while (!return_code&&(i<data_1->order));
		}
	}
}
END_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(derivative)
{
	int i,j;
	struct Cmiss_variable_derivative_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_1);
	data_2=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&(data_1->variable)&&(0<data_1->order)&&
		(data_1->independent_variables)&&data_2&&(data_2->variable)&&
		(0<data_2->order)&&(data_2->independent_variables),return_code,0)
	{
		if (data_1->order==data_2->order)
		{
			if (return_code=Cmiss_variable_same_variable(data_1->variable,
				data_2->variable))
			{
				i=0;
				do
				{
					j=0;
					do
					{
						return_code=Cmiss_variable_same_variable(
							(data_1->independent_variables)[i],
							(data_2->independent_variables)[j]);
						j++;
					} while (!return_code&&(j<data_2->order));
					i++;
				} while (return_code&&(i<data_1->order));
			}
		}
	}
}
END_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(derivative)
{
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->variable),return_code,0)
	{
		return_code=Cmiss_variable_set_independent_variable_value(data->variable,
			independent_variable,value);
	}
}
END_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(derivative)

/*
Global functions
----------------
*/
int Cmiss_variable_derivative_set_type(
	Cmiss_variable_id derivative,
	Cmiss_variable_id dependent_variable,int order,
	Cmiss_variable_id *independent_variables)
/*******************************************************************************
LAST MODIFIED : 10 July 2003

DESCRIPTION :
Sets <derivative> to be the derivative of the <dependent_variable> with respect
to the <independent_variables>.  This function ACCESSes the <dependent_variable>
and <independent_variables>.  After success, the <derivative> is responsible for
DEACCESS/DEALLOCATEing <dependent_variable> and <independent_variables>.
==============================================================================*/
{
	int i,return_code;
	struct Cmiss_variable_derivative_type_specific_data *data;

	ENTER(Cmiss_variable_derivative_set_type);
	return_code=0;
	/* check arguments */
	if (derivative&&dependent_variable&&(0<order)&&independent_variables)
	{
		i=0;
		while ((i<order)&&independent_variables[i])
		{
			i++;
		}
		if (i==order)
		{
			/* 1.  Make dynamic allocations for any new type-specific data */
			if (ALLOCATE(data,struct Cmiss_variable_derivative_type_specific_data,
				1))
			{
				/* 2.  Clear current type-specific data */
				Cmiss_variable_clear_type(derivative);
				/* 3.  Establish the new type */
				if (return_code=Cmiss_variable_set_type_specific_information(derivative,
					Cmiss_variable_derivative_type_string,(void *)data))
				{
					data->order=order;
					data->variable=ACCESS(Cmiss_variable)(dependent_variable);
					data->independent_variables=independent_variables;
					for (i=0;i<order;i++)
					{
						ACCESS(Cmiss_variable)(independent_variables[i]);
					}
					/* set all the methods */
					return_code=CMISS_VARIABLE_ESTABLISH_METHODS(derivative,derivative);
				}
				else
				{
					display_message(ERROR_MESSAGE,"Cmiss_variable_derivative_set_type.  "
						"Could not set type specific information");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Cmiss_variable_derivative_set_type.  "
					"Could not ALLOCATE type specific data");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_derivative_set_type.  "
				"Missing independent variable(s)");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_derivative_set_type.  "
			"Invalid argument(s).  %p %p %d %p\n",derivative,dependent_variable,
			order,independent_variables);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_derivative_set_type */

int Cmiss_variable_divergence_set_type(Cmiss_variable_id divergence,
	Cmiss_variable_id dependent_variable,Cmiss_variable_id independent_variable);
/*******************************************************************************
LAST MODIFIED : 7 March 2003

DESCRIPTION :
Sets <divergence> to be the divergence of the <dependent_variable> with respect
to the <independent_variable>.
==============================================================================*/
/*???DB.  To be done */

int Cmiss_variable_inverse_set_type(Cmiss_variable_id inverse_variable,
	Cmiss_variable_id variable,struct LIST(Cmiss_variable) *dependent_variables);
/*******************************************************************************
LAST MODIFIED : 24 January 2003

DESCRIPTION :
Sets <inverse_variable> to be the inverse of the <variable>.  Its independent
variables are the dependent variables of the <variable> and its
<dependent_variables> are independent variables of the <variable>.
==============================================================================*/
/*???DB.  To be done */
