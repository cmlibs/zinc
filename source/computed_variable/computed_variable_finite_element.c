/*******************************************************************************
FILE : computed_variable_finite_element.c

LAST MODIFIED : 24 March 2003

DESCRIPTION :
Implements computed variables which interface to finite element fields:
- cmiss_number.  Returns the cmiss number of a node or an element
- embedded.  Used for node fields that give element/xi - data at material points
???DB.  Why is Computed_variable_set_type_embedded static?
???DB.  Extend to element/xi
- finite_element.  A wrapper for a FE_field
- node_value
- xi_coordinate.  Returns the xi coordinates as a vector of FE_values -
	basically the identity
???DB.  Currently only implemented for element/xi ie. doesn't return the
	xi_coordinates of the node - depends on element

NOTES :

???DB.  Add an integral variable type?

???DB.  Is a computed_variable_finite_element_utilities module needed?
???DB.  Consistency for is_type functions.  Currently some are iterator/
	conditional and some are not
==============================================================================*/
#include "computed_variable/computed_value_finite_element.h"
#include "computed_variable/computed_variable_finite_element.h"
#include "computed_variable/computed_variable_private.h"
#include "finite_element/finite_element.h"
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
static char computed_variable_element_value_type_string[]="element value";

/*???DB.  To be done */

static char computed_variable_element_xi_type_string[]="element/xi";

struct Computed_variable_element_xi_type_specific_data
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
???DB.  Does this need the option of having a variable? ie.
	<finite_element_variable> should be a particular finite element computed
	variable or NULL if the element/xi is for all finite element computed fields.
==============================================================================*/
{
	struct Computed_value *element_xi;
}; /* struct Computed_variable_element_xi_type_specific_data */

static START_COMPUTED_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(element_xi)
{
	DESTROY(Computed_value)(&(data->element_xi));
	return_code=1;
}
END_COMPUTED_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(element_xi)

static START_COMPUTED_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(
	element_xi)
{
	if (destination->element_xi=CREATE(Computed_value)())
	{
		if (!Computed_value_copy(destination->element_xi,source->element_xi))
		{
			DESTROY(Computed_value)(&(destination->element_xi));
			DEALLOCATE(destination);
		}
	}
	else
	{
		DEALLOCATE(destination);
	}
}
END_COMPUTED_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(element_xi)

static START_COMPUTED_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	element_xi)
{
	FE_value *matrix_value,*matrix_values;
	int i,j,number_of_columns,number_of_columns_save,number_of_rows,
		number_of_values;
	struct Computed_value *value_type;
	struct Computed_variable *independent_variable,**independent_variable_address;
	struct Computed_variable_element_xi_type_specific_data *data;
	struct FE_element *element;

	data=(struct Computed_variable_element_xi_type_specific_data *)
		Computed_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&Computed_value_is_type_element_xi(data->element_xi),
		return_code,0)
	{
		/* get number of rows in derivative matrix */
		if (Computed_value_get_type_element_xi(data->element_xi,&element,
			(FE_value **)NULL))
		{
			if (0<(number_of_rows=get_FE_element_dimension(element)))
			{
				return_code=1;
			}
		}
		if (return_code)
		{
			if (value_type=CREATE(Computed_value)())
			{
				/* check independent variables and count number of columns in derivative
					matrix */
				i=order;
				number_of_columns=0;
				independent_variable_address=independent_variables;
				matrix_values=(FE_value *)NULL;
				while (return_code&&(i>0))
				{
					independent_variable= *independent_variable_address;
					if (return_code=Computed_variable_get_value_type(independent_variable,
						value_type))
					{
						if (Computed_value_is_type_FE_value(value_type))
						{
							number_of_values=1;
						}
						else
						{
							if (Computed_value_is_type_FE_value_vector(value_type))
							{
								return_code=Computed_value_get_type_FE_value_vector(value_type,
									&number_of_values,(FE_value **)NULL);
							}
							else
							{
								if (Computed_value_is_type_element_xi(value_type))
								{
									if (Computed_value_get_type_element_xi(value_type,
										&element,(FE_value **)NULL))
									{
										number_of_values=get_FE_element_dimension(element);
									}
								}
								else
								{
									return_code=0;
								}
							}
						}
						if (return_code)
						{
							if (0<number_of_values)
							{
								number_of_columns_save=number_of_columns;
								number_of_columns=number_of_columns*(number_of_values+1)+
									number_of_values;
								if (REALLOCATE(matrix_value,matrix_values,FE_value,
									number_of_rows*number_of_columns))
								{
									matrix_values=matrix_value;
									/*???DB.  Assuming that only independent variable is itself */
									if (Computed_variable_same_variable(variable,
										independent_variable)&&(number_of_rows==number_of_values))
									{
										matrix_value=matrix_values;
										for (i=0;i<number_of_rows;i++)
										{
											matrix_value += number_of_columns_save;
											for (j=0;j<i;j++)
											{
												*matrix_value=0;
												matrix_value++;
											}
											*matrix_value=1;
											j=number_of_columns-number_of_columns_save-i-1;
											while (j>0)
											{
												matrix_value++;
												*matrix_value=0;
											}
										}
									}
									else
									{
										matrix_value=matrix_values;
										for (i=number_of_rows;i>0;i--)
										{
											matrix_value += number_of_columns_save;
											for (j=number_of_columns-number_of_columns_save;j>0;j--)
											{
												*matrix_value=0;
												matrix_value++;
											}
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Computed_variable_element_xi_evaluate_derivative_type_specific.  "
										"Could not reallocate <matrix_values>");
									DEALLOCATE(matrix_values);
									return_code=0;
								}
							}
							else
							{
								return_code=0;
							}
						}
					}
					i--;
					independent_variable_address++;
				}
				if (return_code)
				{
					if (Computed_value_set_type_FE_value_matrix(value_type,
						number_of_rows,number_of_columns,matrix_values))
					{
						if (ALLOCATE(independent_variable_address,
							struct Computed_variable *,order))
						{
							for (i=0;i<order;i++)
							{
								independent_variable_address[i]=independent_variables[i];
							}
							if (!Computed_value_set_type_derivative_matrix(value,
								variable,order,independent_variable_address,value_type))
							{
								DEALLOCATE(independent_variable_address);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Computed_variable_element_xi_evaluate_derivative_type_specific.  "
								"Could not allocate <independent_variable_address>");
							return_code=0;
						}
					}
					else
					{
						DEALLOCATE(matrix_values);
						return_code=0;
					}
				}
				if (!return_code)
				{
					DESTROY(Computed_value)(&value_type);
				}
			}
		}
	}
}
END_COMPUTED_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(element_xi)

