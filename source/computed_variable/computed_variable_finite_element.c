/*******************************************************************************
FILE : computed_variable_finite_element.c

LAST MODIFIED : 20 July 2003

DESCRIPTION :
Implements computed variables which interface to finite element fields:
- cmiss_number.  Returns the cmiss number of a node or an element
- embedded.  Used for node fields that give element/xi - data at material points
???DB.  Why is Cmiss_variable_set_type_embedded static?
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

???DB.  The modify function on FE fields does more than add/subtract 2*pi.  It
	will use the outside value when the inside value is on the axis.  This will
	make derivatives wrt to axial nodal values 0.  So no_modify is not good
	enough?
	No.  The modify behaviour is a special/discontinuous behaviour in the
	derivative and should be ignored/got rid of.
==============================================================================*/
#include "computed_variable/computed_value_derivative_matrix.h"
#include "computed_variable/computed_value_finite_element.h"
#include "computed_variable/computed_value_matrix.h"
#include "computed_variable/computed_variable_finite_element.h"
#include "computed_variable/computed_variable_private.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
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
static char Cmiss_variable_element_value_type_string[]="Element_value";

/*???DB.  To be done */

static char Cmiss_variable_element_xi_type_string[]="Element_xi";

struct Cmiss_variable_element_xi_type_specific_data
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
???DB.  Does this need the option of having a variable? ie.
	<finite_element_variable> should be a particular finite element computed
	variable or NULL if the element/xi is for all finite element computed fields.
==============================================================================*/
{
	Cmiss_value_id element_xi;
}; /* struct Cmiss_variable_element_xi_type_specific_data */

static START_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(element_xi)
{
	DEACCESS(Cmiss_value)(&(data->element_xi));
	return_code=1;
}
END_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(element_xi)

static START_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(
	element_xi)
{
	if (destination->element_xi=CREATE(Cmiss_value)())
	{
		ACCESS(Cmiss_value)(destination->element_xi);
		if (!Cmiss_value_copy(destination->element_xi,source->element_xi))
		{
			DEACCESS(Cmiss_value)(&(destination->element_xi));
			DEALLOCATE(destination);
		}
	}
	else
	{
		DEALLOCATE(destination);
	}
}
END_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(element_xi)

#if defined (OLD_CODE)
/*???DB.  Changing from fe_value_matrix to matrix and to storing the blocks
	within fe_value_matrix as separate matrices */