static START_COMPUTED_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(element_xi)
{
	struct Computed_value *value;
	struct Computed_variable_element_xi_type_specific_data *data;

	data=(struct Computed_variable_element_xi_type_specific_data *)
		Computed_variable_get_type_specific_data(variable);
	value=Computed_variable_value_get_value(variable_value);
	ASSERT_IF(data&&Computed_value_is_type_element_xi(data->element_xi)&&value,
		return_code,0)
	{
		if (Computed_value_copy(value,data->element_xi))
		{
			return_code=0;
		}
	}
}
END_COMPUTED_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(element_xi)

static
	START_COMPUTED_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	element_xi)
{
	struct Computed_variable_element_xi_type_specific_data *data;

	data=(struct Computed_variable_element_xi_type_specific_data *)
		Computed_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&Computed_value_is_type_element_xi(data->element_xi),
		return_code,0)
	{
		if (Computed_variable_same_variable(dependent_variable,
			independent_variable))
		{
			return_code=Computed_value_copy(value,data->element_xi);
		}
	}
}
END_COMPUTED_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	element_xi)

static
	START_COMPUTED_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	element_xi)
{
	struct Computed_value *temp_value;
	struct Computed_variable_element_xi_type_specific_data *data;

	data=(struct Computed_variable_element_xi_type_specific_data *)
		Computed_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&Computed_value_is_type_element_xi(data->element_xi),
		return_code,0)
	{
		if (Computed_variable_same_variable(dependent_variable,
			independent_variable))
		{
			if (temp_value=CREATE(Computed_value)())
			{
				if (Computed_value_copy(temp_value,data->element_xi))
				{
					if (Computed_value_copy(data->element_xi,value))
					{
						return_code=Computed_value_copy(value,temp_value);
					}
				}
				DESTROY(Computed_value)(&temp_value);
			}
		}
	}
}
END_COMPUTED_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	element_xi)

static START_COMPUTED_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(
	element_xi)
{
	/*???DB.  What about number of xi? */
	return_code=Computed_value_set_type_element_xi(type,
		(struct FE_element *)NULL,(FE_value *)NULL);
}
END_COMPUTED_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(element_xi)

static START_COMPUTED_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(element_xi)
{
	FE_value *xi;
	struct Computed_variable_element_xi_type_specific_data *data;
	struct FE_element *element;

	data=(struct Computed_variable_element_xi_type_specific_data *)
		Computed_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&Computed_value_is_type_element_xi(data->element_xi),
		return_code,0)
	{
		if (Computed_value_get_type_element_xi(data->element_xi,&element,&xi)&&
			element&&xi)
		{
			return_code=1;
		}
	}
}
END_COMPUTED_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(element_xi)

static
	START_COMPUTED_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	element_xi)
{
	return_code=Computed_variable_same_variable(dependent_variable,
		independent_variable);
}
END_COMPUTED_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	element_xi)

static START_COMPUTED_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(element_xi)
{
	return_code=1;
}
END_COMPUTED_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(element_xi)

static START_COMPUTED_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(element_xi)
{
	return_code=Computed_variable_same_variable(variable_1,variable_2);
}
END_COMPUTED_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(element_xi)

static START_COMPUTED_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(
	element_xi)
{
	struct Computed_variable_element_xi_type_specific_data *data_1,*data_2;
	struct FE_element *element_1,*element_2;

	data_1=(struct Computed_variable_element_xi_type_specific_data *)
		Computed_variable_get_type_specific_data(variable_1);
	data_2=(struct Computed_variable_element_xi_type_specific_data *)
		Computed_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&Computed_value_is_type_element_xi(data_1->element_xi)&&
		data_2&&Computed_value_is_type_element_xi(data_2->element_xi),return_code,0)
	{
		if (Computed_value_get_type_element_xi(data_1->element_xi,&element_1,
			(FE_value **)NULL)&&Computed_value_get_type_element_xi(data_2->element_xi,
			&element_2,(FE_value **)NULL)&&(get_FE_element_dimension(element_1)==
			get_FE_element_dimension(element_2)))
		{
			return_code=1;
		}
	}
}
END_COMPUTED_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(element_xi)

static
	START_COMPUTED_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	element_xi)
{
	struct Computed_variable_element_xi_type_specific_data *data;

	data=(struct Computed_variable_element_xi_type_specific_data *)
		Computed_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&Computed_value_is_type_element_xi(data->element_xi),
		return_code,0)
	{
		if (Computed_variable_same_variable(dependent_variable,
			independent_variable))
		{
			/*???DB.  Copy into <independent_variable> as well? */
			return_code=Computed_value_copy(data->element_xi,value);
		}
	}
}
END_COMPUTED_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	element_xi)

static char computed_variable_FE_time_type_string[]="FE time";

/*???DB.  To be done */

static char computed_variable_finite_element_type_string[]="finite element";

struct Computed_variable_finite_element_type_specific_data
/*******************************************************************************
LAST MODIFIED : 16 February 2003

DESCRIPTION :
???DB.  Assuming that the <element>, <field> and <node> know their FE_region
	(can get managers)
==============================================================================*/
{
	FE_value time,*xi;
	int component_number;
	struct FE_element *element;
	struct FE_field *field;
	struct FE_node *node;
}; /* struct Computed_variable_finite_element_type_specific_data */

static START_COMPUTED_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(finite_element)
{
	DEACCESS(FE_element)(&(data->element));
	DEACCESS(FE_field)(&(data->field));
	DEACCESS(FE_node)(&(data->node));
	DEALLOCATE(data->xi);
	return_code=1;
}
END_COMPUTED_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(finite_element)

static START_COMPUTED_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(
	finite_element)
{
	int dimension,i;

	if (source->element)
	{
		if ((0<(dimension=get_FE_element_dimension(source->element)))&&
			(source->xi)&&ALLOCATE(destination->xi,FE_value,dimension))
		{
			for (i=0;i<dimension;i++)
			{
				(destination->xi)[i]=(source->xi)[i];
			}
		}
		else
		{
			DEALLOCATE(destination);
		}
	}
	else
	{
		destination->xi=(FE_value *)NULL;
	}
	if (destination)
	{
		destination->time=source->time;
		if (source->element)
		{
			destination->element=ACCESS(FE_element)(source->element);
		}
		else
		{
			destination->element=(struct FE_element *)NULL;
		}
		destination->component_number=source->component_number;
		if (source->field)
		{
			destination->field=ACCESS(FE_field)(source->field);
		}
		else
		{
			destination->field=(struct FE_field *)NULL;
		}
		if (source->node)
		{
			destination->node=ACCESS(FE_node)(source->node);
		}
		else
		{
			destination->node=(struct FE_node *)NULL;
		}
	}
}
END_COMPUTED_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(finite_element)