static START_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	element_xi)
{
	FE_value *matrix_value,*matrix_values;
	int i,j,number_of_columns,number_of_columns_save,number_of_rows,
		number_of_values;
	Cmiss_value_id value_type;
	Cmiss_variable_id independent_variable,*independent_variable_address;
	struct Cmiss_variable_element_xi_type_specific_data *data;
	struct FE_element *element;

	data=(struct Cmiss_variable_element_xi_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(element_xi)(data->element_xi),
		return_code,0)
	{
		/* get number of rows in derivative matrix */
		if (Cmiss_value_element_xi_get_type(data->element_xi,&element,
			(FE_value **)NULL))
		{
			if (0<(number_of_rows=get_FE_element_dimension(element)))
			{
				return_code=1;
			}
		}
		if (return_code)
		{
			if (value_type=CREATE(Cmiss_value)())
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
					if (return_code=Cmiss_variable_get_value_type(independent_variable,
						value_type))
					{
						if (CMISS_VALUE_IS_TYPE(FE_value)(value_type))
						{
							number_of_values=1;
						}
						else if (CMISS_VALUE_IS_TYPE(FE_value_vector)(value_type))
						{
							return_code=Cmiss_value_FE_value_vector_get_type(value_type,
								&number_of_values,(FE_value **)NULL);
						}
						else if (CMISS_VALUE_IS_TYPE(element_xi)(value_type))
						{
							if (Cmiss_value_element_xi_get_type(value_type,
								&element,(FE_value **)NULL))
							{
								number_of_values=get_FE_element_dimension(element);
							}
						}
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
									/*???DB.  Assuming that only independent variable is itself */
									if (Cmiss_variable_same_variable(variable,
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
										"Cmiss_variable_element_xi_evaluate_derivative_type_specific.  "
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
					if (Cmiss_value_FE_value_matrix_set_type(value_type,
						number_of_rows,number_of_columns,matrix_values))
					{
						if (ALLOCATE(independent_variable_address,
							Cmiss_variable_id ,order))
						{
							for (i=0;i<order;i++)
							{
								independent_variable_address[i]=independent_variables[i];
							}
							if (return_code=Cmiss_value_derivative_matrix_set_type(value,
								variable,order,independent_variable_address,value_type))
							{
								DEALLOCATE(independent_variable_address);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_variable_element_xi_evaluate_derivative_type_specific.  "
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
					DESTROY(Cmiss_value)(&value_type);
				}
			}
			else
			{
				return_code=0;
			}
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(element_xi)
#endif /* defined (OLD_CODE) */

static START_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	element_xi)
{
	Cmiss_value_id *matrices,*matrix,value_type;
	Cmiss_variable_id independent_variable,*independent_variable_address;
	int i,j,number_of_columns,number_of_matrices,number_of_rows,number_of_values,
		offset;
	struct Cmiss_variable_element_xi_type_specific_data *data;
	struct Matrix *matrix_local;

	data=(struct Cmiss_variable_element_xi_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(element_xi)(data->element_xi),
		return_code,0)
	{
		/* get number of rows in derivative matrix */
		return_code=Cmiss_value_element_xi_get_type(data->element_xi,
			&number_of_rows,(struct FE_element **)NULL,(FE_value **)NULL);
		if (return_code&&(0<number_of_rows))
		{
			if (value_type=CREATE(Cmiss_value)())
			{
				/* set up the matrices in the derivative */
				i=order;
				number_of_matrices=0;
				matrices=(Cmiss_value_id *)NULL;
				independent_variable_address=independent_variables;
				while (return_code&&(i>0))
				{
					independent_variable= *independent_variable_address;
					if (return_code=Cmiss_variable_get_value_type(independent_variable,
						value_type))
					{
#if defined (OLD_CODE)
						if (CMISS_VALUE_IS_TYPE(FE_value)(value_type))
						{
							number_of_values=1;
						}
						else if (CMISS_VALUE_IS_TYPE(FE_value_vector)(value_type))
						{
							return_code=Cmiss_value_FE_value_vector_get_type(value_type,
								&number_of_values,(FE_value **)NULL);
						}
						else if (CMISS_VALUE_IS_TYPE(element_xi)(value_type))
						{
							return_code=Cmiss_value_element_xi_get_type(value_type,
								&number_of_values,(struct FE_element **)NULL,(FE_value **)NULL);
						}
						else
						{
							return_code=0;
						}
#endif /* defined (OLD_CODE) */
						return_code=Cmiss_value_get_reals(value_type,&number_of_values,
							(FE_value **)NULL);
						if (return_code&&(0<number_of_values))
						{
							if (REALLOCATE(matrix,matrices,Cmiss_value_id,
								2*number_of_matrices+1))
							{
								matrices=matrix;
								matrix += number_of_matrices;
								if (*matrix=CREATE(Cmiss_value)())
								{
									number_of_matrices++;
									if (matrix_local=CREATE(Matrix)("matrix",DENSE,number_of_rows,
										number_of_values))
									{
										if (return_code=Cmiss_value_matrix_set_type(*matrix,
											matrix_local))
										{
											/*???DB.  Assuming that only independent variable is
												itself */
											if (Cmiss_variable_same_variable(variable,
												independent_variable)&&
												(number_of_rows==number_of_values))
											{
												j=number_of_rows;
												while (return_code&&(j>0))
												{
													return_code=Cmiss_value_matrix_set_value(*matrix,j,j,
														(Matrix_value)1);
													j--;
												}
											}
											offset=number_of_matrices;
											j=number_of_matrices-1;
											while (return_code&&(j>0))
											{
												matrix++;
												if (return_code=Cmiss_value_matrix_get_dimensions(
													*(matrix-offset),(int *)NULL,&number_of_columns))
												{
													if (*matrix=CREATE(Cmiss_value)())
													{
														number_of_matrices++;
														if (matrix_local=CREATE(Matrix)("matrix",DENSE,
															number_of_rows,number_of_values*
															number_of_columns))
														{
															if (return_code=Cmiss_value_matrix_set_type(
																*matrix,matrix_local))
															{
																j--;
															}
															else
															{
																DESTROY(Matrix)(&matrix_local);
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
											}
										}
										else
										{
											DESTROY(Matrix)(&matrix_local);
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
					i--;
					independent_variable_address++;
				}
				if (return_code)
				{
					if (ALLOCATE(independent_variable_address,Cmiss_variable_id,order))
					{
						for (i=0;i<order;i++)
						{
							independent_variable_address[i]=independent_variables[i];
						}
						if (return_code=Cmiss_value_derivative_matrix_set_type(value,
							variable,order,independent_variable_address,matrices))
						{
							/* independent_variable_address and matrices are now managed by
								value */
							independent_variable_address=(Cmiss_variable_id *)NULL;
							matrices=(Cmiss_value_id *)NULL;
						}
						if (independent_variable_address)
						{
							DEALLOCATE(independent_variable_address);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_variable_element_xi_evaluate_derivative_type_specific.  "
							"Could not allocate <independent_variable_address>");
						return_code=0;
					}
				}
				if (matrices)
				{
					matrix=matrices;
					i=number_of_matrices;
					while (i>0)
					{
						DESTROY(Cmiss_value)(matrix);
						matrix++;
						i--;
					}
					DEALLOCATE(matrices);
				}
				DESTROY(Cmiss_value)(&value_type);
			}
			else
			{
				return_code=0;
			}
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(element_xi)

static START_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(element_xi)
{
	struct Cmiss_variable_element_xi_type_specific_data *data;

	data=(struct Cmiss_variable_element_xi_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(element_xi)(data->element_xi),
		return_code,0)
	{
		if (Cmiss_value_copy(value,data->element_xi))
		{
			return_code=0;
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(element_xi)

static
	START_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	element_xi)
{
	struct Cmiss_variable_element_xi_type_specific_data *data;

	data=(struct Cmiss_variable_element_xi_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(element_xi)(data->element_xi),
		return_code,0)
	{
		if (Cmiss_variable_same_variable(dependent_variable,
			independent_variable))
		{
			return_code=Cmiss_value_copy(value,data->element_xi);
		}
	}
}
END_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	element_xi)

static
	START_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	element_xi)
{
	Cmiss_value_id temp_value;
	struct Cmiss_variable_element_xi_type_specific_data *data;

	data=(struct Cmiss_variable_element_xi_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(element_xi)(data->element_xi),
		return_code,0)
	{
		if (Cmiss_variable_same_variable(dependent_variable,
			independent_variable)&&CMISS_VALUE_IS_TYPE(element_xi)(value))
		{
			if (temp_value=CREATE(Cmiss_value)())
			{
				if (Cmiss_value_copy(temp_value,data->element_xi))
				{
					if (Cmiss_value_copy(data->element_xi,value))
					{
						return_code=Cmiss_value_copy(value,temp_value);
					}
				}
				DESTROY(Cmiss_value)(&temp_value);
			}
		}
	}
}
END_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	element_xi)

static START_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(
	element_xi)
{
	int dimension;
	struct Cmiss_variable_element_xi_type_specific_data *data;

	data=(struct Cmiss_variable_element_xi_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(element_xi)(data->element_xi),
		return_code,0)
	{
		if (Cmiss_value_element_xi_get_type(data->element_xi,&dimension,
			(struct FE_element **)NULL,(FE_value **)NULL))
		{
			return_code=Cmiss_value_element_xi_set_type(type,dimension,
				(struct FE_element *)NULL,(FE_value *)NULL);
		}
	}
}
END_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(element_xi)

static START_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(element_xi)
{
	FE_value *xi;
	int dimension;
	struct Cmiss_variable_element_xi_type_specific_data *data;
	struct FE_element *element;

	data=(struct Cmiss_variable_element_xi_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(element_xi)(data->element_xi),
		return_code,0)
	{
		if (Cmiss_value_element_xi_get_type(data->element_xi,&dimension,&element,
			&xi)&&(dimension>0)&&element&&xi)
		{
			return_code=1;
		}
	}
}
END_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(element_xi)

static
	START_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	element_xi)
{
	return_code=Cmiss_variable_same_variable(dependent_variable,
		independent_variable);
}
END_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	element_xi)

static START_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(element_xi)
{
	return_code=1;
}
END_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(element_xi)

static START_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(element_xi)
{
	return_code=Cmiss_variable_same_variable(variable_1,variable_2);
}
END_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(element_xi)

static START_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(
	element_xi)
{
	int dimension_1,dimension_2;
	struct Cmiss_variable_element_xi_type_specific_data *data_1,*data_2;
	struct FE_element *element_1,*element_2;

	data_1=(struct Cmiss_variable_element_xi_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_1);
	data_2=(struct Cmiss_variable_element_xi_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&CMISS_VALUE_IS_TYPE(element_xi)(data_1->element_xi)&&
		data_2&&CMISS_VALUE_IS_TYPE(element_xi)(data_2->element_xi),return_code,
		0)
	{
		if (Cmiss_value_element_xi_get_type(data_1->element_xi,&dimension_1,
			&element_1,(FE_value **)NULL)&&Cmiss_value_element_xi_get_type(
			data_2->element_xi,&dimension_2,&element_2,(FE_value **)NULL))
		{
			if (element_1&&element_2)
			{
				if (dimension_1==dimension_2)
				{
					return_code=1;
				}
			}
			else
			{
				if (!element_1&&!element_2)
				{
					if (dimension_1==dimension_2)
					{
						return_code=1;
					}
				}
			}
		}
	}
}
END_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(element_xi)

static
	START_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	element_xi)
{
	struct Cmiss_variable_element_xi_type_specific_data *data;

	data=(struct Cmiss_variable_element_xi_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(element_xi)(data->element_xi),
		return_code,0)
	{
		if (Cmiss_variable_same_variable(dependent_variable,
			independent_variable))
		{
			/*???DB.  Copy into <independent_variable> as well? */
			return_code=Cmiss_value_copy(data->element_xi,value);
		}
	}
}
END_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	element_xi)

static char Cmiss_variable_FE_time_type_string[]="FE_time";

/*???DB.  To be done */

static char Cmiss_variable_finite_element_type_string[]="Finite_element";

struct Cmiss_variable_finite_element_type_specific_data
/*******************************************************************************
LAST MODIFIED : 11 July 2003

DESCRIPTION :
???DB.  Assuming that the <element>, <field> and <node> know their FE_region
	(can get managers)
==============================================================================*/
{
	FE_value time,*xi;
	int component_number,dimension;
	struct FE_element *element;
	struct FE_field *field;
	struct FE_node *node;
}; /* struct Cmiss_variable_finite_element_type_specific_data */

static START_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(finite_element)
{
	DEACCESS(FE_element)(&(data->element));
	DEACCESS(FE_field)(&(data->field));
	DEACCESS(FE_node)(&(data->node));
	DEALLOCATE(data->xi);
	return_code=1;
}
END_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(finite_element)

static START_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(
	finite_element)
{
	int dimension,i;

	if (source->element)
	{
		dimension=source->dimension;
		if ((0<dimension)&&(source->xi)&&ALLOCATE(destination->xi,FE_value,
			dimension))
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
		destination->dimension=source->dimension;
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
END_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(finite_element)

struct Count_nodal_values_data
{
	enum FE_nodal_value_type value_type;
	int component_number,number_of_components,number_of_values,version;
	struct FE_field *fe_field;
	int *node_offsets,number_of_node_offsets,*number_of_node_values;
	struct FE_node **offset_nodes;
}; /* struct Count_nodal_values_data */

static int component_count_nodal_values(struct FE_node *node,
	struct FE_field *fe_field,int component_number,
	enum FE_nodal_value_type value_type,int version)
/*******************************************************************************
LAST MODIFIED : 29 April 2003

DESCRIPTION :
==============================================================================*/
{
	enum FE_nodal_value_type *nodal_value_type,*nodal_value_types;
	int i,number_of_versions,number_of_values;

	ENTER(component_count_nodal_values);
	number_of_values=1+get_FE_node_field_component_number_of_derivatives(node,
		fe_field,component_number);
	if (FE_NODAL_UNKNOWN!=value_type)
	{
		/*???DB.  Could use FE_nodal_value_version_exists instead */
		i=number_of_values;
		number_of_values=0;
		if (nodal_value_types=get_FE_node_field_component_nodal_value_types(node,
			fe_field,component_number))
		{
			nodal_value_type=nodal_value_types;
			while ((i>0)&&(value_type!= *nodal_value_type))
			{
				nodal_value_type++;
				i--;
			}
			if (i>0)
			{
				number_of_values++;
			}
			DEALLOCATE(nodal_value_types);
		}
	}
	number_of_versions=get_FE_node_field_component_number_of_versions(node,
		fe_field,component_number);
	if ((version<0)||(version>number_of_versions))
	{
		number_of_values *= number_of_versions;
	}
	LEAVE;

	return (number_of_values);
} /* component_count_nodal_values */

static int count_nodal_values(struct FE_node *node,
	void *count_nodal_values_data_void)
/*******************************************************************************
LAST MODIFIED : 29 April 2003

DESCRIPTION :
Counts the number of nodal values specified by <count_nodal_values_data_void> at
the <node>.
==============================================================================*/
{
	int component_number,node_number,number_of_values,return_code;
	struct Count_nodal_values_data *count_nodal_values_data;
	struct FE_node **offset_nodes;

	ENTER(count_nodal_values);
	return_code=0;
	/* check arguments */
	if (node&&(count_nodal_values_data=(struct Count_nodal_values_data *)
		count_nodal_values_data_void))
	{
		if (offset_nodes=count_nodal_values_data->offset_nodes)
		{
			node_number=0;
			while ((node_number<count_nodal_values_data->number_of_node_offsets)&&
				(node!=offset_nodes[node_number]))
			{
				node_number++;
			}
			if (node_number<count_nodal_values_data->number_of_node_offsets)
			{
				(count_nodal_values_data->node_offsets)[node_number]=
					count_nodal_values_data->number_of_values;
			}
		}
		if ((0<=(component_number=count_nodal_values_data->component_number))&&
			(component_number<count_nodal_values_data->number_of_components))
		{
			number_of_values=component_count_nodal_values(node,
				count_nodal_values_data->fe_field,component_number,
				count_nodal_values_data->value_type,
				count_nodal_values_data->version);
		}
		else
		{
			number_of_values=0;
			for (component_number=0;component_number<
				count_nodal_values_data->number_of_components;component_number++)
			{
				number_of_values += component_count_nodal_values(node,
					count_nodal_values_data->fe_field,component_number,
					count_nodal_values_data->value_type,
					count_nodal_values_data->version);
			}
		}
		if ((offset_nodes=count_nodal_values_data->offset_nodes)&&
			(node_number<count_nodal_values_data->number_of_node_offsets))
		{
			(count_nodal_values_data->number_of_node_values)[node_number]=
				number_of_values;
		}
		count_nodal_values_data->number_of_values += number_of_values;
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* count_nodal_values */

static int extract_component_values(
	struct FE_element_field_values *element_field_values,
	int number_of_components,int component_number,
	int **numbers_of_component_values_address,
	FE_value ***component_values_address)
/*******************************************************************************
LAST MODIFIED : 5 April 2003

DESCRIPTION :
Extracts the values for the component(s) (<number_of_components> and
<component_number>) from <element_field_values> and puts in
<*component_values_address> (after allocating storage).

If <*numbers_of_component_values_address> is NULL then storage is allocated for
it and the number of values extracted for each component entered.  Otherwise,
checks that the number of values extracted for each component agrees with the
entries.
==============================================================================*/
{
	FE_value **component_values;
	int extract_numbers_of_component_values,i,number_of_component_values,
		*numbers_of_component_values,return_code;

	ENTER(extract_component_values);
	return_code=0;
	ASSERT_IF(element_field_values&&
		(((1==number_of_components)&&(0<=component_number))||
		((0<number_of_components)&&(-1==component_number)))&&
		numbers_of_component_values_address&&component_values_address,return_code,0)
	{
		extract_numbers_of_component_values=
			!(*numbers_of_component_values_address);
		/* allocate storage for components */
		if (extract_numbers_of_component_values)
		{
			ALLOCATE(numbers_of_component_values,int,number_of_components);
		}
		else
		{
			numbers_of_component_values= *numbers_of_component_values_address;
		}
		ALLOCATE(component_values,FE_value *,number_of_components);
		if (component_values&&numbers_of_component_values)
		{
			if (-1==component_number)
			{
				i=0;
				return_code=1;
				while (return_code&&(i<number_of_components))
				{
					if (return_code=FE_element_field_values_get_component_values(
						element_field_values,i,&number_of_component_values,
						component_values+i))
					{
						if (extract_numbers_of_component_values)
						{
							numbers_of_component_values[i]=number_of_component_values;
						}
						else
						{
							if (numbers_of_component_values[i]!=number_of_component_values)
							{
								DEALLOCATE(component_values[i]);
								return_code=0;
							}
						}
						i++;
					}
				}
				if (!return_code)
				{
					while (i>0)
					{
						i--;
						DEALLOCATE(component_values[i]);
					}
				}
			}
			else
			{
				if (return_code=FE_element_field_values_get_component_values(
					element_field_values,component_number,&number_of_component_values,
					component_values))
				{
					if (extract_numbers_of_component_values)
					{
						*numbers_of_component_values=number_of_component_values;
					}
					else
					{
						if (*numbers_of_component_values!=number_of_component_values)
						{
							DEALLOCATE(*component_values);
							return_code=0;
						}
					}
				}
			}
			if (return_code)
			{
				*component_values_address=component_values;
				*numbers_of_component_values_address=numbers_of_component_values;
			}
		}
		if (!return_code)
		{
			DEALLOCATE(component_values);
			if (extract_numbers_of_component_values)
			{
				DEALLOCATE(numbers_of_component_values);
			}
		}
	}
	LEAVE;

	return (return_code);
} /* extract_component_values */

static int extract_component_monomial_info(
	struct FE_element_field_values *element_field_values,
	int number_of_components,int component_number,int number_of_xi,
	int **numbers_of_component_values_address,
	int ***component_monomial_info_address,FE_value **monomial_values_address)
/*******************************************************************************
LAST MODIFIED : 5 April 2003

DESCRIPTION :
Extracts the component monomial info (<number_of_components> and
<component_number>) from <element_field_values> and puts in
<*component_monomial_info_address> (after allocating storage).

If <*numbers_of_component_values_address> is NULL then storage is allocated for
it and the number of values for each component calculated and entered.
Otherwise, checks that the number of values extracted for each component agrees
with the entries.

If <monomial_values_address> is not NULL and <*monomial_values_address> is NULL,
then storage for the maximum number of component values is allocated and
assigned to <*monomial_values_address>.
==============================================================================*/
{
	FE_value *monomial_values;
	int **component_monomial_info,extract_numbers_of_component_values,i,j,
		maximum_number_of_component_values,number_of_component_values,
		*numbers_of_component_values,return_code;

	ENTER(extract_component_monomial_info);
	return_code=0;
	ASSERT_IF(element_field_values&&
		(((1==number_of_components)&&(0<=component_number))||
		((0<number_of_components)&&(-1==component_number)))&&(0<number_of_xi)&&
		numbers_of_component_values_address&&component_monomial_info_address,
		return_code,0)
	{
		/* allocate storage for components */
		extract_numbers_of_component_values=
			!(*numbers_of_component_values_address);
		/* allocate storage for components */
		if (extract_numbers_of_component_values)
		{
			ALLOCATE(numbers_of_component_values,int,number_of_components);
		}
		else
		{
			numbers_of_component_values= *numbers_of_component_values_address;
		}
		ALLOCATE(component_monomial_info,int *,number_of_components);
		if (numbers_of_component_values&&component_monomial_info)
		{
			i=0;
			return_code=1;
			maximum_number_of_component_values=0;
			while (return_code&&(i<number_of_components))
			{
				if (ALLOCATE(component_monomial_info[i],int,number_of_xi+1))
				{
					if (-1==component_number)
					{
						return_code=FE_element_field_values_get_monomial_component_info(
							element_field_values,i,component_monomial_info[i]);
					}
					else
					{
						return_code=FE_element_field_values_get_monomial_component_info(
							element_field_values,component_number,component_monomial_info[i]);
					}
					if (return_code)
					{
						number_of_component_values=1;
						for (j=1;j<=component_monomial_info[i][0];j++)
						{
							number_of_component_values *= 1+component_monomial_info[i][j];
						}
						if (number_of_component_values>maximum_number_of_component_values)
						{
							maximum_number_of_component_values=number_of_component_values;
						}
						if (extract_numbers_of_component_values)
						{
							numbers_of_component_values[i]=number_of_component_values;
						}
						else
						{
							if (numbers_of_component_values[i]!=number_of_component_values)
							{
								return_code=0;
							}
						}
					}
					i++;
				}
			}
			if (return_code)
			{
				if (monomial_values_address&&!(*monomial_values_address))
				{
					if ((0<maximum_number_of_component_values)&&ALLOCATE(monomial_values,
						FE_value,maximum_number_of_component_values))
					{
						*monomial_values_address=monomial_values;
					}
					else
					{
						return_code=0;
					}
				}
				if (return_code)
				{
					*numbers_of_component_values_address=numbers_of_component_values;
					*component_monomial_info_address=component_monomial_info;
				}
			}
			if (!return_code)
			{
				while (i>0)
				{
					i--;
					DEALLOCATE(component_monomial_info[i]);
				}
			}
		}
		if (!return_code)
		{
			DEALLOCATE(component_monomial_info);
			if (extract_numbers_of_component_values)
			{
				DEALLOCATE(numbers_of_component_values);
			}
		}
	}
	LEAVE;

	return (return_code);
} /* extract_component_monomial_info */

static int nodal_value_calculate_component_values(
	Cmiss_variable_id fe_variable,struct FE_field *fe_field,
	int component_number,FE_value fe_time,struct FE_element *element,
	struct FE_node *node,enum FE_nodal_value_type nodal_value_type,int version,
	int *number_of_element_field_nodes_address,
	struct FE_node ***element_field_nodes_address,
	struct FE_element_field_values *element_field_values,
	int *number_of_nodal_values_address,int **numbers_of_component_values_address,
	FE_value ****component_values_address)
/*******************************************************************************
LAST MODIFIED : 2 May 2003

DESCRIPTION :
Calculate the component values for the derivatives of the specified components
(<component_number>) of <fe_field> in the <element> with respect to the
specified nodal values (<node>, <nodal_value_type> and <version>.

<*number_of_nodal_values_address> is calculated and an array with an entry for
each of the specified nodal values (<*number_of_nodal_values_address> long> is
allocated, NULLed and assigned to <*component_values_address>.  For each nodal
value, if the derivative is not identically zero then a 2 dimensional array
(first index is component number and second index is component value number) is
allocated, filled-in and assigned to the corresponding entry in
<*component_values_address>.

The interpolation function for the <fe_field> component(s) is assumed to be a
linear combination of basis functions polynomial in the element coordinates.
The coefficients for the linear combination are the nodal values.  So, the
derivative with respect to a nodal value is the corresponding basis function.
The component values are for the monomials.  To calculate these, all the nodal
values for the <element> are set to zero, the nodal value of interest is set to
one and <calculate_FE_element_field_values> is used.

<*number_of_element_field_nodes_address> and <*element_field_nodes_address> are
for the <fe_field> and <element> and can be passed in or computed in here.
<element_field_values> is working storage that is passed in.

???DB.  Can get all from <fe_variable>.  Do multiple times?
???DB.  Have to call with variable (not independent_fe_variable which may be
	NULL)
==============================================================================*/
{
	FE_value ***component_values,***nodal_value_component_values,
		*temp_FE_value_address;
	int *element_field_nodal_value_offsets,*element_field_number_of_nodal_values,
		*element_field_number_of_specified_nodal_values,i,j,k,number_of_components,
		number_of_element_field_nodes,number_of_nodal_values,
		number_of_saved_element_field_nodes,number_of_values,number_of_versions,
		return_code;
	Cmiss_value_id *element_field_saved_nodal_values,
		*element_field_specified_nodal_values;
	Cmiss_variable_id variable_temp;
	struct Count_nodal_values_data count_nodal_values_data;
	struct FE_region *fe_region;
	struct FE_node **element_field_nodes;

	ENTER(nodal_value_calculate_component_values);
	return_code=0;
	/* check arguments */
	fe_region=FE_field_get_FE_region(fe_field);
	number_of_components=get_FE_field_number_of_components(fe_field);
	if (fe_variable&&fe_field&&fe_region&&(0<number_of_components)&&
		((-1==component_number)||
		((0<=component_number)&&(component_number<number_of_components)))&&
		element&&element_field_values&&number_of_nodal_values_address&&
		component_values_address)
	{
		if (-1!=component_number)
		{
			number_of_components=1;
		}
		if (number_of_element_field_nodes_address&&element_field_nodes_address)
		{
			number_of_element_field_nodes= *number_of_element_field_nodes_address;
			element_field_nodes= *element_field_nodes_address;
		}
		else
		{
			number_of_element_field_nodes=0;
			element_field_nodes=(struct FE_node **)NULL;
		}
		if ((number_of_element_field_nodes>0)&&element_field_nodes)
		{
			return_code=0;
		}
		else
		{
			return_code=calculate_FE_element_field_nodes(element,
				fe_field,&number_of_element_field_nodes,
				&element_field_nodes,(struct FE_element *)NULL);
		}
		if (return_code)
		{
			/* set up working storage */
			ALLOCATE(element_field_nodal_value_offsets,int,
				number_of_element_field_nodes);
			ALLOCATE(element_field_number_of_specified_nodal_values,int,
				number_of_element_field_nodes);
			ALLOCATE(element_field_number_of_nodal_values,int,
				number_of_element_field_nodes);
			ALLOCATE(element_field_saved_nodal_values,Cmiss_value_id,
				number_of_element_field_nodes);
			ALLOCATE(element_field_specified_nodal_values,Cmiss_value_id,
				number_of_element_field_nodes);
			variable_temp=CREATE(Cmiss_variable)(
				(struct Cmiss_variable_package *)NULL,"temp");
			if (element_field_nodal_value_offsets&&
				element_field_number_of_specified_nodal_values&&
				element_field_number_of_nodal_values&&
				element_field_saved_nodal_values&&
				element_field_specified_nodal_values&&variable_temp)
			{
				/* NULL working storage and calculate total number of nodal values for
					each <element_field_node> */
				for (i=0;i<number_of_element_field_nodes;i++)
				{
					element_field_saved_nodal_values[i]=(Cmiss_value_id )NULL;
					element_field_specified_nodal_values[i]=(Cmiss_value_id )NULL;
					element_field_nodal_value_offsets[i]= -1;
					element_field_number_of_specified_nodal_values[i]=0;
					element_field_number_of_nodal_values[i]=0;
					if (-1==component_number)
					{
						for (j=0;j<number_of_components;j++)
						{
							number_of_values=
								(1+get_FE_node_field_component_number_of_derivatives(
								element_field_nodes[i],fe_field,j));
							number_of_versions=
								get_FE_node_field_component_number_of_versions(
								element_field_nodes[i],fe_field,j);
							if ((version<0)||(version>number_of_versions))
							{
								element_field_number_of_nodal_values[i] +=
									number_of_values*number_of_versions;
							}
							else
							{
								element_field_number_of_nodal_values[i] += number_of_values;
							}
						}
					}
					else
					{
						number_of_values=
							(1+get_FE_node_field_component_number_of_derivatives(
							element_field_nodes[i],fe_field,component_number));
						number_of_versions=
							get_FE_node_field_component_number_of_versions(
							element_field_nodes[i],fe_field,component_number);
						if ((version<0)||(version>number_of_versions))
						{
							element_field_number_of_nodal_values[i] +=
								number_of_values*number_of_versions;
						}
						else
						{
							element_field_number_of_nodal_values[i] += number_of_values;
						}
					}
				}
				/* calculate total number of specified nodal values (<number_of_values>)
					and the number of specified nodal values for the
					<element_field_nodes> */
				count_nodal_values_data.number_of_values=0;
				count_nodal_values_data.value_type=nodal_value_type;
				count_nodal_values_data.version=version;
				count_nodal_values_data.fe_field=fe_field;
				count_nodal_values_data.component_number=component_number;
				count_nodal_values_data.number_of_components=number_of_components;
				count_nodal_values_data.number_of_node_offsets=
					number_of_element_field_nodes;
				count_nodal_values_data.offset_nodes=element_field_nodes;
				count_nodal_values_data.node_offsets=element_field_nodal_value_offsets;
				count_nodal_values_data.number_of_node_values=
					element_field_number_of_specified_nodal_values;
				if (node)
				{
					return_code=count_nodal_values(node,(void *)&count_nodal_values_data);
				}
				else
				{
					return_code=FE_region_for_each_FE_node(fe_region,
						count_nodal_values,(void *)&count_nodal_values_data);
				}
				number_of_nodal_values=count_nodal_values_data.number_of_values;
				if (return_code&&(0<number_of_nodal_values))
				{
					/* create working computed values for saving current values and
						setting temporary values */
					i=0;
					while (return_code&&(i<number_of_element_field_nodes))
					{
						if (0<element_field_number_of_nodal_values[i])
						{
							if ((element_field_saved_nodal_values[i]=
								CREATE(Cmiss_value)())&&ALLOCATE(temp_FE_value_address,
								FE_value,element_field_number_of_nodal_values[i])&&
								Cmiss_value_FE_value_vector_set_type(
								element_field_saved_nodal_values[i],
								element_field_number_of_nodal_values[i],
								temp_FE_value_address))
							{
								for (j=element_field_number_of_nodal_values[i];j>0;j--)
								{
									*temp_FE_value_address=(FE_value)0;
									temp_FE_value_address++;
								}
							}
							else
							{
								if (element_field_saved_nodal_values[i])
								{
									if (temp_FE_value_address)
									{
										DEALLOCATE(temp_FE_value_address);
									}
									DESTROY(Cmiss_value)(element_field_saved_nodal_values+i);
								}
								return_code=0;
							}
						}
						if (return_code&&(0<
							element_field_number_of_specified_nodal_values[i]))
						{
							if ((element_field_specified_nodal_values[i]=
								CREATE(Cmiss_value)())&&ALLOCATE(temp_FE_value_address,
								FE_value,element_field_number_of_specified_nodal_values[i])&&
								Cmiss_value_FE_value_vector_set_type(
								element_field_specified_nodal_values[i],
								element_field_number_of_specified_nodal_values[i],
								temp_FE_value_address))
							{
								for (j=element_field_number_of_specified_nodal_values[i];j>0;
									j--)
								{
									*temp_FE_value_address=(FE_value)0;
									temp_FE_value_address++;
								}
							}
							else
							{
								if (element_field_specified_nodal_values[i])
								{
									if (temp_FE_value_address)
									{
										DEALLOCATE(temp_FE_value_address);
									}
									DESTROY(Cmiss_value)(
										element_field_specified_nodal_values+i);
								}
								return_code=0;
							}
						}
						i++;
					}
					/* allocate storage for the component_values */
					if (return_code&&ALLOCATE(component_values,FE_value **,
						number_of_nodal_values))
					{
						/* NULL the component values array */
						nodal_value_component_values=component_values;
						for (i=number_of_nodal_values;i>0;i--)
						{
							*nodal_value_component_values=(FE_value **)NULL;
							nodal_value_component_values++;
						}
						/* save and zero all the nodal values for the <fe_field> on the
							element */
						number_of_saved_element_field_nodes=0;
						while (return_code&&(number_of_saved_element_field_nodes<
							number_of_element_field_nodes))
						{
							if (return_code=Cmiss_variable_nodal_value_set_type(
								variable_temp,fe_variable,element_field_nodes[
								number_of_saved_element_field_nodes],FE_NODAL_UNKNOWN,version))
							{
								return_code=
									Cmiss_variable_get_set_independent_variable_value(
									fe_variable,variable_temp,
									element_field_saved_nodal_values[
									number_of_saved_element_field_nodes]);
							}
							number_of_saved_element_field_nodes++;
						}
						/* calculate component values for nodal value derivatives */
						i=0;
						while (return_code&&(i<number_of_element_field_nodes))
						{
							if ((element_field_nodal_value_offsets[i]>=0)&&
								(element_field_number_of_specified_nodal_values[i]>0))
							{
								nodal_value_component_values=component_values+
									element_field_nodal_value_offsets[i];
								if (Cmiss_value_FE_value_vector_get_type(
									element_field_specified_nodal_values[i],&number_of_values,
									&temp_FE_value_address)&&(number_of_values==
									element_field_number_of_specified_nodal_values[i])&&
									Cmiss_variable_nodal_value_set_type(variable_temp,
									fe_variable,element_field_nodes[i],nodal_value_type,
									version))
								{
									j=0;
									while (return_code&&(j<number_of_values))
									{
										/* set nodal values */
										temp_FE_value_address[j]=(FE_value)1;
										if (Cmiss_variable_set_independent_variable_value(
											fe_variable,variable_temp,
											element_field_specified_nodal_values[i])&&
											clear_FE_element_field_values(element_field_values)&&
											FE_element_field_values_set_no_modify(
											element_field_values)&&
											calculate_FE_element_field_values(element,fe_field,
											fe_time,(char)0,element_field_values,
											(struct FE_element *)NULL))
										{
											return_code=extract_component_values(
												element_field_values,number_of_components,
												component_number,numbers_of_component_values_address,
												nodal_value_component_values);
										}
										else
										{
											return_code=0;
										}
										/* set nodal values */
										temp_FE_value_address[j]=(FE_value)0;
										nodal_value_component_values++;
										j++;
									}
									if (return_code)
									{
										return_code=Cmiss_variable_set_independent_variable_value(
											fe_variable,variable_temp,
											element_field_specified_nodal_values[i]);
									}
									if (!return_code)
									{
										while (j>0)
										{
											j--;
											nodal_value_component_values--;
											if (*nodal_value_component_values)
											{
												for (k=0;k<number_of_components;k++)
												{
													DEALLOCATE((*nodal_value_component_values)[k]);
												}
												DEALLOCATE(*nodal_value_component_values);
											}
										}
									}
								}
								else
								{
									return_code=0;
								}
							}
							i++;
						}
						if (return_code)
						{
							*number_of_nodal_values_address=number_of_nodal_values;
							*component_values_address=component_values;
						}
						else
						{
							while (i>0)
							{
								i--;
								if ((element_field_nodal_value_offsets[i]>=0)&&
									(element_field_number_of_specified_nodal_values[i]>0))
								{
									nodal_value_component_values=component_values+
										element_field_nodal_value_offsets[i];
									for (j=0;
										j<element_field_number_of_specified_nodal_values[i];j++)
									{
										if (*nodal_value_component_values)
										{
											for (k=0;k<number_of_components;k++)
											{
												DEALLOCATE((*nodal_value_component_values)[k]);
											}
										}
										DEALLOCATE(*nodal_value_component_values);
										nodal_value_component_values++;
									}
								}
							}
							DEALLOCATE(component_values);
						}
						/* reset all the nodal values for the <fe_field> on the element */
						i=0;
						while (return_code&&(i<number_of_saved_element_field_nodes))
						{
							if (return_code=Cmiss_variable_nodal_value_set_type(
								variable_temp,fe_variable,element_field_nodes[i],
								FE_NODAL_UNKNOWN,version))
							{
								return_code=Cmiss_variable_set_independent_variable_value(
									fe_variable,variable_temp,
									element_field_saved_nodal_values[i]);
							}
							i++;
						}
					}
					else
					{
						return_code=0;
					}
					/* destroy working computed values for saving current values and
						setting temporary values */
					for (i=0;i<number_of_element_field_nodes;i++)
					{
						if (element_field_saved_nodal_values[i])
						{
							DESTROY(Cmiss_value)(element_field_saved_nodal_values+i);
						}
						if (element_field_specified_nodal_values[i])
						{
							DESTROY(Cmiss_value)(element_field_specified_nodal_values+i);
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
			/* get rid of working storage */
			DESTROY(Cmiss_variable)(&variable_temp);
			DEALLOCATE(element_field_specified_nodal_values);
			DEALLOCATE(element_field_saved_nodal_values);
			DEALLOCATE(element_field_number_of_nodal_values);
			DEALLOCATE(element_field_number_of_specified_nodal_values);
			DEALLOCATE(element_field_nodal_value_offsets);
		}
		if (return_code&&number_of_element_field_nodes_address&&
			element_field_nodes_address)
		{
			*number_of_element_field_nodes_address=number_of_element_field_nodes;
			*element_field_nodes_address=element_field_nodes;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"nodal_value_calculate_component_values.  "
			"Invalid argument(s).  %p %p %p %d %d %p %p %d %d %p %p %p %p %p",
			fe_variable,fe_field,fe_region,number_of_components,component_number,
			element,node,nodal_value_type,version,
			number_of_element_field_nodes_address,element_field_nodes_address,
			element_field_values,number_of_nodal_values_address,
			component_values_address);
	}
	LEAVE;

	return (return_code);
} /* nodal_value_calculate_component_values */

static int calculate_monomial_derivative_values(int number_of_xi_coordinates,
	int *xi_orders,int *xi_derivative_orders,FE_value *xi_coordinates,
	FE_value *monomial_derivative_values)
/*******************************************************************************
LAST MODIFIED : 5 April 2003

DESCRIPTION :
For the specified monomials (<number_of_xi_coordinates> and order<=<xi_orders>
for each xi), calculates there derivatives (<xi_derivative_orders>) at
<xi_coordinates> and stores in <monomial_derivative_values>.  Expects the memory
to already be allocated for the <monomial_derivative_values>.
NB.  xi_1 is varying slowest (xi_n fastest)
==============================================================================*/
{
	FE_value *temp_value,*value,xi,*xi_coordinate,xi_power;
	int derivative_order,i,j,k,order,number_of_values,return_code,
		*xi_derivative_order,*xi_order,zero_derivative;

	ENTER(calculate_monomial_derivative_values);
	return_code=0;
	if ((0<number_of_xi_coordinates)&&(xi_order=xi_orders)&&
		(xi_derivative_order=xi_derivative_orders)&&(xi_coordinate=xi_coordinates)&&
		monomial_derivative_values)
	{
		/* check for zero derivative */
		zero_derivative=0;
		number_of_values=1;
		for (i=number_of_xi_coordinates;i>0;i--)
		{
			order= *xi_order;
			if (*xi_derivative_order>order)
			{
				zero_derivative=1;
			}
			number_of_values *= order+1;
			xi_derivative_order++;
			xi_order++;
		}
		value=monomial_derivative_values;
		if (zero_derivative)
		{
			/* zero derivative */
			for (k=number_of_values;k>0;k--)
			{
				*value=(FE_value)0;
				value++;
			}
		}
		else
		{
			xi_order=xi_orders;
			xi_derivative_order=xi_derivative_orders;
			*value=1;
			number_of_values=1;
			for (i=number_of_xi_coordinates;i>0;i--)
			{
				xi= *xi_coordinate;
				xi_coordinate++;
				derivative_order= *xi_derivative_order;
				xi_derivative_order++;
				order= *xi_order;
				xi_order++;
				if (0<derivative_order)
				{
					xi_power=1;
					for (j=derivative_order;j>1;j--)
					{
						xi_power *= (FE_value)j;
					}
					value += number_of_values*(derivative_order-1);
					for (j=derivative_order;j<=order;j++)
					{
						temp_value=monomial_derivative_values;
						for (k=number_of_values;k>0;k--)
						{
							value++;
							*value=(*temp_value)*xi_power;
							temp_value++;
						}
						xi_power *= xi*(float)(j+1)/(float)(j-derivative_order+1);
					}
					temp_value=monomial_derivative_values;
					for (k=derivative_order*number_of_values;k>0;k--)
					{
						*temp_value=0;
						temp_value++;
					}
				}
				else
				{
					xi_power=xi;
					for (j=order;j>0;j--)
					{
						temp_value=monomial_derivative_values;
						for (k=number_of_values;k>0;k--)
						{
							value++;
							*value=(*temp_value)*xi_power;
							temp_value++;
						}
						xi_power *= xi;
					}
				}
				number_of_values *= (order+1);
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"calculate_monomial_derivative_values.  "
			"Invalid argument(s).  %d %p %p %p %p",number_of_xi_coordinates,xi_orders,
			xi_derivative_orders,xi_coordinates,monomial_derivative_values);
	}
	LEAVE;

	return (return_code);
} /* calculate_monomial_derivative_values */

enum Cmiss_variable_evaluate_derivative_finite_element_type
{
	ELEMENT_XI_TYPE,
	NODAL_VALUE_TYPE,
	ZERO_DERIVATIVE_TYPE
}; /* enum Cmiss_variable_evaluate_derivative_finite_element_type */

#if defined (OLD_CODE)
/*???DB.  Changing from fe_value_matrix to matrix and to storing the blocks
	within fe_value_matrix as separate matrices */
static START_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	finite_element)
/*******************************************************************************
LAST MODIFIED : 11 July 2003

DESCRIPTION :
???DB.  Only valid for monomial standard basis and nodal value based.  This is
	enforced by <FE_element_field_values_get_monomial_component_info> and
	<FE_element_field_values_get_component_values>.
==============================================================================*/
{
	enum Cmiss_variable_evaluate_derivative_finite_element_type
		*independent_variables_type;
	enum FE_nodal_value_type independent_value_type;
	FE_value component_value,**component_values,**derivative_component_values,
		****independent_variables_component_values,*matrix_value,*matrix_values,
		*monomial_derivative_values,***nodal_value_component_values,
		*value_address_1,*value_address_2,*xi;
	int carry,component_number,**component_monomial_info,
		*derivative_independent_values,*derivative_independent_variables,
		*derivative_independent_variables_mapping,derivative_number_of_values,
		derivative_order,i,independent_variables_index,*independent_variables_copy,
		*independent_variables_number_of_values,independent_version,j,k,
		nodal_value_order,number_of_columns,number_of_columns_save,
		number_of_components,number_of_element_field_nodes,number_of_values,
		number_of_xi,*numbers_of_component_values,*xi_derivative_orders,
		zero_derivative;
	Cmiss_value_id value_type;
	Cmiss_variable_id independent_fe_variable,independent_variable,
		*local_independent_variables,variable_temp;
	struct Cmiss_variable_finite_element_type_specific_data *data;
	struct FE_element *element;
	struct FE_element_field_values *element_field_values;
	struct FE_field *fe_field;
	struct FE_node **element_field_nodes,*independent_node;

	data=(struct Cmiss_variable_finite_element_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data,return_code,0)
	{
		/* check that defined.  Not defined if node is specified */
		if ((fe_field=data->field)&&(element=data->element)&&
			(0<(number_of_xi=data->dimension))&&(xi=data->xi))
		{
			/* set up temporary storage */
			element_field_values=CREATE(FE_element_field_values)();
			value_type=CREATE(Cmiss_value)();
			variable_temp=CREATE(Cmiss_variable)(
				(struct Cmiss_variable_package *)NULL,"temp");
			ALLOCATE(independent_variables_number_of_values,int,order);
			ALLOCATE(independent_variables_type,
				enum Cmiss_variable_evaluate_derivative_finite_element_type,order);
			ALLOCATE(independent_variables_copy,int,order);
			ALLOCATE(independent_variables_component_values,FE_value ***,order);
			ALLOCATE(derivative_independent_variables,int,order+1);
			ALLOCATE(derivative_independent_values,int,order+1);
			ALLOCATE(derivative_independent_variables_mapping,int,order+1);
			ALLOCATE(xi_derivative_orders,int,number_of_xi);
			if (value_type&&variable_temp&&element_field_values&&
				independent_variables_number_of_values&&independent_variables_type&&
				independent_variables_copy&&independent_variables_component_values&&
				derivative_independent_variables&&derivative_independent_values&&
				derivative_independent_variables_mapping&&xi_derivative_orders)
			{
				/* get number of rows in derivative matrix (<number_of_components>) and
					check that <component_number> is valid */
				component_number=data->component_number;
				if (Cmiss_variable_get_value_type(variable,value_type))
				{
					number_of_components=0;
					if (CMISS_VALUE_IS_TYPE(FE_value)(value_type))
					{
						number_of_components=1;
						return_code=1;
					}
					else if (CMISS_VALUE_IS_TYPE(FE_value_vector)(value_type))
					{
						return_code=Cmiss_value_FE_value_vector_get_type(value_type,
							&number_of_components,(FE_value **)NULL);
					}
					if (return_code)
					{
						return_code=0;
						if (0<number_of_components)
						{
							if ((0<=component_number)&&
								(component_number<number_of_components))
							{
								number_of_components=1;
								return_code=1;
							}
							else
							{
								if (-1==component_number)
								{
									return_code=1;
								}
							}
						}
					}
				}
				/* loop over independent variables building up derivative matrix */
				number_of_columns=0;
				matrix_values=(FE_value *)NULL;
				independent_variables_index=0;
				/* temporary storage that is built up along with the derivative
					matrix */
				number_of_element_field_nodes=0;
				element_field_nodes=(struct FE_node **)NULL;
				component_monomial_info=(int **)NULL;
				component_values=(FE_value **)NULL;
				numbers_of_component_values=(int *)NULL;
				monomial_derivative_values=(FE_value *)NULL;
				while (return_code&&(independent_variables_index<order))
				{
					independent_variable=
						independent_variables[independent_variables_index];
					independent_variables_copy[independent_variables_index]=0;
					independent_variables_number_of_values[independent_variables_index]=0;
					/* calculate component values and monomial information for
						derivatives */
					number_of_values=0;
					if (CMISS_VARIABLE_IS_TYPE(element_xi)(independent_variable))
					{
						/*???DB.  Currently element/xi can not be for a particular field */
						/* differentiating with respect to element xi */
						independent_variables_type[independent_variables_index]=
							ELEMENT_XI_TYPE;
						if (!component_values||!component_monomial_info)
						{
							if ((return_code=clear_FE_element_field_values(
								element_field_values))&&
								(return_code=calculate_FE_element_field_values(element,
								fe_field,data->time,(char)0,element_field_values,
								(struct FE_element *)NULL)))
							{
								if (!component_values)
								{
									return_code=extract_component_values(element_field_values,
										number_of_components,component_number,
										&numbers_of_component_values,&component_values);
								}
								if (!component_monomial_info)
								{
									return_code=extract_component_monomial_info(
										element_field_values,number_of_components,component_number,
										number_of_xi,&numbers_of_component_values,
										&component_monomial_info,&monomial_derivative_values);
								}
							}
						}
						/* use the element/xi for the field */
						number_of_values=number_of_xi;
					}
					else if (CMISS_VARIABLE_IS_TYPE(nodal_value)(independent_variable))
					{
						if (return_code=Cmiss_variable_nodal_value_get_type(
							independent_variable,&independent_fe_variable,&independent_node,
							&independent_value_type,&independent_version))
						{
							if (!independent_fe_variable||Cmiss_variable_same_variable(
								variable,independent_fe_variable))
							{
								/* differentiating with respect own nodal value */
								independent_variables_type[independent_variables_index]=
									NODAL_VALUE_TYPE;
								i=0;
								while ((i<independent_variables_index)&&((NODAL_VALUE_TYPE!=
									independent_variables_type[i])||
									!Cmiss_variable_same_variable(independent_variable,
									independent_variables[i])))
								{
									i++;
								}
								if (i<independent_variables_index)
								{
									/* reuse previously calculated mappings */
									independent_variables_copy[independent_variables_index]=1;
									independent_variables_component_values[
										independent_variables_index]=
										independent_variables_component_values[i];
									number_of_values=independent_variables_number_of_values[i];
								}
								else
								{
									if (return_code=nodal_value_calculate_component_values(
										variable,fe_field,component_number,data->time,element,
										independent_node,independent_value_type,independent_version,
										&number_of_element_field_nodes,&element_field_nodes,
										element_field_values,&number_of_values,
										&numbers_of_component_values,
										independent_variables_component_values+
										independent_variables_index))
									{
										if (!component_monomial_info)
										{
											return_code=extract_component_monomial_info(
												element_field_values,number_of_components,
												component_number,number_of_xi,
												&numbers_of_component_values,&component_monomial_info,
												&monomial_derivative_values);
										}
									}
								}
							}
							else
							{
								independent_variables_type[independent_variables_index]=
									ZERO_DERIVATIVE_TYPE;
							}
						}
					}
					else
					{
						independent_variables_type[independent_variables_index]=
							ZERO_DERIVATIVE_TYPE;
					}
					if (return_code&&(ZERO_DERIVATIVE_TYPE==independent_variables_type[
						independent_variables_index]))
					{
						if (return_code=Cmiss_variable_get_value_type(
							independent_variable,
							value_type))
						{
							if (CMISS_VALUE_IS_TYPE(FE_value)(value_type))
							{
								number_of_values=1;
							}
							else if (CMISS_VALUE_IS_TYPE(FE_value_vector)(value_type))
							{
								return_code=Cmiss_value_FE_value_vector_get_type(value_type,
									&number_of_values,(FE_value **)NULL);
							}
							else if (CMISS_VALUE_IS_TYPE(element_xi)(value_type))
							{
								/* use the element/xi for the field */
								number_of_values=number_of_xi;
							}
							else
							{
								return_code=0;
							}
						}
					}
					if (return_code&&(0<number_of_values))
					{
						independent_variables_number_of_values[
							independent_variables_index]=number_of_values;
						number_of_columns_save=number_of_columns;
						number_of_columns=number_of_columns*(number_of_values+1)+
							number_of_values;
						if (REALLOCATE(matrix_value,matrix_values,FE_value,
							number_of_components*number_of_columns))
						{
							matrix_values=matrix_value;
						}
						else
						{
							return_code=0;
						}
						if (return_code)
						{
							/* relocate old matrix values and zero new matrix values */
							matrix_value=matrix_values+
								(number_of_components*number_of_columns);
							value_address_1=matrix_values+
								(number_of_components*number_of_columns_save);
							for (i=number_of_components;i>1;i--)
							{
								for (j=number_of_columns-number_of_columns_save;j>0;j--)
								{
									matrix_value--;
									*matrix_value=0;
								}
								for (j=number_of_columns_save;j>0;j--)
								{
									matrix_value--;
									value_address_1--;
									*matrix_value= *value_address_1;
								}
							}
							for (j=number_of_columns-number_of_columns_save;j>0;j--)
							{
								matrix_value--;
								*matrix_value=0;
							}
							/* loop over new derivative blocks and calculate */
							matrix_value=matrix_values+number_of_columns_save;
							for (i=0;i<=order;i++)
							{
								derivative_independent_variables[i]=0;
							}
							derivative_independent_variables[independent_variables_index]=1;
							while (return_code&&(0==derivative_independent_variables[
								independent_variables_index+1]))
							{
								/* determine the element/xi and nodal_value orders */
								nodal_value_order=0;
								i=0;
								zero_derivative=0;
								derivative_number_of_values=1;
								derivative_order=0;
								/* zero derivative check.  Doesn't check if the derivative order
									for a particular xi exceeds the order of the polynomial
									(done in <calculate_monomial_derivative_values>) */
									/*???DB.  Could be more efficient */
								/* Also set up mapping from variables involved in derivative to
									all independent variables */
								while (i<=independent_variables_index)
								{
									if (derivative_independent_variables[i])
									{
										switch (independent_variables_type[i])
										{
											case ELEMENT_XI_TYPE:
											{
												/* do nothing (but don't fail) */
											} break;
											case NODAL_VALUE_TYPE:
											{
												nodal_value_order++;
												nodal_value_component_values=
													independent_variables_component_values[i];
											} break;
											case ZERO_DERIVATIVE_TYPE:
											{
												zero_derivative=1;
											} break;
											default:
											{
												return_code=0;
											} break;
										}
										derivative_number_of_values *=
											independent_variables_number_of_values[i];
										derivative_independent_variables_mapping[derivative_order]=
											i;
										derivative_order++;
									}
									i++;
								}
								if ((nodal_value_order>1)||(zero_derivative))
								{
									matrix_value += derivative_number_of_values;
								}
								else
								{
									for (i=0;i<=derivative_order;i++)
									{
										derivative_independent_values[i]=0;
									}
									/* chose the component values to use for calculating the
										values */
									if (0==nodal_value_order)
									{
										derivative_component_values=component_values;
									}
									else
									{
										derivative_component_values=nodal_value_component_values[0];
									}
									/* loop over the values within the derivative block */
									while (return_code&&
										(0==derivative_independent_values[derivative_order]))
									{
										if (derivative_component_values)
										{
											for (i=0;i<number_of_xi;i++)
											{
												xi_derivative_orders[i]=0;
											}
											for (i=0;i<derivative_order;i++)
											{
												j=derivative_independent_variables_mapping[i];
												if (ELEMENT_XI_TYPE==independent_variables_type[
													derivative_independent_variables_mapping[i]])
												{
													xi_derivative_orders[
														derivative_independent_values[i]]++;
												}
											}
											/* loop over components */
											i=0;
											while (return_code&&(i<number_of_components))
											{
												if (return_code=calculate_monomial_derivative_values(
													number_of_xi,component_monomial_info[i]+1,
													xi_derivative_orders,xi,monomial_derivative_values))
												{
													/* calculate the derivative */
													component_value=0;
													value_address_1=monomial_derivative_values;
													value_address_2=derivative_component_values[i];
													for (j=numbers_of_component_values[i];j>0;j--)
													{
														component_value +=
															(*value_address_1)*(*value_address_2);
														value_address_1++;
														value_address_2++;
													}
													matrix_value[i*number_of_columns]=component_value;
												}
												i++;
											}
										}
										/* step to next value within derivative block */
										matrix_value++;
										i=0;
										carry=1;
										do
										{
											derivative_independent_values[i]++;
											j=derivative_independent_variables_mapping[i];
											if (derivative_independent_values[i]>=
												independent_variables_number_of_values[j])
											{
												derivative_independent_values[i]=0;
											}
											else
											{
												carry=0;
											}
											if (NODAL_VALUE_TYPE==independent_variables_type[j])
											{
												/* update the component values to use for calculating
													the values */
												derivative_component_values=
													nodal_value_component_values[
													derivative_independent_values[i]];
											}
											i++;
										} while ((i<derivative_order)&&carry);
										if (carry)
										{
											derivative_independent_values[derivative_order]++;
										}
									}
								}
								/* step to next derivative block */
								i=0;
								while ((i<order)&&derivative_independent_variables[i])
								{
									derivative_independent_variables[i]=0;
									i++;
								}
								derivative_independent_variables[i]=1;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_variable_derivative_evaluate_derivative_type_specific.  "
								"Could not reallocate <matrix_values>");
							DEALLOCATE(matrix_values);
							return_code=0;
						}
					}
					else
					{
						return_code=0;
					}
					/* step to next independent variable */
					independent_variables_index++;
				}
				if (return_code)
				{
					/* set up the result (<value>) */
					if (Cmiss_value_FE_value_matrix_set_type(value_type,
						number_of_components,number_of_columns,matrix_values))
					{
						if (ALLOCATE(local_independent_variables,Cmiss_variable_id,order))
						{
							for (i=0;i<order;i++)
							{
								local_independent_variables[i]=independent_variables[i];
							}
							if (return_code=Cmiss_value_derivative_matrix_set_type(value,
								variable,order,local_independent_variables,value_type))
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
								"Cmiss_variable_finite_element_evaluate_derivative_type_specific.  "
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
				else
				{
					DEALLOCATE(matrix_values);
				}
				/* remove temporary storage that was built up along with the derivative
					matrix */
				DEALLOCATE(numbers_of_component_values);
				if (component_values)
				{
					for (i=0;i<number_of_components;i++)
					{
						DEALLOCATE(component_values[i]);
					}
					DEALLOCATE(component_values);
				}
				if (component_monomial_info)
				{
					for (i=0;i<number_of_components;i++)
					{
						DEALLOCATE(component_monomial_info[i]);
					}
					DEALLOCATE(component_monomial_info);
				}
				if (element_field_nodes)
				{
					for (i=0;i<number_of_element_field_nodes;i++)
					{
						DEACCESS(FE_node)(element_field_nodes+i);
					}
					DEALLOCATE(element_field_nodes);
				}
				for (i=0;i<independent_variables_index;i++)
				{
					if ((NODAL_VALUE_TYPE==independent_variables_type[i])&&
						!independent_variables_copy[i])
					{
						for (j=0;j<independent_variables_number_of_values[i];j++)
						{
							if (independent_variables_component_values[i][j])
							{
								for (k=0;k<number_of_components;k++)
								{
									DEALLOCATE(independent_variables_component_values[i][j][k]);
								}
								DEALLOCATE(independent_variables_component_values[i][j]);
							}
						}
						DEALLOCATE(independent_variables_component_values[i]);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_finite_element_evaluate_derivative_type_specific.  "
					"Could not allocate working storage");
			}
			/* remove temporary storage */
			DEALLOCATE(xi_derivative_orders);
			DEALLOCATE(derivative_independent_variables_mapping);
			DEALLOCATE(derivative_independent_values);
			DEALLOCATE(derivative_independent_variables);
			DEALLOCATE(independent_variables_component_values);
			DEALLOCATE(independent_variables_type);
			DEALLOCATE(independent_variables_number_of_values);
			DEALLOCATE(independent_variables_copy);
			if (element_field_values)
			{
				DESTROY(FE_element_field_values)(&element_field_values);
			}
			if (variable_temp)
			{
				DESTROY(Cmiss_variable)(&variable_temp);
			}
			if (!return_code&&value_type)
			{
				DESTROY(Cmiss_value)(&value_type);
			}
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(finite_element)
#endif /* defined (OLD_CODE) */

static START_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	finite_element)
/*******************************************************************************
LAST MODIFIED : 27 June 2003

DESCRIPTION :
???DB.  Only valid for monomial standard basis and nodal value based.  This is
	enforced by <FE_element_field_values_get_monomial_component_info> and
	<FE_element_field_values_get_component_values>.
==============================================================================*/
{
	Cmiss_value_id *matrices,*matrix,value_type;
	Cmiss_variable_id independent_fe_variable,independent_variable,
		*local_independent_variables;
	double component_value;
	enum Cmiss_variable_evaluate_derivative_finite_element_type
		*independent_variables_type;
	enum FE_nodal_value_type independent_value_type;
	FE_value **component_values,**derivative_component_values,
		****independent_variables_component_values,*monomial_derivative_values,
		***nodal_value_component_values,*value_address_1,*value_address_2,*xi;
	int carry,column_number,component_number,**component_monomial_info,
		*derivative_independent_values,*derivative_independent_variables,
		*derivative_independent_variables_mapping,derivative_number_of_values,
		derivative_order,i,independent_variables_index,*independent_variables_copy,
		*independent_variables_number_of_values,independent_version,j,k,
		nodal_value_order,number_of_components,number_of_element_field_nodes,
		number_of_matrices,number_of_values,number_of_xi,
		*numbers_of_component_values,*xi_derivative_orders,zero_derivative;
	struct Cmiss_variable_finite_element_type_specific_data *data;
	struct FE_element *element;
	struct FE_element_field_values *element_field_values;
	struct FE_field *fe_field;
	struct FE_node **element_field_nodes,*independent_node;
	struct Matrix *matrix_local;

	data=(struct Cmiss_variable_finite_element_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data,return_code,0)
	{
		/* check that defined.  Not defined if node is specified */
		if ((fe_field=data->field)&&(element=data->element)&&
			(0<(number_of_xi=data->dimension))&&(xi=data->xi))
		{
			/* set up temporary storage */
			element_field_values=CREATE(FE_element_field_values)();
			value_type=CREATE(Cmiss_value)();
			ALLOCATE(independent_variables_number_of_values,int,order);
			ALLOCATE(independent_variables_type,
				enum Cmiss_variable_evaluate_derivative_finite_element_type,order);
			ALLOCATE(independent_variables_copy,int,order);
			ALLOCATE(independent_variables_component_values,FE_value ***,order);
			ALLOCATE(derivative_independent_variables,int,order+1);
			ALLOCATE(derivative_independent_values,int,order+1);
			ALLOCATE(derivative_independent_variables_mapping,int,order+1);
			ALLOCATE(xi_derivative_orders,int,number_of_xi);
			if (value_type&&element_field_values&&
				independent_variables_number_of_values&&independent_variables_type&&
				independent_variables_copy&&independent_variables_component_values&&
				derivative_independent_variables&&derivative_independent_values&&
				derivative_independent_variables_mapping&&xi_derivative_orders)
			{
				/* get number of rows in derivative matrix (<number_of_components>) and
					check that <component_number> is valid */
				component_number=data->component_number;
				if (Cmiss_variable_get_value_type(variable,value_type))
				{
					number_of_components=0;
					if (CMISS_VALUE_IS_TYPE(FE_value)(value_type))
					{
						number_of_components=1;
						return_code=1;
					}
					else if (CMISS_VALUE_IS_TYPE(FE_value_vector)(value_type))
					{
						return_code=Cmiss_value_FE_value_vector_get_type(value_type,
							&number_of_components,(FE_value **)NULL);
					}
					if (return_code)
					{
						return_code=0;
						if (0<number_of_components)
						{
							if ((0<=component_number)&&
								(component_number<number_of_components))
							{
								number_of_components=1;
								return_code=1;
							}
							else
							{
								if (-1==component_number)
								{
									return_code=1;
								}
							}
						}
					}
				}
				/* loop over independent variables building up derivative matrix */
				independent_variables_index=0;
				/* temporary storage that is built up along with the derivative
					matrix */
				number_of_element_field_nodes=0;
				element_field_nodes=(struct FE_node **)NULL;
				component_monomial_info=(int **)NULL;
				component_values=(FE_value **)NULL;
				numbers_of_component_values=(int *)NULL;
				monomial_derivative_values=(FE_value *)NULL;
				matrices=(Cmiss_value_id *)NULL;
				number_of_matrices=0;
				while (return_code&&(independent_variables_index<order))
				{
					independent_variable=
						independent_variables[independent_variables_index];
					independent_variables_copy[independent_variables_index]=0;
					independent_variables_number_of_values[independent_variables_index]=0;
					/* calculate component values and monomial information for
						derivatives */
					number_of_values=0;
					if (CMISS_VARIABLE_IS_TYPE(element_xi)(independent_variable))
					{
						/*???DB.  Currently element/xi can not be for a particular field */
						/* differentiating with respect to element xi */
						independent_variables_type[independent_variables_index]=
							ELEMENT_XI_TYPE;
						if (!component_values||!component_monomial_info)
						{
							if ((return_code=clear_FE_element_field_values(
								element_field_values))&&
								(return_code=calculate_FE_element_field_values(element,
								fe_field,data->time,(char)0,element_field_values,
								(struct FE_element *)NULL)))
							{
								if (!component_values)
								{
									return_code=extract_component_values(element_field_values,
										number_of_components,component_number,
										&numbers_of_component_values,&component_values);
								}
								if (!component_monomial_info)
								{
									return_code=extract_component_monomial_info(
										element_field_values,number_of_components,component_number,
										number_of_xi,&numbers_of_component_values,
										&component_monomial_info,&monomial_derivative_values);
								}
							}
						}
						/* use the element/xi for the field */
						number_of_values=number_of_xi;
					}
					else if (CMISS_VARIABLE_IS_TYPE(nodal_value)(independent_variable))
					{
						if (return_code=Cmiss_variable_nodal_value_get_type(
							independent_variable,&independent_fe_variable,&independent_node,
							&independent_value_type,&independent_version))
						{
							if (!independent_fe_variable||Cmiss_variable_same_variable(
								variable,independent_fe_variable))
							{
								/* differentiating with respect own nodal value */
								independent_variables_type[independent_variables_index]=
									NODAL_VALUE_TYPE;
								i=0;
								while ((i<independent_variables_index)&&((NODAL_VALUE_TYPE!=
									independent_variables_type[i])||
									!Cmiss_variable_same_variable(independent_variable,
									independent_variables[i])))
								{
									i++;
								}
								if (i<independent_variables_index)
								{
									/* reuse previously calculated mappings */
									independent_variables_copy[independent_variables_index]=1;
									independent_variables_component_values[
										independent_variables_index]=
										independent_variables_component_values[i];
									number_of_values=independent_variables_number_of_values[i];
								}
								else
								{
									if (return_code=nodal_value_calculate_component_values(
										variable,fe_field,component_number,data->time,element,
										independent_node,independent_value_type,independent_version,
										&number_of_element_field_nodes,&element_field_nodes,
										element_field_values,&number_of_values,
										&numbers_of_component_values,
										independent_variables_component_values+
										independent_variables_index))
									{
										if (!component_monomial_info)
										{
											return_code=extract_component_monomial_info(
												element_field_values,number_of_components,
												component_number,number_of_xi,
												&numbers_of_component_values,&component_monomial_info,
												&monomial_derivative_values);
										}
									}
								}
							}
							else
							{
								independent_variables_type[independent_variables_index]=
									ZERO_DERIVATIVE_TYPE;
							}
						}
					}
					else
					{
						independent_variables_type[independent_variables_index]=
							ZERO_DERIVATIVE_TYPE;
					}
					if (return_code&&(ZERO_DERIVATIVE_TYPE==independent_variables_type[
						independent_variables_index]))
					{
						if (return_code=Cmiss_variable_get_value_type(
							independent_variable,
							value_type))
						{
							if (CMISS_VALUE_IS_TYPE(FE_value)(value_type))
							{
								number_of_values=1;
							}
							else if (CMISS_VALUE_IS_TYPE(FE_value_vector)(value_type))
							{
								return_code=Cmiss_value_FE_value_vector_get_type(value_type,
									&number_of_values,(FE_value **)NULL);
							}
							else if (CMISS_VALUE_IS_TYPE(element_xi)(value_type))
							{
								/* use the element/xi for the field */
								number_of_values=number_of_xi;
							}
							else
							{
								return_code=0;
							}
						}
					}
					if (return_code&&(0<number_of_values))
					{
						independent_variables_number_of_values[
							independent_variables_index]=number_of_values;
						if (REALLOCATE(matrix,matrices,Cmiss_value_id,
							2*number_of_matrices+1))
						{
							matrices=matrix;
							matrix += number_of_matrices;
							/* loop over new derivative blocks and calculate */
							for (i=0;i<=order;i++)
							{
								derivative_independent_variables[i]=0;
							}
							derivative_independent_variables[independent_variables_index]=1;
							while (return_code&&(0==derivative_independent_variables[
								independent_variables_index+1]))
							{
								/* determine the element/xi and nodal_value orders */
								nodal_value_order=0;
								i=0;
								zero_derivative=0;
								derivative_number_of_values=1;
								derivative_order=0;
								/* zero derivative check.  Doesn't check if the derivative order
									for a particular xi exceeds the order of the polynomial
									(done in <calculate_monomial_derivative_values>) */
									/*???DB.  Could be more efficient */
								/* Also set up mapping from variables involved in derivative to
									all independent variables */
								while (i<=independent_variables_index)
								{
									if (derivative_independent_variables[i])
									{
										switch (independent_variables_type[i])
										{
											case ELEMENT_XI_TYPE:
											{
												/* do nothing (but don't fail) */
											} break;
											case NODAL_VALUE_TYPE:
											{
												nodal_value_order++;
												nodal_value_component_values=
													independent_variables_component_values[i];
											} break;
											case ZERO_DERIVATIVE_TYPE:
											{
												zero_derivative=1;
											} break;
											default:
											{
												return_code=0;
											} break;
										}
										derivative_number_of_values *=
											independent_variables_number_of_values[i];
										derivative_independent_variables_mapping[derivative_order]=
											i;
										derivative_order++;
									}
									i++;
								}
								if (*matrix=CREATE(Cmiss_value)())
								{
									number_of_matrices++;
									if (matrix_local=CREATE(Matrix)("matrix",DENSE,
										number_of_components,derivative_number_of_values))
									{
										if (!(return_code=Cmiss_value_matrix_set_type(*matrix,
											matrix_local)))
										{
											DESTROY(Matrix)(&matrix_local);
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
								if (return_code&&!((nodal_value_order>1)||(zero_derivative)))
								{
									for (i=0;i<=derivative_order;i++)
									{
										derivative_independent_values[i]=0;
									}
									/* chose the component values to use for calculating the
										values */
									if (0==nodal_value_order)
									{
										derivative_component_values=component_values;
									}
									else
									{
										derivative_component_values=nodal_value_component_values[0];
									}
									/* loop over the values within the derivative block */
									column_number=1;
									while (return_code&&
										(0==derivative_independent_values[derivative_order]))
									{
										if (derivative_component_values)
										{
											for (i=0;i<number_of_xi;i++)
											{
												xi_derivative_orders[i]=0;
											}
											for (i=0;i<derivative_order;i++)
											{
												if (ELEMENT_XI_TYPE==independent_variables_type[
													derivative_independent_variables_mapping[i]])
												{
													xi_derivative_orders[
														derivative_independent_values[i]]++;
												}
											}
											/* loop over components */
											i=0;
											while (return_code&&(i<number_of_components))
											{
												if (return_code=calculate_monomial_derivative_values(
													number_of_xi,component_monomial_info[i]+1,
													xi_derivative_orders,xi,monomial_derivative_values))
												{
													/* calculate the derivative */
													component_value=(double)0;
													value_address_1=monomial_derivative_values;
													value_address_2=derivative_component_values[i];
													for (j=numbers_of_component_values[i];j>0;j--)
													{
														component_value +=
															(double)(*value_address_1)*
															(double)(*value_address_2);
														value_address_1++;
														value_address_2++;
													}
													return_code=Cmiss_value_matrix_set_value(*matrix,
														i+1,column_number,(Matrix_value)component_value);
												}
												i++;
											}
										}
										/* step to next value within derivative block */
										column_number++;
										i=0;
										carry=1;
										do
										{
											derivative_independent_values[i]++;
											j=derivative_independent_variables_mapping[i];
											if (derivative_independent_values[i]>=
												independent_variables_number_of_values[j])
											{
												derivative_independent_values[i]=0;
											}
											else
											{
												carry=0;
											}
											if (NODAL_VALUE_TYPE==independent_variables_type[j])
											{
												/* update the component values to use for calculating
													the values */
												derivative_component_values=
													nodal_value_component_values[
													derivative_independent_values[i]];
											}
											i++;
										} while ((i<derivative_order)&&carry);
										if (carry)
										{
											derivative_independent_values[derivative_order]++;
										}
									}
								}
								/* step to next derivative block */
								matrix++;
								i=0;
								while ((i<order)&&derivative_independent_variables[i])
								{
									derivative_independent_variables[i]=0;
									i++;
								}
								derivative_independent_variables[i]=1;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_variable_finite_element_evaluate_derivative_type_specific.  "
								"Could not reallocate <matrices>");
							return_code=0;
						}
					}
					else
					{
						return_code=0;
					}
					/* step to next independent variable */
					independent_variables_index++;
				}
				if (return_code)
				{
					/* set up the result (<value>) */
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
							"Cmiss_variable_finite_element_evaluate_derivative_type_specific.  "
							"Could not allocate <independent_variable_address>");
						return_code=0;
					}
				}
				if (matrices)
				{
					matrix=matrices;
					i=number_of_matrices;
					while (i>0)
					{
						DESTROY(Cmiss_value)(matrix);
						matrix++;
						i--;
					}
					DEALLOCATE(matrices);
				}
				/* remove temporary storage that was built up along with the derivative
					matrix */
				DEALLOCATE(numbers_of_component_values);
				if (component_values)
				{
					for (i=0;i<number_of_components;i++)
					{
						DEALLOCATE(component_values[i]);
					}
					DEALLOCATE(component_values);
				}
				if (component_monomial_info)
				{
					for (i=0;i<number_of_components;i++)
					{
						DEALLOCATE(component_monomial_info[i]);
					}
					DEALLOCATE(component_monomial_info);
				}
				if (element_field_nodes)
				{
					for (i=0;i<number_of_element_field_nodes;i++)
					{
						DEACCESS(FE_node)(element_field_nodes+i);
					}
					DEALLOCATE(element_field_nodes);
				}
				for (i=0;i<independent_variables_index;i++)
				{
					if ((NODAL_VALUE_TYPE==independent_variables_type[i])&&
						!independent_variables_copy[i])
					{
						for (j=0;j<independent_variables_number_of_values[i];j++)
						{
							if (independent_variables_component_values[i][j])
							{
								for (k=0;k<number_of_components;k++)
								{
									DEALLOCATE(independent_variables_component_values[i][j][k]);
								}
								DEALLOCATE(independent_variables_component_values[i][j]);
							}
						}
						DEALLOCATE(independent_variables_component_values[i]);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_finite_element_evaluate_derivative_type_specific.  "
					"Could not allocate working storage");
			}
			/* remove temporary storage */
			DEALLOCATE(xi_derivative_orders);
			DEALLOCATE(derivative_independent_variables_mapping);
			DEALLOCATE(derivative_independent_values);
			DEALLOCATE(derivative_independent_variables);
			DEALLOCATE(independent_variables_component_values);
			DEALLOCATE(independent_variables_type);
			DEALLOCATE(independent_variables_number_of_values);
			DEALLOCATE(independent_variables_copy);
			if (element_field_values)
			{
				DESTROY(FE_element_field_values)(&element_field_values);
			}
			if (value_type)
			{
				DESTROY(Cmiss_value)(&value_type);
			}
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(finite_element)

static START_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(finite_element)
{
	FE_value *fe_value_vector;
	int number_of_fe_values;
	struct Cmiss_variable_finite_element_type_specific_data *data;

	data=(struct Cmiss_variable_finite_element_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->field),return_code,0)
	{
		if ((data->node)||((data->element)&&(data->xi)))
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
						return_code=Cmiss_value_FE_value_set_type(value,
							*fe_value_vector);
					}
					else
					{
						return_code=Cmiss_value_FE_value_vector_set_type(value,
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
					"Cmiss_variable_finite_element_evaluate_type_specific.  "
					"Could not ALLOCATE <fe_value_vector>.  %d",number_of_fe_values);
			}
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(finite_element)

static
	START_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	finite_element)
{
	FE_value *xi;
	int dimension,i;
	Cmiss_variable_id fe_variable;
	struct Cmiss_variable_finite_element_type_specific_data *data;
	struct Cmiss_variable_value *variable_value;

	data=(struct Cmiss_variable_finite_element_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data,return_code,0)
	{
		if (CMISS_VARIABLE_IS_TYPE(element_xi)(independent_variable))
		{
			if ((data->element)&&(data->xi))
			{
				dimension=data->dimension;
				if ((0<dimension)&&ALLOCATE(xi,FE_value,dimension))
				{
					if (Cmiss_value_element_xi_set_type(value,dimension,data->element,xi))
					{
						for (i=0;i<dimension;i++)
						{
							xi[i]=(data->xi)[i];
						}
						return_code=1;
					}
					else
					{
						DEALLOCATE(xi);
					}
				}
			}
		}
		else if (CMISS_VARIABLE_IS_TYPE(nodal_value)(independent_variable))
		{
			if (Cmiss_variable_nodal_value_get_type(independent_variable,
				&fe_variable,(struct FE_node **)NULL,(enum FE_nodal_value_type *)NULL,
				(int *)NULL)&&Cmiss_variable_same_variable(dependent_variable,
				fe_variable))
			{
				if (variable_value=CREATE(Cmiss_variable_value)(independent_variable,
					value))
				{
					return_code=Cmiss_variable_evaluate(variable_value,
						(struct LIST(Cmiss_variable_value) *)NULL);
					DESTROY(Cmiss_variable_value)(&variable_value);
				}
			}
		}
	}
}
END_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	finite_element)

static
	START_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	finite_element)
{
	FE_value *value_xi,*xi;
	int dimension,i;
	Cmiss_variable_id fe_variable;
	struct Cmiss_variable_finite_element_type_specific_data *data;
	struct FE_element *element;

	data=(struct Cmiss_variable_finite_element_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data,return_code,0)
	{
		if (CMISS_VARIABLE_IS_TYPE(element_xi)(independent_variable))
		{
			if (Cmiss_value_element_xi_get_type(value,&dimension,&element,&value_xi))
			{
				if ((0<dimension)&&value_xi)
				{
					if (ALLOCATE(xi,FE_value,dimension))
					{
						for (i=0;i<dimension;i++)
						{
							xi[i]=value_xi[i];
						}
						if (element)
						{
							ACCESS(FE_element)(element);
						}
						return_code=1;
					}
				}
				else
				{
					if (element)
					{
						ACCESS(FE_element)(element);
					}
					xi=(FE_value *)NULL;
					return_code=1;
				}
				if (return_code)
				{
					if (Cmiss_value_element_xi_set_type(value,data->dimension,
						data->element,data->xi))
					{
						if (data->element)
						{
							DEACCESS(FE_element)(&(data->element));
						}
						data->dimension=dimension;
						data->element=element;
						data->xi=xi;
						return_code=1;
					}
					else
					{
						if (xi)
						{
							DEALLOCATE(xi);
						}
						if (element)
						{
							DEACCESS(FE_element)(&element);
						}
						return_code=0;
					}
				}
			}
		}
		else if (CMISS_VARIABLE_IS_TYPE(nodal_value)(independent_variable))
		{
			if (Cmiss_variable_nodal_value_get_type(independent_variable,
				&fe_variable,(struct FE_node **)NULL,(enum FE_nodal_value_type *)NULL,
				(int *)NULL)&&Cmiss_variable_same_variable(dependent_variable,
				fe_variable))
			{
				return_code=Cmiss_variable_get_set_independent_variable_value(
					independent_variable,independent_variable,value);
			}
		}
	}
}
END_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(finite_element)

static START_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(
	finite_element)
{
	int number_of_fe_values;
	struct Cmiss_variable_finite_element_type_specific_data *data;

	data=(struct Cmiss_variable_finite_element_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
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
				return_code=Cmiss_value_FE_value_set_type(type,(FE_value)0);
			}
			else
			{
				return_code=Cmiss_value_FE_value_vector_set_type(type,
					number_of_fe_values,(FE_value *)NULL);
			}
		}
	}
}
END_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(finite_element)

static START_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(finite_element)
{
	struct Cmiss_variable_finite_element_type_specific_data *data;

	data=(struct Cmiss_variable_finite_element_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->field),return_code,0)
	{
		if ((data->node)||((data->element)&&(data->xi)))
		{
			return_code=1;
		}
	}
}
END_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(finite_element)

static
	START_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	finite_element)
{
	Cmiss_variable_id fe_variable;

	if (CMISS_VARIABLE_IS_TYPE(element_xi)(independent_variable))
	{
		return_code=1;
	}
	else if (CMISS_VARIABLE_IS_TYPE(nodal_value)(independent_variable))
	{
		if (Cmiss_variable_nodal_value_get_type(independent_variable,
			&fe_variable,(struct FE_node **)NULL,(enum FE_nodal_value_type *)NULL,
			(int *)NULL)&&Cmiss_variable_same_variable(dependent_variable,
			fe_variable))
		{
			return_code=1;
		}
	}
}
END_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	finite_element)

static START_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(finite_element)
{
	struct Cmiss_variable_finite_element_type_specific_data *data;
	struct FE_region *region;

	data=(struct Cmiss_variable_finite_element_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->field),return_code,0)
	{
		/* check the field can be destroyed */
			/*???DB.  From Computed_field_finite_element_not_in_use */
		if ((region=FE_field_get_FE_region(data->field))&&
			(FE_region_contains_FE_field(region,data->field)))
		{
			if (2==FE_field_get_access_count(data->field))
			{
				return_code=1;
			}
		}
		else
		{
			return_code=1;
		}
	}
}
END_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(finite_element)

static START_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(finite_element)
{
	struct Cmiss_variable_finite_element_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_variable_finite_element_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_1);
	data_2=(struct Cmiss_variable_finite_element_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&(data_1->field)&&data_2&&(data_2->field),return_code,0)
	{
		if ((data_1->field==data_2->field)&&
			((data_1->component_number==data_2->component_number)||
			(-1==data_1->component_number)||(-1==data_2->component_number)))
		{
			return_code=1;
		}
	}
}
END_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(finite_element)

static START_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(
	finite_element)
{
	struct Cmiss_variable_finite_element_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_variable_finite_element_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_1);
	data_2=(struct Cmiss_variable_finite_element_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&(data_1->field)&&data_2&&(data_2->field),return_code,0)
	{
		if (data_1->field==data_2->field)
		{
			/*???DB.  What about source variables? */
			return_code=1;
		}
	}
}
END_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(finite_element)

static
	START_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	finite_element)
{
	FE_value *value_xi,*xi;
	int dimension,i;
	Cmiss_variable_id fe_variable;
	struct Cmiss_variable_finite_element_type_specific_data *data;
	struct FE_element *element;

	data=(struct Cmiss_variable_finite_element_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data,return_code,0)
	{
		if (CMISS_VARIABLE_IS_TYPE(element_xi)(independent_variable))
		{
			if ((data->element)&&(data->xi)&&
				Cmiss_value_element_xi_get_type(value,&dimension,&element,&value_xi))
			{
				if ((0<dimension)&&ALLOCATE(xi,FE_value,dimension))
				{
					for (i=0;i<dimension;i++)
					{
						xi[i]=value_xi[i];
					}
					DEACCESS(FE_element)(&(data->element));
					data->element=element;
					DEALLOCATE(data->xi);
					data->xi=xi;
					return_code=1;
				}
			}
		}
		else if (CMISS_VARIABLE_IS_TYPE(nodal_value)(independent_variable))
		{
			if (Cmiss_variable_nodal_value_get_type(independent_variable,
				&fe_variable,(struct FE_node **)NULL,(enum FE_nodal_value_type *)NULL,
				(int *)NULL)&&Cmiss_variable_same_variable(dependent_variable,
				fe_variable))
			{
				return_code=Cmiss_variable_set_independent_variable_value(
					independent_variable,independent_variable,value);
			}
		}
	}
}
END_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	finite_element)

static char Cmiss_variable_nodal_value_type_string[]="Nodal_value";

struct Cmiss_variable_nodal_value_type_specific_data
/*******************************************************************************
LAST MODIFIED : 25 March 2003

DESCRIPTION :
==============================================================================*/
{
	enum FE_nodal_value_type value_type;
	int version;
	Cmiss_variable_id fe_variable;
	struct FE_node *node;
}; /* struct Cmiss_variable_nodal_value_type_specific_data */

static START_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(nodal_value)
{
	DEACCESS(Cmiss_variable)(&(data->fe_variable));
	if (data->node)
	{
		DEACCESS(FE_node)(&(data->node));
	}
	return_code=1;
}
END_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(nodal_value)

static START_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(
	nodal_value)
{
	if (destination->fe_variable=ACCESS(Cmiss_variable)(source->fe_variable))
	{
		if (source->node)
		{
			destination->node=ACCESS(FE_node)(source->node);
		}
		else
		{
			destination->node=(struct FE_node *)NULL;
		}
		destination->value_type=source->value_type;
		destination->version=source->version;
	}
	else
	{
		DEALLOCATE(destination);
	}
}
END_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(nodal_value)

static START_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	nodal_value)
{
	Cmiss_value_id *matrices,*matrix,value_type;
	Cmiss_variable_id independent_variable,*independent_variable_address;
	int i,j,number_of_columns,number_of_matrices,number_of_rows,number_of_values,
		offset;
	struct Cmiss_variable_nodal_value_type_specific_data *data;
	struct Matrix *matrix_local;

	data=(struct Cmiss_variable_nodal_value_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->fe_variable),return_code,0)
	{
		if (value_type=CREATE(Cmiss_value)())
		{
			/* get number of rows in derivative matrix */
			if (return_code=Cmiss_variable_get_value_type(variable,value_type))
			{
				if (CMISS_VALUE_IS_TYPE(FE_value)(value_type))
				{
					number_of_rows=1;
				}
				else if (CMISS_VALUE_IS_TYPE(FE_value_vector)(value_type))
				{
					return_code=Cmiss_value_FE_value_vector_get_type(value_type,
						&number_of_rows,(FE_value **)NULL);
				}
				else
				{
					number_of_rows=0;
				}
				if (0<number_of_rows)
				{
					/* check independent variables and count number of columns in
						derivative matrix */
					i=order;
					number_of_matrices=0;
					matrices=(Cmiss_value_id *)NULL;
					independent_variable_address=independent_variables;
					while (return_code&&(i>0))
					{
						independent_variable= *independent_variable_address;
						if (return_code=Cmiss_variable_get_value_type(
							independent_variable,value_type))
						{
#if defined (OLD_CODE)
							if (CMISS_VALUE_IS_TYPE(FE_value)(value_type))
							{
								number_of_values=1;
							}
							else if (CMISS_VALUE_IS_TYPE(FE_value_vector)(value_type))
							{
								return_code=Cmiss_value_FE_value_vector_get_type(value_type,
									&number_of_values,(FE_value **)NULL);
							}
							else if (CMISS_VALUE_IS_TYPE(element_xi)(value_type))
							{
								return_code=Cmiss_value_element_xi_get_type(value_type,
									&number_of_values,(struct FE_element **)NULL,
									(FE_value **)NULL);
							}
							else
							{
								return_code=0;
							}
#endif /* defined (OLD_CODE) */
							return_code=Cmiss_value_get_reals(value_type,&number_of_values,
								(FE_value **)NULL);
							if (return_code&&(0<number_of_values))
							{
								if (REALLOCATE(matrix,matrices,Cmiss_value_id,
									2*number_of_matrices+1))
								{
									matrices=matrix;
									matrix += number_of_matrices;
									if (*matrix=CREATE(Cmiss_value)())
									{
										number_of_matrices++;
										if (matrix_local=CREATE(Matrix)("matrix",DENSE,
											number_of_rows,number_of_values))
										{
											if (return_code=Cmiss_value_matrix_set_type(*matrix,
												matrix_local))
											{
												/*???DB.  Assuming that only independent variable is
													itself */
												if (Cmiss_variable_same_variable(variable,
													independent_variable)&&
													(number_of_rows==number_of_values))
												{
													j=number_of_rows;
													while (return_code&&(j>0))
													{
														return_code=Cmiss_value_matrix_set_value(*matrix,j,
															j,(Matrix_value)1);
														j--;
													}
												}
												offset=number_of_matrices;
												j=number_of_matrices-1;
												while (return_code&&(j>0))
												{
													matrix++;
													if (return_code=Cmiss_value_matrix_get_dimensions(
														*(matrix-offset),(int *)NULL,&number_of_columns))
													{
														if (*matrix=CREATE(Cmiss_value)())
														{
															number_of_matrices++;
															if (matrix_local=CREATE(Matrix)("matrix",DENSE,
																number_of_rows,number_of_values*
																number_of_columns))
															{
																if (return_code=Cmiss_value_matrix_set_type(
																	*matrix,matrix_local))
																{
																	j--;
																}
																else
																{
																	DESTROY(Matrix)(&matrix_local);
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
												}
											}
											else
											{
												DESTROY(Matrix)(&matrix_local);
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
						i--;
						independent_variable_address++;
					}
					if (return_code)
					{
						if (ALLOCATE(independent_variable_address,Cmiss_variable_id,order))
						{
							for (i=0;i<order;i++)
							{
								independent_variable_address[i]=independent_variables[i];
							}
							if (return_code=Cmiss_value_derivative_matrix_set_type(value,
								variable,order,independent_variable_address,matrices))
							{
								/* independent_variable_address and matrices are now managed by
									value */
								independent_variable_address=(Cmiss_variable_id *)NULL;
								matrices=(Cmiss_value_id *)NULL;
							}
							if (independent_variable_address)
							{
								DEALLOCATE(independent_variable_address);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_variable_nodal_value_evaluate_derivative_type_specific.  "
								"Could not allocate <independent_variable_address>");
							return_code=0;
						}
					}
					if (matrices)
					{
						matrix=matrices;
						i=number_of_matrices;
						while (i>0)
						{
							DEACCESS(Cmiss_value)(matrix);
							matrix++;
							i--;
						}
						DEALLOCATE(matrices);
					}
				}
				else
				{
					return_code=0;
				}
			}
			DESTROY(Cmiss_value)(&value_type);
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(nodal_value)

enum Swap_nodal_values_type
{
	SWAP_NODAL_VALUES_GET,
	SWAP_NODAL_VALUES_SET,
	SWAP_NODAL_VALUES_SWAP
}; /* enum Swap_nodal_values_type */

struct Swap_nodal_values_data
{
	enum FE_nodal_value_type value_type;
	enum Swap_nodal_values_type swap_type;
	FE_value *values;
	int component_number,number_of_components,number_of_values,value_number,
		version;
	struct FE_field *fe_field;
}; /* struct Swap_nodal_values_data */

static int swap_nodal_values(struct FE_node *node,
	void *swap_nodal_values_data_void)
/*******************************************************************************
LAST MODIFIED : 30 April 2003

DESCRIPTION :
Swaps the nodal values specified by <swap_nodal_values_data_void> at the <node>.
==============================================================================*/
{
	enum FE_nodal_value_type *nodal_value_type,*nodal_value_types,value_type;
	enum Swap_nodal_values_type swap_type;
	FE_value swap_value,*swap_value_1,*swap_value_2,*value,*values;
	int all_components,component_number,i,j,number_of_swap_values,
		number_of_values,number_of_values_per_version,number_of_versions,
		return_code,version;
	struct FE_field *fe_field;
	struct Swap_nodal_values_data *swap_nodal_values_data;

	ENTER(swap_nodal_values);
	return_code=0;
	/* check arguments */
	if (node&&(swap_nodal_values_data=(struct Swap_nodal_values_data *)
		swap_nodal_values_data_void))
	{
		fe_field=swap_nodal_values_data->fe_field;
		swap_type=swap_nodal_values_data->swap_type;
		value_type=swap_nodal_values_data->value_type;
		if (get_FE_nodal_field_FE_value_values(fe_field,node,&number_of_values,
			&values))
		{
			value=values;
			return_code=1;
			component_number=0;
			all_components=(swap_nodal_values_data->component_number<0)||
				(swap_nodal_values_data->component_number>=
				swap_nodal_values_data->number_of_components);
			while (return_code&&(component_number<
				swap_nodal_values_data->number_of_components))
			{
				number_of_values_per_version=
					1+get_FE_node_field_component_number_of_derivatives(node,fe_field,
					component_number);
				number_of_versions=get_FE_node_field_component_number_of_versions(node,
					fe_field,component_number);
				if (all_components||
					(component_number==swap_nodal_values_data->component_number))
				{
					swap_value_1=(swap_nodal_values_data->values)+
						(swap_nodal_values_data->value_number);
					if (FE_NODAL_UNKNOWN==value_type)
					{
						if ((0<=(version=swap_nodal_values_data->version))&&(version<
							number_of_versions))
						{
							swap_value_2=value+(version*number_of_values_per_version);
							number_of_swap_values=number_of_values_per_version;
						}
						else
						{
							swap_value_2=value;
							number_of_swap_values=
								number_of_versions*number_of_values_per_version;
						}
						switch (swap_type)
						{
							case SWAP_NODAL_VALUES_GET:
							{
								memcpy(swap_value_1,swap_value_2,
									number_of_swap_values*sizeof(FE_value));
							} break;
							case SWAP_NODAL_VALUES_SET:
							{
								memcpy(swap_value_2,swap_value_1,
									number_of_swap_values*sizeof(FE_value));
							} break;
							case SWAP_NODAL_VALUES_SWAP:
							{
								for (i=number_of_swap_values;i>0;i--)
								{
									swap_value= *swap_value_1;
									*swap_value_1= *swap_value_2;
									*swap_value_2=swap_value;
									swap_value_1++;
									swap_value_2++;
								}
							} break;
						}
						swap_nodal_values_data->value_number += number_of_swap_values;
					}
					else
					{
						if (nodal_value_types=get_FE_node_field_component_nodal_value_types(
							node,fe_field,component_number))
						{
							nodal_value_type=nodal_value_types;
							i=0;
							while ((i<number_of_values_per_version)&&
								(value_type!= *nodal_value_type))
							{
								nodal_value_type++;
								i++;
							}
							if (i<number_of_values_per_version)
							{
								if ((0<=(version=swap_nodal_values_data->version))&&(version<
									number_of_versions))
								{
									i += version*number_of_values_per_version;
									swap_value_1=(swap_nodal_values_data->values)+
										(swap_nodal_values_data->value_number);
									swap_value_2=value+i;
									switch (swap_type)
									{
										case SWAP_NODAL_VALUES_GET:
										{
											*swap_value_1= *swap_value_2;
										} break;
										case SWAP_NODAL_VALUES_SET:
										{
											*swap_value_2= *swap_value_1;
										} break;
										case SWAP_NODAL_VALUES_SWAP:
										{
											swap_value= *swap_value_1;
											*swap_value_1= *swap_value_2;
											*swap_value_2=swap_value;
										} break;
									}
									(swap_nodal_values_data->value_number)++;
								}
								else
								{
									swap_value_1=(swap_nodal_values_data->values)+
										(swap_nodal_values_data->value_number);
									swap_value_2=value+i;
									switch (swap_type)
									{
										case SWAP_NODAL_VALUES_GET:
										{
											for (j=number_of_versions;j>0;j--)
											{
												*swap_value_1= *swap_value_2;
												swap_value_1++;
												swap_value_2 += number_of_values_per_version;
											}
										} break;
										case SWAP_NODAL_VALUES_SET:
										{
											for (j=number_of_versions;j>0;j--)
											{
												*swap_value_2= *swap_value_1;
												swap_value_1++;
												swap_value_2 += number_of_values_per_version;
											}
										} break;
										case SWAP_NODAL_VALUES_SWAP:
										{
											for (j=number_of_versions;j>0;j--)
											{
												swap_value= *swap_value_1;
												*swap_value_1= *swap_value_2;
												*swap_value_2=swap_value;
												swap_value_1++;
												swap_value_2 += number_of_values_per_version;
											}
										} break;
									}
									swap_nodal_values_data->value_number += number_of_versions;
								}
							}
							DEALLOCATE(nodal_value_types);
						}
						else
						{
							return_code=0;
						}
					}
				}
				value += number_of_values_per_version*number_of_versions;
				component_number++;
			}
			if (return_code&&((SWAP_NODAL_VALUES_SET==swap_type)||
				(SWAP_NODAL_VALUES_SWAP==swap_type)))
			{
				return_code=set_FE_nodal_field_FE_value_values(fe_field,node,values,
					&number_of_values);
			}
			DEALLOCATE(values);
		}
	}
	LEAVE;

	return (return_code);
} /* swap_nodal_values */

static int Cmiss_variable_swap_nodal_values(
	enum Swap_nodal_values_type swap_type,Cmiss_variable_id variable,
	Cmiss_value_id fe_value_vector)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
==============================================================================*/
{
	int component_number,number_of_values,return_code;
	Cmiss_value_id value_type;
	struct Cmiss_variable_nodal_value_type_specific_data *data;
	struct FE_field *fe_field;
	struct FE_region *fe_region;
	struct Swap_nodal_values_data swap_nodal_values_data;

	ENTER(Cmiss_variable_swap_nodal_values);
	return_code=0;
	if (CMISS_VARIABLE_IS_TYPE(nodal_value)(variable)&&
		(data=(struct Cmiss_variable_nodal_value_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable))&&(data->fe_variable)&&
		Cmiss_variable_finite_element_get_type(data->fe_variable,&fe_field,
		&component_number)&&(fe_region=FE_field_get_FE_region(fe_field)))
	{
		swap_nodal_values_data.swap_type=swap_type;
		swap_nodal_values_data.number_of_values=0;
		swap_nodal_values_data.value_type=data->value_type;
		swap_nodal_values_data.version=data->version;
		swap_nodal_values_data.fe_field=fe_field;
		swap_nodal_values_data.component_number=component_number;
		swap_nodal_values_data.number_of_components=
			get_FE_field_number_of_components(fe_field);
		swap_nodal_values_data.value_number=0;
		swap_nodal_values_data.values=(FE_value *)NULL;
		if (value_type=CREATE(Cmiss_value)())
		{
			if (Cmiss_variable_get_value_type(variable,value_type)&&
				Cmiss_value_FE_value_vector_get_type(value_type,
				&(swap_nodal_values_data.number_of_values),(FE_value **)NULL)&&
				(0<swap_nodal_values_data.number_of_values))
			{
				switch (swap_type)
				{
					case SWAP_NODAL_VALUES_GET:
					{
						if (ALLOCATE(swap_nodal_values_data.values,FE_value,
							swap_nodal_values_data.number_of_values))
						{
							return_code=1;
						}
					} break;
					case SWAP_NODAL_VALUES_SET:
					case SWAP_NODAL_VALUES_SWAP:
					{
						if (Cmiss_value_FE_value_vector_get_type(fe_value_vector,
							&number_of_values,&(swap_nodal_values_data.values)))
						{
							if (number_of_values>=swap_nodal_values_data.number_of_values)
							{
								return_code=1;
							}
						}
					} break;
				}
				if (return_code)
				{
					if (data->node)
					{
						return_code=swap_nodal_values(data->node,
							(void *)&swap_nodal_values_data);
					}
					else
					{
						return_code=FE_region_for_each_FE_node(fe_region,swap_nodal_values,
							(void *)&swap_nodal_values_data);
					}
				}
			}
			DESTROY(Cmiss_value)(&value_type);
		}
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_swap_nodal_values */

static START_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(nodal_value)
{
	return_code=Cmiss_variable_swap_nodal_values(SWAP_NODAL_VALUES_GET,
		variable,value);
}
END_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(nodal_value)

static
	START_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	nodal_value)
{
	struct Cmiss_variable_nodal_value_type_specific_data *data;
	struct Cmiss_variable_value *variable_value;

	data=(struct Cmiss_variable_nodal_value_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&data->fe_variable,return_code,0)
	{
		if (Cmiss_variable_same_variable(dependent_variable,
			independent_variable))
		{
			if (variable_value=CREATE(Cmiss_variable_value)(dependent_variable,
				value))
			{
				return_code=Cmiss_variable_evaluate(variable_value,
					(struct LIST(Cmiss_variable_value) *)NULL);
				DESTROY(Cmiss_variable_value)(&variable_value);
			}
		}
	}
}
END_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	nodal_value)

static
	START_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	nodal_value)
{
	if (Cmiss_variable_same_variable(dependent_variable,independent_variable))
	{
		return_code=Cmiss_variable_swap_nodal_values(SWAP_NODAL_VALUES_SWAP,
			dependent_variable,value);
	}
}
END_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	nodal_value)

static START_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(
	nodal_value)
{
	int component_number;
	struct Cmiss_variable_nodal_value_type_specific_data *data;
	struct Count_nodal_values_data count_nodal_values_data;
	struct FE_field *fe_field;
	struct FE_region *fe_region;

	data=(struct Cmiss_variable_nodal_value_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->fe_variable),return_code,0)
	{
		if (Cmiss_variable_finite_element_get_type(data->fe_variable,&fe_field,
			&component_number)&&(fe_region=FE_field_get_FE_region(fe_field)))
		{
			count_nodal_values_data.number_of_values=0;
			count_nodal_values_data.value_type=data->value_type;
			count_nodal_values_data.version=data->version;
			count_nodal_values_data.fe_field=fe_field;
			count_nodal_values_data.component_number=component_number;
			count_nodal_values_data.number_of_components=
				get_FE_field_number_of_components(fe_field);
			count_nodal_values_data.number_of_node_offsets=0;
			count_nodal_values_data.offset_nodes=(struct FE_node **)NULL;
			count_nodal_values_data.node_offsets=(int *)NULL;
			count_nodal_values_data.number_of_node_values=(int *)NULL;
			if (data->node)
			{
				return_code=count_nodal_values(data->node,
					(void *)&count_nodal_values_data);
			}
			else
			{
				return_code=FE_region_for_each_FE_node(fe_region,count_nodal_values,
					(void *)&count_nodal_values_data);
			}
			if (return_code)
			{
				switch (get_FE_field_value_type(fe_field))
				{
					case FE_VALUE_VALUE:
					{
						return_code=Cmiss_value_FE_value_vector_set_type(type,
							count_nodal_values_data.number_of_values,(FE_value *)NULL);
					} break;
					default:
					{
						return_code=0;
					} break;
				}
			}
		}
	}
}
END_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(nodal_value)

static START_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(nodal_value)
{
	struct Cmiss_variable_nodal_value_type_specific_data *data;

	data=(struct Cmiss_variable_nodal_value_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->fe_variable),return_code,0)
	{
		return_code=1;
	}
}
END_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(nodal_value)

static
	START_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	nodal_value)
{
	return_code=Cmiss_variable_same_variable(dependent_variable,
		independent_variable);
}
END_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	nodal_value)

static START_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(nodal_value)
{
	return_code=1;
}
END_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(nodal_value)

static START_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(nodal_value)
{
	return_code=Cmiss_variable_same_variable(variable_1,variable_2);
}
END_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(nodal_value)

static START_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(
	nodal_value)
{
	struct Cmiss_variable_nodal_value_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_variable_nodal_value_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_1);
	data_2=(struct Cmiss_variable_nodal_value_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&(data_1->fe_variable)&&data_2&&(data_2->fe_variable),
		return_code,0)
	{
		if (Cmiss_variable_same_variable(data_1->fe_variable,
			data_2->fe_variable)&&(data_1->value_type==data_2->value_type)&&
			(data_1->version==data_2->version)&&(data_1->node==data_2->node))
		{
			return_code=1;
		}
	}
}
END_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(nodal_value)

static
	START_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	nodal_value)
{
	if (Cmiss_variable_same_variable(dependent_variable,
		independent_variable))
	{
		return_code=Cmiss_variable_swap_nodal_values(SWAP_NODAL_VALUES_SET,
			dependent_variable,value);
	}
}
END_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	nodal_value)

static char Cmiss_variable_scale_factor_type_string[]="Scale_factor";

/*???DB.  To be done */

/*
Global functions
----------------
*/
int Cmiss_variable_element_value_set_type(Cmiss_variable_id variable,
	Cmiss_variable_id fe_variable,struct FE_element *element,
	int *grid_point,int version);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Converts the <variable> into a element_value Cmiss_variable for the
specified <fe_variable> (a finite_element Cmiss_variable, all finite element
computed variables if NULL), <element> (all elements if NULL), <grid_point> (all
grid points if NULL), <version> (all versions if 0).
==============================================================================*/

DECLARE_CMISS_VARIABLE_IS_TYPE_FUNCTION(element_value)

int Cmiss_variable_element_value_get_type(
	Cmiss_variable_id variable,
	Cmiss_variable_id *fe_variable_address,
	struct FE_element **element_address,int **grid_point_address,
	int *version_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type element_value, gets its <*fe_variable_address>,
<*element_address>, <*grid_point_address> and <*version_address>.

The calling program must not DEALLOCATE the returned <*grid_point_address>.
==============================================================================*/

int Cmiss_variable_element_xi_set_type(Cmiss_variable_id variable,
	int dimension)
/*******************************************************************************
LAST MODIFIED : 11 July 2003

DESCRIPTION :
Converts the <variable> into a element_xi Cmiss_variable with the specified
<dimension>.
==============================================================================*/
{
	int return_code;
	struct Cmiss_variable_element_xi_type_specific_data *data;

	ENTER(Cmiss_variable_element_xi_set_type);
	return_code=0;
	/* check arguments */
	if (variable&&(0<=dimension))
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,struct Cmiss_variable_element_xi_type_specific_data,1))
		{
			if (data->element_xi=CREATE(Cmiss_value)())
			{
				ACCESS(Cmiss_value)(data->element_xi);
				if (Cmiss_value_element_xi_set_type(data->element_xi,dimension,
					(struct FE_element *)NULL,(FE_value *)NULL))
				{
					/* 2.  Clear current type-specific data */
					Cmiss_variable_clear_type(variable);
					/* 3.  Establish the new type */
					Cmiss_variable_set_type_specific_information(variable,
						Cmiss_variable_element_xi_type_string,(void *)data);
					/* set all the methods */
					return_code=CMISS_VARIABLE_ESTABLISH_METHODS(variable,
						element_xi);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_variable_element_xi_set_type.  "
						"Could not Cmiss_value_element_xi_set_type");
					DEACCESS(Cmiss_value)(&(data->element_xi));
					DEALLOCATE(data);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_element_xi_set_type.  "
					"Could not CREATE(Cmiss_value)");
				DEALLOCATE(data);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_element_xi_set_type.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_element_xi_set_type.  "
			"Invalid argument(s).  %p %d\n",variable,dimension);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_element_xi_set_type */

DECLARE_CMISS_VARIABLE_IS_TYPE_FUNCTION(element_xi)

int Cmiss_variable_element_xi_get_type(Cmiss_variable_id variable,
	int *dimension_address)
/*******************************************************************************
LAST MODIFIED : 11 July 2003

DESCRIPTION :
If <variable> is of type element_xi, gets its <*dimension_address>.
==============================================================================*/
{
	int return_code;
	struct Cmiss_variable_element_xi_type_specific_data *data;

	ENTER(Cmiss_variable_element_xi_get_type);
	return_code=0;
	/* check arguments */
	if (variable&&CMISS_VARIABLE_IS_TYPE(element_xi)(variable)&&dimension_address)
	{
		data=(struct Cmiss_variable_element_xi_type_specific_data *)
			Cmiss_variable_get_type_specific_data(variable);
		ASSERT_IF(data,return_code,0)
		{
			return_code=Cmiss_value_element_xi_get_type(data->element_xi,
				dimension_address,(struct FE_element **)NULL,(FE_value **)NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_element_xi_get_type.  "
			"Invalid argument(s).  %p %p",variable,dimension_address);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_element_xi_get_type */

int Cmiss_variable_FE_time_set_type(Cmiss_variable_id variable,
	Cmiss_variable_id fe_variable);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Converts the <variable> into a FE_time Cmiss_variable for the specified
<fe_variable> (a finite_element Cmiss_variable, all finite element
computed variables if NULL).
==============================================================================*/

DECLARE_CMISS_VARIABLE_IS_TYPE_FUNCTION(FE_time)

int Cmiss_variable_FE_time_get_type(Cmiss_variable_id variable,
	Cmiss_variable_id *fe_variable_address,FE_value *fe_time_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type FE_time, gets its <*fe_variable_address> and
<*fe_time_address>.
==============================================================================*/

int Cmiss_variable_finite_element_set_type(
	Cmiss_variable_id variable,struct FE_field *fe_field,
	int component_number)
/*******************************************************************************
LAST MODIFIED : 11 July 2003

DESCRIPTION :
Converts the <variable> into a finite_element Cmiss_variable for the
specified <fe_field> and <component_number> (all components if -1).

Need pointer to <fe_field_manager> so can call MANAGED_OBJECT_NOT_IN_USE in
Cmiss_variable_finite_element_not_in_use.

???DB.  Assuming that the <fe_field> "knows" its FE_region (can get FE_field
	manager)
==============================================================================*/
{
	int return_code;
	struct Cmiss_variable_finite_element_type_specific_data *data;

	ENTER(Cmiss_variable_finite_element_set_type);
	return_code=0;
	/* check arguments */
	if (variable&&fe_field)
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,
			struct Cmiss_variable_finite_element_type_specific_data,1))
		{
			/* 2.  Clear current type-specific data */
			Cmiss_variable_clear_type(variable);
			/* 3.  Establish the new type */
			if (return_code=Cmiss_variable_set_type_specific_information(variable,
				Cmiss_variable_finite_element_type_string,(void *)data))
			{
				data->dimension=0;
				data->time=(FE_value)0;
				data->xi=(FE_value *)NULL;
				data->element=(struct FE_element *)NULL;
				data->field=ACCESS(FE_field)(fe_field);
				data->component_number=component_number;
				data->node=(struct FE_node *)NULL;
				/* set all the methods */
				return_code=CMISS_VARIABLE_ESTABLISH_METHODS(variable,
					finite_element);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_finite_element_set_type.  "
					"Could not set type specific information");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_finite_element_set_type.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_finite_element_set_type.  "
			"Invalid argument(s).  %p %p",variable,fe_field);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_finite_element_set_type */

DECLARE_CMISS_VARIABLE_IS_TYPE_FUNCTION(finite_element)

int Cmiss_variable_finite_element_get_type(
	Cmiss_variable_id variable,struct FE_field **fe_field_address,
	int *component_number_address)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
If <variable> is of type finite_element, gets its <*fe_field_address> and
<*component_number_address>.
==============================================================================*/
{
	int return_code;
	struct Cmiss_variable_finite_element_type_specific_data *data;

	ENTER(Cmiss_variable_finite_element_get_type);
	return_code=0;
	/* check arguments */
	if (variable&&CMISS_VARIABLE_IS_TYPE(finite_element)(variable)&&
		(fe_field_address||component_number_address))
	{
		data=(struct Cmiss_variable_finite_element_type_specific_data *)
			Cmiss_variable_get_type_specific_data(variable);
		ASSERT_IF(data,return_code,0)
		{
			if (fe_field_address)
			{
				*fe_field_address=data->field;
			}
			if (component_number_address)
			{
				*component_number_address=data->component_number;
			}
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_finite_element_get_type.  "
			"Invalid argument(s).  %p %p %p",variable,fe_field_address,
			component_number_address);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_finite_element_get_type */

int Cmiss_variable_nodal_value_set_type(Cmiss_variable_id variable,
	Cmiss_variable_id fe_variable,struct FE_node *node,
	enum FE_nodal_value_type value_type,int version)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Converts the <variable> into a nodal_value Cmiss_variable for the
specified <fe_variable> (a finite_element Cmiss_variable), <node> (all nodes
if NULL), <value_type> (all types if FE_NODAL_UNKNOWN), <version> (all versions
if 0).
==============================================================================*/
{
	int return_code;
	struct Cmiss_variable_nodal_value_type_specific_data *data;

	ENTER(Cmiss_variable_nodal_value_set_type);
	return_code=0;
	/* check arguments */
	if (variable&&fe_variable)
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,
			struct Cmiss_variable_nodal_value_type_specific_data,1))
		{
			/* 2.  Clear current type-specific data */
			Cmiss_variable_clear_type(variable);
			/* 3.  Establish the new type */
			if (return_code=Cmiss_variable_set_type_specific_information(variable,
				Cmiss_variable_nodal_value_type_string,(void *)data))
			{
				data->fe_variable=ACCESS(Cmiss_variable)(fe_variable);
				if (node)
				{
					data->node=ACCESS(FE_node)(node);
				}
				else
				{
					data->node=(struct FE_node *)NULL;
				}
				data->value_type=value_type;
				data->version=version;
				/* set all the methods */
				return_code=CMISS_VARIABLE_ESTABLISH_METHODS(variable,nodal_value);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_nodal_value_set_type.  "
					"Could not set type specific information");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_nodal_value_set_type.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_nodal_value_set_type.  "
			"Invalid argument(s).  %p %p",variable,fe_variable);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_nodal_value_set_type */

DECLARE_CMISS_VARIABLE_IS_TYPE_FUNCTION(nodal_value)

int Cmiss_variable_nodal_value_get_type(Cmiss_variable_id variable,
	Cmiss_variable_id *fe_variable_address,struct FE_node **node_address,
	enum FE_nodal_value_type *value_type_address,int *version_address)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
If <variable> is of type nodal_value, gets its <*fe_variable_address>,
<*node_address>, <*value_type_address> and <*version_address>.
==============================================================================*/
{
	int return_code;
	struct Cmiss_variable_nodal_value_type_specific_data *data;

	ENTER(Cmiss_variable_nodal_value_get_type);
	return_code=0;
	/* check arguments */
	if (variable&&CMISS_VARIABLE_IS_TYPE(nodal_value)(variable)&&
		(fe_variable_address||node_address||value_type_address||version_address))
	{
		data=(struct Cmiss_variable_nodal_value_type_specific_data *)
			Cmiss_variable_get_type_specific_data(variable);
		ASSERT_IF(data,return_code,0)
		{
			if (fe_variable_address)
			{
				*fe_variable_address=data->fe_variable;
			}
			if (node_address)
			{
				*node_address=data->node;
			}
			if (value_type_address)
			{
				*value_type_address=data->value_type;
			}
			if (version_address)
			{
				*version_address=data->version;
			}
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_nodal_value_get_type.  "
			"Invalid argument(s).  %p %p %p %p %p",variable,fe_variable_address,
			node_address,value_type_address,version_address);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_nodal_value_get_type */

int Cmiss_variable_scale_factor_set_type(Cmiss_variable_id variable,
	Cmiss_variable_id fe_variable,struct FE_element *element,
	int local_node_number,enum FE_nodal_value_type value_type,int version);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Converts the <variable> into a scale_factor Cmiss_variable for the
specified <fe_variable> (a finite_element Cmiss_variable, all finite element
computed variables if NULL), <element> (all elements if NULL),
<local_node_number> (all local nodes if -1), <value_type> (all types if
FE_NODAL_UNKNOWN), <version> (all versions if 0).
==============================================================================*/

DECLARE_CMISS_VARIABLE_IS_TYPE_FUNCTION(scale_factor)

int Cmiss_variable_scale_factor_get_type(Cmiss_variable_id variable,
	Cmiss_variable_id *fe_variable_address,struct FE_element **element_address,
	int *local_node_number_address,enum FE_nodal_value_type *value_type_address,
	int *version_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type scale_factor, gets its <*fe_variable_address>,
<*element_address>, <*local_node_number_address>, <*value_type_address> and
<*version_address>.
==============================================================================*/