static START_COMPUTED_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	finite_element)
{
	FE_value *matrix_value,*matrix_values;
	int component_number,i,j,number_of_columns,number_of_columns_save,
		number_of_rows,number_of_values;
	struct Computed_value *value_type;
	struct Computed_variable *independent_variable,**independent_variable_address;
	struct Computed_variable_finite_element_type_specific_data *data;
	struct FE_element *element;

	data=(struct Computed_variable_finite_element_type_specific_data *)
		Computed_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->field),return_code,0)
	{
		component_number=data->component_number;
		if (value_type=CREATE(Computed_value)())
		{
			/* get number of rows in derivative matrix */
			if (Computed_variable_get_value_type(variable,value_type))
			{
				if (Computed_value_is_type_FE_value(value_type))
				{
					number_of_rows=1;
					return_code=1;
				}
				else
				{
					if (Computed_value_is_type_FE_value_vector(value_type))
					{
						return_code=Computed_value_get_type_FE_value_vector(value_type,
							&number_of_rows,(FE_value **)NULL);
					}
				}
				if (return_code)
				{
					if (0<number_of_rows)
					{
						if ((0<=component_number)&&(component_number<number_of_rows))
						{
							number_of_rows=1;
							return_code=1;
						}
						else
						{
							if (-1==component_number)
							{
								return_code=1;
							}
							else
							{
								return_code=0;
							}
						}
					}
					else
					{
						return_code=0;
					}
				}
			}
			/* check independent variables and count number of columns in derivative
				matrix */
			i=order;
			number_of_columns=0;
			independent_variable_address=independent_variables;
			matrix_values=(FE_value *)NULL;
			while (return_code&&(i>0))
			{
				independent_variable= *independent_variable_address;
				/*???DB.  Set up variables for rows and columns */
					/*???DB.  To be done */
				if (return_code=Computed_variable_get_value_type(independent_variable,
					value_type))
				{
					if (Computed_value_is_type_FE_value(value_type))
					{
						number_of_values=1;
					}
					else if (Computed_value_is_type_FE_value_vector(value_type))
					{
						return_code=Computed_value_get_type_FE_value_vector(value_type,
							&number_of_values,(FE_value **)NULL);
					}
					else if (Computed_value_is_type_element_xi(value_type))
					{
						if (Computed_value_get_type_element_xi(value_type,
							&element,(FE_value **)NULL))
						{
							number_of_values=get_FE_element_dimension(element);
						}
					}
					/*???DB.  Need nodal values */
					else
					{
						return_code=0;
					}
					if (return_code)
					{
						if (0<number_of_values)
						{
							number_of_columns_save=number_of_columns;
							number_of_columns=number_of_columns*(number_of_values+1)+
								number_of_values;
							if (REALLOCATE(matrix_value,matrix_values,FE_value,
								number_of_rows*number_of_columns))
							{
								matrix_values=matrix_value;
								if (Computed_variable_is_type_element_xi(independent_variable))
								{
									/* differentiating with respect to element xi */
									/*???DB.  Use
										CREATE(FE_element_field_values)
										calculate_FE_element_field_values
										FE_element_field_values_get_component_values
											???DB.  After blending (ie for monomials)?  Yes
											???DB.  Want blending matrix as well?
										FE_element_field_values_get_monomial_component_info
										DESTROY(FE_element_field_values)
										which are in the new region finite_element module */
/*???DB.  Where I'm up to */
									/*???DB.  To be done */
								}
								else if (Computed_variable_is_type_nodal_value(
									independent_variable))
								{
									/* differentiating with respect to nodal value */
									/*???DB.  To be done */
								}
								else
								{
									/* zero derivative */
									matrix_value=matrix_values;
									for (i=number_of_rows;i>0;i--)
									{
										matrix_value += number_of_columns_save;
										for (j=number_of_columns-number_of_columns_save;j>0;j--)
										{
											*matrix_value=0;
											matrix_value++;
										}
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Computed_variable_derivative_evaluate_derivative_type_specific.  "
									"Could not reallocate <matrix_values>");
								DEALLOCATE(matrix_values);
								return_code=0;
							}
						}
						else
						{
							return_code=0;
						}
					}
				}
				i--;
				independent_variable_address++;
			}
			if (return_code)
			{
				if (Computed_value_set_type_FE_value_matrix(value_type,
					number_of_rows,number_of_columns,matrix_values))
				{
					if (ALLOCATE(independent_variable_address,
						struct Computed_variable *,order))
					{
						for (i=0;i<order;i++)
						{
							independent_variable_address[i]=independent_variables[i];
						}
						if (!Computed_value_set_type_derivative_matrix(value,
							variable,order,independent_variable_address,value_type))
						{
							DEALLOCATE(independent_variable_address);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_variable_derivative_evaluate_derivative_type_specific.  "
							"Could not allocate <independent_variable_address>");
						return_code=0;
					}
				}
				else
				{
					DEALLOCATE(matrix_values);
					return_code=0;
				}
			}
			if (!return_code)
			{
				DESTROY(Computed_value)(&value_type);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_variable_finite_element_evaluate_derivative_type_specific.  "
				"Could not create <value_type>");
		}
	}
}
END_COMPUTED_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(finite_element)

static START_COMPUTED_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(finite_element)
{
	FE_value *fe_value_vector;
	int number_of_fe_values;
	struct Computed_value *value;
	struct Computed_variable_finite_element_type_specific_data *data;

	data=(struct Computed_variable_finite_element_type_specific_data *)
		Computed_variable_get_type_specific_data(variable);
	value=Computed_variable_value_get_value(variable_value);
	ASSERT_IF(data&&(data->field)&&value,return_code,0)
	{
		number_of_fe_values=get_FE_field_number_of_components(data->field);
		if ((0<=data->component_number)&&
			(data->component_number<number_of_fe_values))
		{
			number_of_fe_values=1;
		}
		if ((0<number_of_fe_values)&&ALLOCATE(fe_value_vector,FE_value,
			number_of_fe_values))
		{
			if (calculate_FE_field(data->field,data->component_number,data->node,
				data->element,data->xi,data->time,fe_value_vector))
			{
				if (1==number_of_fe_values)
				{
					return_code=Computed_value_set_type_FE_value(value,*fe_value_vector);
				}
				else
				{
					return_code=Computed_value_set_type_FE_value_vector(value,
						number_of_fe_values,fe_value_vector);
				}
				if (!return_code)
				{
					DEALLOCATE(fe_value_vector);
				}
			}
			else
			{
				DEALLOCATE(fe_value_vector);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_variable_finite_element_evaluate_type_specific.  "
				"Could not ALLOCATE <fe_value_vector>.  %d",number_of_fe_values);
		}
	}
}
END_COMPUTED_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(finite_element)

static
	START_COMPUTED_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	finite_element)
{
	/*???DB.  To be done */
#if defined (OLD_CODE)
	struct Computed_variable_finite_element_type_specific_data *data;

	data=(struct Computed_variable_finite_element_type_specific_data *)
		Computed_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->variable),return_code,0)
	{
		return_code=Computed_variable_get_independent_variable_value(
			data->variable,independent_variable,value);
	}
#endif /* defined (OLD_CODE) */
}
END_COMPUTED_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	finite_element)

static
	START_COMPUTED_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	finite_element)
{
	/*???DB.  To be done */
#if defined (OLD_CODE)
	struct Computed_variable_finite_element_type_specific_data *data;

	data=(struct Computed_variable_finite_element_type_specific_data *)
		Computed_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->variable),return_code,0)
	{
		return_code=Computed_variable_set_independent_variable_value(
			data->variable,independent_variable,value);
	}
#endif /* defined (OLD_CODE) */
}
END_COMPUTED_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(finite_element)

static START_COMPUTED_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(
	finite_element)
{
	int number_of_fe_values;
	struct Computed_variable_finite_element_type_specific_data *data;

	data=(struct Computed_variable_finite_element_type_specific_data *)
		Computed_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->field),return_code,0)
	{
		number_of_fe_values=get_FE_field_number_of_components(data->field);
		if ((0<=data->component_number)&&
			(data->component_number<number_of_fe_values))
		{
			number_of_fe_values=1;
		}
		if (0<number_of_fe_values)
		{
			if (1==number_of_fe_values)
			{
				return_code=Computed_value_set_type_FE_value(type,(FE_value)0);
			}
			else
			{
				return_code=Computed_value_set_type_FE_value_vector(type,
					number_of_fe_values,(FE_value *)NULL);
			}
		}
	}
}
END_COMPUTED_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(finite_element)

static START_COMPUTED_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(finite_element)
{
	struct Computed_variable_finite_element_type_specific_data *data;

	data=(struct Computed_variable_finite_element_type_specific_data *)
		Computed_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->field),return_code,0)
	{
		if ((data->node)||((data->element)&&(data->xi)))
		{
			return_code=1;
		}
	}
}
END_COMPUTED_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(finite_element)

static
	START_COMPUTED_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	finite_element)
{
	/*???DB.  To be done */
#if defined (OLD_CODE)
	struct Computed_variable_finite_element_type_specific_data *data;

	data=(struct Computed_variable_finite_element_type_specific_data *)
		Computed_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->variable),return_code,0)
	{
		return_code=Computed_variable_is_independent_variable_of(
			data->variable,independent_variable);
	}
#endif /* defined (OLD_CODE) */
}
END_COMPUTED_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	finite_element)

static START_COMPUTED_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(finite_element)
{
	/*???DB.  To be done */
		/*???DB.  Need regions */
#if defined (OLD_CODE)
	struct Computed_variable_finite_element_type_specific_data *data;

	data=(struct Computed_variable_finite_element_type_specific_data *)
		Computed_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->variable)&&(data->independent_variable),return_code,0)
	{
		if (return_code=Computed_variable_not_in_use(data->variable))
		{
			return_code=Computed_variable_not_in_use(data->independent_variable);
		}
	}
#endif /* defined (OLD_CODE) */
}
END_COMPUTED_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(finite_element)

static START_COMPUTED_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(finite_element)
{
	/*???DB.  To be done */
#if defined (OLD_CODE)
	struct Computed_variable_finite_element_type_specific_data *data_1,*data_2;

	data_1=(struct Computed_variable_finite_element_type_specific_data *)
		Computed_variable_get_type_specific_data(variable_1);
	data_2=(struct Computed_variable_finite_element_type_specific_data *)
		Computed_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&(data_1->variable)&&(data_1->independent_variable)&&
		data_2&&(data_2->variable)&&(data_2->independent_variable),return_code,0)
	{
		if (return_code=Computed_variable_overlap(data_1->variable,
			data_2->variable))
		{
			return_code=Computed_variable_overlap(data_1->independent_variable,
				data_2->independent_variable);
		}
	}
#endif /* defined (OLD_CODE) */
}
END_COMPUTED_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(finite_element)

static START_COMPUTED_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(
	finite_element)
{
	struct Computed_variable_finite_element_type_specific_data *data_1,*data_2;

	data_1=(struct Computed_variable_finite_element_type_specific_data *)
		Computed_variable_get_type_specific_data(variable_1);
	data_2=(struct Computed_variable_finite_element_type_specific_data *)
		Computed_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&(data_1->field)&&data_2&&(data_2->field),return_code,0)
	{
		if (data_1->field==data_2->field)
		{
			/*???DB.  What about source variables? */
			return_code=1;
		}
	}
}
END_COMPUTED_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(finite_element)

static
	START_COMPUTED_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	finite_element)
{
	/*???DB.  To be done */
#if defined (OLD_CODE)
	struct Computed_variable_finite_element_type_specific_data *data;

	data=(struct Computed_variable_finite_element_type_specific_data *)
		Computed_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->variable),return_code,0)
	{
		return_code=Computed_variable_set_independent_variable_value(data->variable,
			independent_variable,value);
	}
#endif /* defined (OLD_CODE) */
}
END_COMPUTED_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	finite_element)

static char computed_variable_nodal_value_type_string[]="nodal value";

/*???DB.  To be done */

static char computed_variable_scale_factor_type_string[]="scale factor";

/*???DB.  To be done */

/*
Global functions
----------------
*/
int Computed_variable_set_type_element_value(struct Computed_variable *variable,
	struct Computed_variable *fe_variable,struct FE_element *element,
	int *grid_point,int version);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Converts the <variable> into a element_value Computed_variable for the
specified <fe_variable> (a finite_element Computed_variable, all finite element
computed variables if NULL), <element> (all elements if NULL), <grid_point> (all
grid points if NULL), <version> (all versions if 0).
==============================================================================*/

DECLARE_COMPUTED_VARIABLE_IS_TYPE_FUNCTION(element_value)

int Computed_variable_get_type_element_value(
	struct Computed_variable *variable,
	struct Computed_variable **fe_variable_address,
	struct FE_element **element_address,int **grid_point_address,
	int *version_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type element_value, gets its <*fe_variable_address>,
<*element_address>, <*grid_point_address> and <*version_address>.

The calling program must not DEALLOCATE the returned <*grid_point_address>.
==============================================================================*/

int Computed_variable_set_type_element_xi(struct Computed_variable *variable)
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
???DB.  Is a <fe_variable> needed?
Converts the <variable> into a element_xi Computed_variable for the
specified <fe_variable> (a finite_element Computed_variable, all finite element
computed variables if NULL).
==============================================================================*/
{
	int return_code;
	struct Computed_variable_element_xi_type_specific_data *data;

	ENTER(Computed_variable_set_type_element_xi);
	return_code=0;
	/* check arguments */
	if (variable)
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,struct Computed_variable_element_xi_type_specific_data,1))
		{
			if (data->element_xi=CREATE(Computed_value)())
			{
				if (Computed_value_set_type_element_xi(data->element_xi,
					(struct FE_element *)NULL,(FE_value *)NULL))
				{
					/* 2.  Clear current type-specific data */
					Computed_variable_clear_type(variable);
					/* 3.  Establish the new type */
					Computed_variable_set_type_specific_information(variable,
						computed_variable_element_xi_type_string,(void *)data);
					/* set all the methods */
					return_code=COMPUTED_VARIABLE_ESTABLISH_METHODS(variable,
						element_xi);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_variable_set_type_element_xi.  "
						"Could not Computed_value_set_type_element_xi");
					DESTROY(Computed_value)(&(data->element_xi));
					DEALLOCATE(data);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_variable_set_type_element_xi.  "
					"Could not CREATE(Computed_value)");
				DEALLOCATE(data);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_variable_set_type_element_xi.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_variable_set_type_element_xi.  "
			"Invalid argument(s).  %p\n",variable);
	}
	LEAVE;

	return (return_code);
} /* Computed_variable_set_type_element_xi */

DECLARE_COMPUTED_VARIABLE_IS_TYPE_FUNCTION(element_xi)

#if defined (OLD_CODE)
int Computed_variable_get_type_element_xi(
	struct Computed_variable *variable,
	struct Computed_variable **fe_variable_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type element_value, gets its <*fe_variable_address>.
==============================================================================*/
#endif /* defined (OLD_CODE) */

int Computed_variable_set_type_FE_time(struct Computed_variable *variable,
	struct Computed_variable *fe_variable);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Converts the <variable> into a FE_time Computed_variable for the specified
<fe_variable> (a finite_element Computed_variable, all finite element
computed variables if NULL).
==============================================================================*/

DECLARE_COMPUTED_VARIABLE_IS_TYPE_FUNCTION(FE_time)

int Computed_variable_get_type_FE_time(struct Computed_variable *variable,
	struct Computed_variable **fe_variable_address,FE_value *fe_time_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type FE_time, gets its <*fe_variable_address> and
<*fe_time_address>.
==============================================================================*/

int Computed_variable_set_type_finite_element(
	struct Computed_variable *variable,struct FE_field *fe_field,
	int component_number)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
Converts the <variable> into a finite_element Computed_variable for the
specified <fe_field> and <component_number> (all components if -1).

Need pointer to <fe_field_manager> so can call MANAGED_OBJECT_NOT_IN_USE in
Computed_variable_finite_element_not_in_use.

???DB.  Assuming that the <fe_field> "knows" its FE_region (can get FE_field
	manager)
==============================================================================*/
{
	int return_code;
	struct Computed_variable_finite_element_type_specific_data *data;

	ENTER(Computed_variable_set_type_finite_element);
	return_code=0;
	/* check arguments */
	if (variable&&fe_field)
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,
			struct Computed_variable_finite_element_type_specific_data,1))
		{
			/* 2.  Clear current type-specific data */
			Computed_variable_clear_type(variable);
			/* 3.  Establish the new type */
			if (return_code=Computed_variable_set_type_specific_information(variable,
				computed_variable_finite_element_type_string,(void *)data))
			{
				data->time=(FE_value)0;
				data->xi=(FE_value *)NULL;
				data->element=(struct FE_element *)NULL;
				data->field=ACCESS(FE_field)(fe_field);
				data->component_number=component_number;
				data->node=(struct FE_node *)NULL;
				/* set all the methods */
				return_code=COMPUTED_VARIABLE_ESTABLISH_METHODS(variable,
					finite_element);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_variable_set_type_finite_element.  "
					"Could not set type specific information");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_variable_set_type_finite_element.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_variable_set_type_finite_element.  "
			"Invalid argument(s).  %p %p\n",variable,fe_field);
	}
	LEAVE;

	return (return_code);
} /* Computed_variable_set_type_finite_element */

DECLARE_COMPUTED_VARIABLE_IS_TYPE_FUNCTION(finite_element)

int Computed_variable_get_type_finite_element(
	struct Computed_variable *variable,struct FE_field **fe_field_address,
	int *component_number_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type finite_element, gets its <*fe_field_address> and
<*component_number_address>.
==============================================================================*/

int Computed_variable_set_type_nodal_value(struct Computed_variable *variable,
	struct Computed_variable *fe_variable,struct FE_node *node,
	enum FE_nodal_value_type value_type,int version);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Converts the <variable> into a nodal_value Computed_variable for the
specified <fe_variable> (a finite_element Computed_variable, all finite element
computed variables if NULL), <node> (all nodes if NULL), <value_type> (all types
if FE_NODAL_UNKNOWN), <version> (all versions if 0).
==============================================================================*/

DECLARE_COMPUTED_VARIABLE_IS_TYPE_FUNCTION(nodal_value)

int Computed_variable_get_type_nodal_value(struct Computed_variable *variable,
	struct Computed_variable **fe_variable_address,struct FE_node **node_address,
	enum FE_nodal_value_type *value_type_address,int *version_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type nodal_value, gets its <*fe_variable_address>,
<*node_address>, <*value_type_address> and <*version_address>.
==============================================================================*/

int Computed_variable_set_type_scale_factor(struct Computed_variable *variable,
	struct Computed_variable *fe_variable,struct FE_element *element,
	int local_node_number,enum FE_nodal_value_type value_type,int version);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Converts the <variable> into a scale_factor Computed_variable for the
specified <fe_variable> (a finite_element Computed_variable, all finite element
computed variables if NULL), <element> (all elements if NULL),
<local_node_number> (all local nodes if -1), <value_type> (all types if
FE_NODAL_UNKNOWN), <version> (all versions if 0).
==============================================================================*/

DECLARE_COMPUTED_VARIABLE_IS_TYPE_FUNCTION(scale_factor)

int Computed_variable_get_type_scale_factor(struct Computed_variable *variable,
	struct Computed_variable **fe_variable_address,
	struct FE_element **element_address,int *local_node_number_address,
	enum FE_nodal_value_type *value_type_address,int *version_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type scale_factor, gets its <*fe_variable_address>,
<*element_address>, <*local_node_number_address>, <*value_type_address> and
<*version_address>.
==============================================================================*/

/*???DB.  Where I'm up to */
#if defined (OLD_CODE)
int Computed_variable_is_read_only_with_fe_field(
	struct Computed_variable *field,void *fe_field_void);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is read only and a
wrapper for the finite element field <(struct FE_field *)fe_field_void>.

???DB.  Move to computed_variable_finite_element_utilities?
==============================================================================*/

int Computed_variable_has_coordinate_fe_field(struct Computed_variable *field,
	void *dummy);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is a finite_element
Computed_variable and the FE_field it wraps is of coordinate type.

???DB.  Move to computed_variable_finite_element_utilities?
==============================================================================*/

int Computed_variable_is_finite_element_and_scalar_integer(
	struct Computed_variable *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is a finite_element
Computed_variable that returns a single integer.

???DB.  Move to computed_variable_finite_element_utilities?
==============================================================================*/

int Computed_variable_is_grid_based_finite_element_and_scalar_integer(
	struct Computed_variable *field,void *element_void);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is a finite_element
Computed_variable that returns a single integer and for which the wrapped FE_field
is grid-based.

Used for choosing field suitable for identifying grid points.

???DB.  Move to computed_variable_finite_element_utilities?
==============================================================================*/

int remove_computed_variable_from_manager_given_FE_field(
	struct MANAGER(Computed_variable) *computed_variable_manager,
	struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Removes the finite_element Computed_variable that wraps <fe_field> from the
<computed_variable_manager> - this will mean that the wrapping Computed_variable is
destroyed.
==============================================================================*/

int destroy_computed_variable_given_fe_field(
	struct MANAGER(Computed_variable) *computed_variable_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Removes the finite_element Computed_variable that wraps <fe_field> from the
<computed_variable_manager> - this will mean that the wrapping Computed_variable is
destroyed.  Also removes the <fe_field> from the <fe_field_manager> - this will
mean that <fe_field> is destroyed.
==============================================================================*/

int Computed_variable_is_type_cmiss_number(struct Computed_variable *field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Returns true if <field> is a cmiss_number Computed_variable.
==============================================================================*/

int Computed_variable_set_type_cmiss_number(struct Computed_variable *field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Converts <field> into a cmiss_number Computed_variable - returns the cmiss number
of a node or an element.  Sets the number of components to 1.

If function fails, <field> is guaranteed to be unchanged from its original
state, although its cache may be lost.
==============================================================================*/

int Computed_variable_is_type_embedded(struct Computed_variable *field,void *dummy);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is an embedded
Computed_variable.
==============================================================================*/

int Computed_variable_depends_on_embedded_field(struct Computed_variable *field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Returns true if the field is of embedded type or depends on any computed fields
which are of embedded type.
==============================================================================*/

int Computed_variable_is_type_xi_coordinates(struct Computed_variable *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Iterator/conditional function returning true if <field> is an xi_coordinates
Computed_variable.
==============================================================================*/

int Computed_variable_set_type_xi_coordinates(struct Computed_variable *field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Converts <field> into a xi_coordinates Computed_variable.  Sets the number of
components to 3 - if the element has a dimension < 3 than the later xis are set
to zero.

If function fails, <field> is guaranteed to be unchanged from its original
state, although its cache may be lost.
==============================================================================*/

int Computed_variable_is_type_node_value(struct Computed_variable *field);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Returns true if <field> is a xi_coordinates Computed_variable.
==============================================================================*/

int Computed_variable_set_type_node_value(struct Computed_variable *field,
	struct FE_field *fe_field,enum FE_nodal_value_type nodal_value_type,
	int version_number);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Converts <field> into a node_value Computed_variable - returns the values for
given <nodal_value_type> and <version_number> of <fe_field> at a node.  Sets the
number of components and coordinate system to be the same as for <fe_field>.

If function fails, <field> is guaranteed to be unchanged from its original
state, although its cache may be lost.

???DB.  Will be replaced by the Computed_variable Computed_variables?
==============================================================================*/

struct LIST(FE_field) *Computed_variable_get_defining_FE_field_list(
	struct Computed_variable *field,
	struct MANAGER(Computed_variable) *computed_variable_manager);
/*******************************************************************************
LAST MODIFIED : 12 January 2003

DESCRIPTION :
Returns the list of FE_fields that <field> depends on, by looking through the
<computed_variable_manager>.
==============================================================================*/

struct Computed_variable_finite_element_package *
	Computed_variable_register_types_finite_element(
	struct Computed_variable_package *computed_variable_package,
	struct MANAGER(FE_field) *fe_field_manager,struct FE_time *fe_time);
/*******************************************************************************
LAST MODIFIED : 9 November 2001

DESCRIPTION :
This function registers the finite_element related types of Computed_variables and
also registers with the <fe_field_manager> so that any fe_fields are
automatically wrapped in corresponding computed_variables.
==============================================================================*/

int Computed_variable_deregister_types_finite_element(
	struct Computed_variable_finite_element_package
	*computed_variable_finite_element_package);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
#endif /* defined (OLD_CODE) */
