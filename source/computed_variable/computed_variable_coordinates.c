/*******************************************************************************
FILE : computed_variable_coordinates.c

LAST MODIFIED : 28 July 2003

DESCRIPTION :
Implements computed variables which transform between coordinate systems.
==============================================================================*/
#include <math.h>
#include "computed_variable/computed_value_derivative_matrix.h"
#include "computed_variable/computed_value_fe_value.h"
#include "computed_variable/computed_value_matrix.h"
#include "computed_variable/computed_variable_coordinates.h"
#include "computed_variable/computed_variable_private.h"
#include "general/debug.h"
#include "general/geometry.h"
#include "user_interface/message.h"

/*
Module methods
--------------
*/
static char Cmiss_variable_coordinates_type_string[]="Coordinates";

struct Cmiss_variable_coordinates_type_specific_data
/*******************************************************************************
LAST MODIFIED : 26 June 2003

DESCRIPTION :
???DB.  Should it contain a Coordinate_system?  No, because value will just be a
	vector and so can't check?
==============================================================================*/
{
	Cmiss_value_id coordinates;
}; /* struct Cmiss_variable_coordinates_type_specific_data */

static START_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(coordinates)
{
	DEACCESS(Cmiss_value)(&(data->coordinates));
	return_code=1;
}
END_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(coordinates)

static START_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(
	coordinates)
{
	if (destination->coordinates=CREATE(Cmiss_value)())
	{
		ACCESS(Cmiss_value)(destination->coordinates);
		if (!Cmiss_value_copy(destination->coordinates,source->coordinates))
		{
			DEACCESS(Cmiss_value)(&(destination->coordinates));
			DEALLOCATE(destination);
		}
	}
	else
	{
		DEALLOCATE(destination);
	}
}
END_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(coordinates)

static START_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	coordinates)
{
	Cmiss_value_id *matrices,*matrix,value_type;
	Cmiss_variable_id independent_variable,*independent_variable_address;
	int i,j,number_of_columns,number_of_matrices,number_of_rows,number_of_values,
		offset;
	struct Cmiss_variable_coordinates_type_specific_data *data;
	struct Matrix *matrix_local;

	data=(struct Cmiss_variable_coordinates_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->coordinates),
		return_code,0)
	{
		/* get number of rows in derivative matrix */
		if (Cmiss_value_FE_value_vector_get_type(data->coordinates,&number_of_rows,
			(FE_value **)NULL)&&(0<number_of_rows))
		{
			if (value_type=CREATE(Cmiss_value)())
			{
				/* check independent variables and count number of matrices in
					derivative */
				return_code=1;
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
							"Cmiss_variable_coordinates_evaluate_derivative_type_specific.  "
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
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(coordinates)

static START_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(coordinates)
{
	struct Cmiss_variable_coordinates_type_specific_data *data;

	data=(struct Cmiss_variable_coordinates_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->coordinates),
		return_code,0)
	{
		if (Cmiss_value_copy(value,data->coordinates))
		{
			return_code=0;
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(coordinates)

static
	START_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	coordinates)
{
	struct Cmiss_variable_coordinates_type_specific_data *data;

	data=(struct Cmiss_variable_coordinates_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->coordinates),
		return_code,0)
	{
		if (Cmiss_variable_same_variable(dependent_variable,
			independent_variable))
		{
			return_code=Cmiss_value_copy(value,data->coordinates);
		}
	}
}
END_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	coordinates)

static
	START_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	coordinates)
{
	Cmiss_value_id temp_value;
	struct Cmiss_variable_coordinates_type_specific_data *data;

	data=(struct Cmiss_variable_coordinates_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->coordinates),
		return_code,0)
	{
		if (Cmiss_variable_same_variable(dependent_variable,
			independent_variable)&&CMISS_VALUE_IS_TYPE(FE_value_vector)(value))
		{
			if (temp_value=CREATE(Cmiss_value)())
			{
				if (Cmiss_value_copy(temp_value,data->coordinates))
				{
					if (Cmiss_value_copy(data->coordinates,value))
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
	coordinates)

static START_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(
	coordinates)
{
	int number_of_fe_values;
	struct Cmiss_variable_coordinates_type_specific_data *data;

	data=(struct Cmiss_variable_coordinates_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->coordinates),
		return_code,0)
	{
		if (Cmiss_value_FE_value_vector_get_type(data->coordinates,
			&number_of_fe_values,(FE_value **)NULL))
		{
			return_code=Cmiss_value_FE_value_vector_set_type(type,number_of_fe_values,
				(FE_value *)NULL);
		}
	}
}
END_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(coordinates)

static START_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(coordinates)
{
	FE_value *fe_values;
	int number_of_fe_values;
	struct Cmiss_variable_coordinates_type_specific_data *data;

	data=(struct Cmiss_variable_coordinates_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->coordinates),
		return_code,0)
	{
		if (Cmiss_value_FE_value_vector_get_type(data->coordinates,
			&number_of_fe_values,&fe_values)&&(0<number_of_fe_values)&&fe_values)
		{
			return_code=1;
		}
	}
}
END_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(coordinates)

static
	START_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	coordinates)
{
	return_code=Cmiss_variable_same_variable(dependent_variable,
		independent_variable);
}
END_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	coordinates)

static START_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(coordinates)
{
	return_code=1;
}
END_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(coordinates)

static START_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(coordinates)
{
	return_code=Cmiss_variable_same_variable(variable_1,variable_2);
}
END_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(coordinates)

static START_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(
	coordinates)
{
	int number_of_fe_values_1,number_of_fe_values_2;
	struct Cmiss_variable_coordinates_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_variable_coordinates_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_1);
	data_2=(struct Cmiss_variable_coordinates_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data_1->coordinates)&&
		data_2&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data_2->coordinates),
		return_code,0)
	{
		if (Cmiss_value_FE_value_vector_get_type(data_1->coordinates,
			&number_of_fe_values_1,(FE_value **)NULL)&&
			Cmiss_value_FE_value_vector_get_type(data_2->coordinates,
			&number_of_fe_values_2,(FE_value **)NULL)&&
			(number_of_fe_values_1==number_of_fe_values_2))
		{
			return_code=1;
		}
	}
}
END_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(coordinates)

static
	START_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	coordinates)
{
	struct Cmiss_variable_coordinates_type_specific_data *data;

	data=(struct Cmiss_variable_coordinates_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->coordinates),
		return_code,0)
	{
		if (Cmiss_variable_same_variable(dependent_variable,
			independent_variable))
		{
			/*???DB.  Copy into <independent_variable> as well? */
			return_code=Cmiss_value_copy(data->coordinates,value);
		}
	}
}
END_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	coordinates)

static char Cmiss_variable_spheroidal_coordinates_focus_type_string[]=
	"Spheroidal_coordinates_focus";

struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data
/*******************************************************************************
LAST MODIFIED : 26 June 2003

DESCRIPTION :
==============================================================================*/
{
	Cmiss_value_id focus;
}; /* struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data */

static START_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)
{
	DEACCESS(Cmiss_value)(&(data->focus));
	return_code=1;
}
END_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(spheroidal_coordinates_focus)

static START_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)
{
	if (destination->focus=CREATE(Cmiss_value)())
	{
		ACCESS(Cmiss_value)(destination->focus);
		if (!Cmiss_value_copy(destination->focus,source->focus))
		{
			DEACCESS(Cmiss_value)(&(destination->focus));
			DEALLOCATE(destination);
		}
	}
	else
	{
		DEALLOCATE(destination);
	}
}
END_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)

static START_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)
{
	Cmiss_value_id *matrices,*matrix,value_type;
	Cmiss_variable_id independent_variable,*independent_variable_address;
	int i,j,number_of_columns,number_of_matrices,number_of_rows,number_of_values,
		offset;
	struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data *data;
	struct Matrix *matrix_local;

	data=(struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value)(data->focus),return_code,0)
	{
		/* get number of rows in derivative matrix */
		number_of_rows=1;
		if (value_type=CREATE(Cmiss_value)())
		{
			/* check independent variables and count number of matrices in
				derivative */
			return_code=1;
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
						"Cmiss_variable_spheroidal_coordinates_focus_evaluate_derivative_type_specific.  "
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
	}
}
END_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)

static START_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)
{
	struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data *data;

	data=(struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value)(data->focus),return_code,0)
	{
		if (Cmiss_value_copy(value,data->focus))
		{
			return_code=0;
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(spheroidal_coordinates_focus)

static
	START_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)
{
	struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data *data;

	data=(struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value)(data->focus),return_code,0)
	{
		if (Cmiss_variable_same_variable(dependent_variable,
			independent_variable))
		{
			return_code=Cmiss_value_copy(value,data->focus);
		}
	}
}
END_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)

static
	START_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)
{
	Cmiss_value_id temp_value;
	struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data *data;

	data=(struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value)(data->focus),return_code,0)
	{
		if (Cmiss_variable_same_variable(dependent_variable,
			independent_variable)&&CMISS_VALUE_IS_TYPE(FE_value)(value))
		{
			if (temp_value=CREATE(Cmiss_value)())
			{
				if (Cmiss_value_copy(temp_value,data->focus))
				{
					if (Cmiss_value_copy(data->focus,value))
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
	spheroidal_coordinates_focus)

static START_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)
{
	struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data *data;

	data=(struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value)(data->focus),return_code,0)
	{
		return_code=Cmiss_value_FE_value_set_type(type,(FE_value)0);
	}
}
END_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)

static START_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)
{
	struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data *data;

	data=(struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value)(data->focus),return_code,0)
	{
		return_code=1;
	}
}
END_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)

static
	START_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)
{
	return_code=Cmiss_variable_same_variable(dependent_variable,
		independent_variable);
}
END_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)

static START_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)
{
	return_code=1;
}
END_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)

static START_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)
{
	return_code=Cmiss_variable_same_variable(variable_1,variable_2);
}
END_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(spheroidal_coordinates_focus)

static START_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)
{
	struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data *data_1,
		*data_2;

	data_1=
		(struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_1);
	data_2=
		(struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&CMISS_VALUE_IS_TYPE(FE_value)(data_1->focus)&&data_2&&
		CMISS_VALUE_IS_TYPE(FE_value)(data_2->focus),return_code,0)
	{
		return_code=1;
	}
}
END_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)

static
	START_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)
{
	struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data *data;

	data=(struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value)(data->focus),return_code,0)
	{
		if (Cmiss_variable_same_variable(dependent_variable,
			independent_variable))
		{
			/*???DB.  Copy into <independent_variable> as well? */
			return_code=Cmiss_value_copy(data->focus,value);
		}
	}
}
END_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	spheroidal_coordinates_focus)

static char 
	Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_string[]=
	"Prolate_spheroidal_to_rectangular_cartesian";

struct
	Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data
/*******************************************************************************
LAST MODIFIED : 27 June 2003

DESCRIPTION :
==============================================================================*/
{
	double focus,lambda,mu,theta;
}; /* struct
	Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data
	*/

static START_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)
{
	return_code=1;
}
END_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)

static START_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)
{
	destination->focus=source->focus;
	destination->lambda=source->lambda;
	destination->mu=source->mu;
	destination->theta=source->theta;
}
END_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)

enum Cmiss_variable_evaluate_derivative_prolate_spheroidal_to_rectangular_cartesian_type
{
	COORDINATES_TYPE,
	FOCUS_TYPE,
	ZERO_DERIVATIVE_TYPE
}; /* enum Cmiss_variable_evaluate_derivative_prolate_spheroidal_to_rectangular_cartesian_type */

#define NUMBER_OF_PROLATE_SPHEROIDAL_COORDINATES 3

static START_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)
/*******************************************************************************
LAST MODIFIED : 28 July 2003

DESCRIPTION :
==============================================================================*/
{
	Cmiss_value_id *matrices,*matrix,value_type;
	Cmiss_variable_id independent_variable,*local_independent_variables;
	double a1,a2,a3,a4,a5,a6,a7,a8,a9;
	enum Cmiss_variable_evaluate_derivative_prolate_spheroidal_to_rectangular_cartesian_type
		*independent_variables_type;
	int coordinates_order,*derivative_independent_variables,
		derivative_number_of_values,derivative_order,focus_order,i,
		independent_variables_index,*independent_variables_number_of_values,
		number_of_components,number_of_matrices,number_of_values,zero_derivative;
	struct Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data *data;
	struct Matrix *matrix_local;

	data=(struct Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data,return_code,0)
	{
		/* set up temporary storage */
		value_type=CREATE(Cmiss_value)();
		ALLOCATE(independent_variables_number_of_values,int,order);
		ALLOCATE(independent_variables_type,
			enum Cmiss_variable_evaluate_derivative_prolate_spheroidal_to_rectangular_cartesian_type,
			order);
		ALLOCATE(derivative_independent_variables,int,order+1);
		if (value_type&&independent_variables_number_of_values&&
			independent_variables_type&&derivative_independent_variables)
		{
			return_code=1;
			/* get number of rows in derivative matrix (<number_of_components>) */
			number_of_components=NUMBER_OF_PROLATE_SPHEROIDAL_COORDINATES;
			/* loop over independent variables building up derivative matrix */
			independent_variables_index=0;
			/* temporary storage that is built up along with the derivative matrix */
			matrices=(Cmiss_value_id *)NULL;
			number_of_matrices=0;
			while (return_code&&(independent_variables_index<order))
			{
				independent_variable=independent_variables[independent_variables_index];
				/* calculate component values and monomial information for
					derivatives */
				number_of_values=0;
				if (CMISS_VARIABLE_IS_TYPE(coordinates)(independent_variable))
				{
					/* differentiating with respect to coordinates */
					independent_variables_type[independent_variables_index]=
						COORDINATES_TYPE;
					return_code=Cmiss_variable_coordinates_get_type(independent_variable,
						&number_of_values);
				}
				else if (CMISS_VARIABLE_IS_TYPE(spheroidal_coordinates_focus)(
					independent_variable))
				{
					independent_variables_type[independent_variables_index]=FOCUS_TYPE;
					number_of_values=1;
				}
				else
				{
					independent_variables_type[independent_variables_index]=
						ZERO_DERIVATIVE_TYPE;
				}
				if (return_code&&(ZERO_DERIVATIVE_TYPE==independent_variables_type[
					independent_variables_index]))
				{
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
							/* determine the coordinates and focus orders */
							coordinates_order=0;
							focus_order=0;
							i=0;
							zero_derivative=0;
							derivative_number_of_values=1;
							derivative_order=0;
							/* also set up mapping from variables involved in derivative to
								all independent variables */
							while (i<=independent_variables_index)
							{
								if (derivative_independent_variables[i])
								{
									switch (independent_variables_type[i])
									{
										case COORDINATES_TYPE:
										{
											coordinates_order++;
										} break;
										case FOCUS_TYPE:
										{
											focus_order++;
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
							if (return_code&&!((focus_order>1)||(zero_derivative)))
							{
								a1=sinh(data->lambda);
								a2=cosh(data->lambda);
								if (0==focus_order)
								{
									a1 *= data->focus;
									a2 *= data->focus;
								}
								a3=sin(data->mu);
								a4=cos(data->mu);
								a5=sin(data->theta);
								a6=cos(data->theta);
								a7=a1*a3;
								switch (coordinates_order)
								{
									case 0:
									{
										if (!(Cmiss_value_matrix_set_value(*matrix,
											1,1,(Matrix_value)(a2*a4))&&
											Cmiss_value_matrix_set_value(*matrix,
											2,1,(Matrix_value)(a7*a6))&&
											Cmiss_value_matrix_set_value(*matrix,
											3,1,(Matrix_value)(a7*a5))))
										{
											return_code=0;
										}
									} break;
									case 1:
									{
										a8=a1*a4;
										a9=a2*a3;
										if (!(Cmiss_value_matrix_set_value(*matrix,
											1,1,(Matrix_value)(a8))&&
											Cmiss_value_matrix_set_value(*matrix,
											1,2,(Matrix_value)(-a9))&&
											Cmiss_value_matrix_set_value(*matrix,
											1,3,(Matrix_value)(0))&&
											Cmiss_value_matrix_set_value(*matrix,
											2,1,(Matrix_value)(a9*a6))&&
											Cmiss_value_matrix_set_value(*matrix,
											2,2,(Matrix_value)(a8*a6))&&
											Cmiss_value_matrix_set_value(*matrix,
											2,3,(Matrix_value)(-a7*a5))&&
											Cmiss_value_matrix_set_value(*matrix,
											3,1,(Matrix_value)(a9*a5))&&
											Cmiss_value_matrix_set_value(*matrix,
											3,2,(Matrix_value)(a8*a5))&&
											Cmiss_value_matrix_set_value(*matrix,
											3,3,(Matrix_value)(a7*a6))))
										{
											return_code=0;
										}
									} break;
									case 2:
									{
										a8=a1*a4;
										a9=a2*a3;
										if (!(Cmiss_value_matrix_set_value(*matrix,
											1,1,(Matrix_value)(a2*a4))&&
											Cmiss_value_matrix_set_value(*matrix,
											1,2,(Matrix_value)(-a1*a3))&&
											Cmiss_value_matrix_set_value(*matrix,
											1,3,(Matrix_value)(0))&&
											Cmiss_value_matrix_set_value(*matrix,
											1,4,(Matrix_value)(-a1*a3))&&
											Cmiss_value_matrix_set_value(*matrix,
											1,5,(Matrix_value)(-a2*a4))&&
											Cmiss_value_matrix_set_value(*matrix,
											1,6,(Matrix_value)(0))&&
											Cmiss_value_matrix_set_value(*matrix,
											1,7,(Matrix_value)(0))&&
											Cmiss_value_matrix_set_value(*matrix,
											1,8,(Matrix_value)(0))&&
											Cmiss_value_matrix_set_value(*matrix,
											1,9,(Matrix_value)(0))&&
											Cmiss_value_matrix_set_value(*matrix,
											2,1,(Matrix_value)(a7*a6))&&
											Cmiss_value_matrix_set_value(*matrix,
											2,2,(Matrix_value)(a2*a4*a6))&&
											Cmiss_value_matrix_set_value(*matrix,
											2,3,(Matrix_value)(-a9*a5))&&
											Cmiss_value_matrix_set_value(*matrix,
											2,4,(Matrix_value)(a2*a4*a6))&&
											Cmiss_value_matrix_set_value(*matrix,
											2,5,(Matrix_value)(-a7*a6))&&
											Cmiss_value_matrix_set_value(*matrix,
											2,6,(Matrix_value)(-a8*a5))&&
											Cmiss_value_matrix_set_value(*matrix,
											2,7,(Matrix_value)(-a9*a5))&&
											Cmiss_value_matrix_set_value(*matrix,
											2,8,(Matrix_value)(-a8*a5))&&
											Cmiss_value_matrix_set_value(*matrix,
											2,9,(Matrix_value)(-a7*a6))&&
											Cmiss_value_matrix_set_value(*matrix,
											3,1,(Matrix_value)(a7*a5))&&
											Cmiss_value_matrix_set_value(*matrix,
											3,2,(Matrix_value)(a2*a4*a5))&&
											Cmiss_value_matrix_set_value(*matrix,
											3,3,(Matrix_value)(a9*a6))&&
											Cmiss_value_matrix_set_value(*matrix,
											3,4,(Matrix_value)(a2*a4*a5))&&
											Cmiss_value_matrix_set_value(*matrix,
											3,5,(Matrix_value)(-a7*a5))&&
											Cmiss_value_matrix_set_value(*matrix,
											3,6,(Matrix_value)(a8*a6))&&
											Cmiss_value_matrix_set_value(*matrix,
											3,7,(Matrix_value)(a9*a6))&&
											Cmiss_value_matrix_set_value(*matrix,
											3,8,(Matrix_value)(a8*a6))&&
											Cmiss_value_matrix_set_value(*matrix,
											3,9,(Matrix_value)(-a7*a5))))
										{
											return_code=0;
										}
									} break;
									default:
									{
										display_message(INFORMATION_MESSAGE,
											"Code for d3(rectangular_cartesian)/d(prolate_spheroidal)3 and higher has not been written\n");
										return_code=0;
									} break;
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
							"Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_evaluate_derivative_type_specific.  "
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
						"Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_evaluate_derivative_type_specific.  "
						"Could not allocate <independent_variable_address>");
					return_code=0;
				}
			}
			if (!return_code&&matrices)
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
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_evaluate_derivative_type_specific.  "
				"Could not allocate working storage");
		}
		/* remove temporary storage that was built up along with the derivative
			matrix */
		DEALLOCATE(derivative_independent_variables);
		DEALLOCATE(independent_variables_type);
		DEALLOCATE(independent_variables_number_of_values);
		if (value_type)
		{
			DESTROY(Cmiss_value)(&value_type);
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(prolate_spheroidal_to_rectangular_cartesian)

static START_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(prolate_spheroidal_to_rectangular_cartesian)
{
	double a1,a2,a3,a4,a5,a6,a7;
	FE_value *fe_value_vector;
	struct Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data *data;

	data=(struct Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data,return_code,0)
	{
		if (ALLOCATE(fe_value_vector,FE_value,3))
		{
			a1=(data->focus)*sinh(data->lambda);
			a2=(data->focus)*cosh(data->lambda);
			a3=sin(data->mu);
			a4=cos(data->mu);
			a5=sin(data->theta);
			a6=cos(data->theta);
			a7=a1*a3;
			fe_value_vector[0]=(FE_value)(a2*a4);
			fe_value_vector[1]=(FE_value)(a7*a6);
			fe_value_vector[2]=(FE_value)(a7*a5);
			return_code=Cmiss_value_FE_value_vector_set_type(value,3,fe_value_vector);
			if (!return_code)
			{
				DEALLOCATE(fe_value_vector);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_evaluate_type_specific.  "
				"Could not allocate fe_value_vector");
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(prolate_spheroidal_to_rectangular_cartesian)

static
	START_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)
{
	FE_value *fe_value_vector;
	struct Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data *data;

	data=(struct Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data,return_code,0)
	{
		if (CMISS_VARIABLE_IS_TYPE(coordinates)(independent_variable))
		{
			if (ALLOCATE(fe_value_vector,FE_value,3))
			{
				if (Cmiss_value_FE_value_vector_set_type(value,3,fe_value_vector))
				{
					fe_value_vector[0]=(FE_value)(data->lambda);
					fe_value_vector[1]=(FE_value)(data->mu);
					fe_value_vector[2]=(FE_value)(data->theta);
					return_code=1;
				}
				else
				{
					DEALLOCATE(fe_value_vector);
				}
			}
		}
		else if (CMISS_VARIABLE_IS_TYPE(spheroidal_coordinates_focus)(
			independent_variable))
		{
			return_code=Cmiss_value_FE_value_set_type(value,(FE_value)(data->focus));
		}
	}
}
END_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)

static
	START_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)
{
	FE_value *fe_value_vector,temp_fe_value;
	int number_of_fe_values;
	struct Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data *data;

	data=(struct Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data,return_code,0)
	{
		if (CMISS_VARIABLE_IS_TYPE(coordinates)(independent_variable))
		{
			if (Cmiss_value_FE_value_vector_get_type(value,&number_of_fe_values,
				&fe_value_vector)&&(3==number_of_fe_values)&&fe_value_vector)
			{
				temp_fe_value=fe_value_vector[0];
				fe_value_vector[0]=(FE_value)(data->lambda);
				data->lambda=(double)temp_fe_value;
				temp_fe_value=fe_value_vector[1];
				fe_value_vector[1]=(FE_value)(data->mu);
				data->mu=(double)temp_fe_value;
				temp_fe_value=fe_value_vector[2];
				fe_value_vector[2]=(FE_value)(data->theta);
				data->theta=(double)temp_fe_value;
				return_code=1;
			}
		}
		else if (CMISS_VARIABLE_IS_TYPE(spheroidal_coordinates_focus)(
			independent_variable))
		{
			if (Cmiss_value_FE_value_get_type(value,&temp_fe_value))
			{
				if (Cmiss_value_FE_value_set_type(value,(FE_value)(data->focus)))
				{
					data->focus=(double)temp_fe_value;
					return_code=1;
				}
			}
		}
	}
}
END_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(prolate_spheroidal_to_rectangular_cartesian)

static START_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)
{
	struct Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data *data;

	data=(struct Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data,return_code,0)
	{
		return_code=Cmiss_value_FE_value_vector_set_type(type,3,(FE_value *)NULL);
	}
}
END_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(prolate_spheroidal_to_rectangular_cartesian)

static START_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(prolate_spheroidal_to_rectangular_cartesian)
{
	struct Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data *data;

	data=(struct Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data,return_code,0)
	{
		return_code=1;
	}
}
END_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(prolate_spheroidal_to_rectangular_cartesian)

static
	START_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)
{
	if (CMISS_VARIABLE_IS_TYPE(coordinates)(independent_variable)||
		CMISS_VARIABLE_IS_TYPE(spheroidal_coordinates_focus)(independent_variable))
	{
		return_code=1;
	}
}
END_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)

static START_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)
{
	struct Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data *data;

	data=(struct Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data,return_code,0)
	{
		return_code=1;
	}
}
END_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)

static START_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)
{
	return_code=Cmiss_variable_same_variable(variable_1,variable_2);
}
END_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)

static START_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)
{
	return_code=1;
}
END_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)

static
	START_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)
{
	FE_value fe_value,*fe_value_vector;
	int number_of_fe_values;
	struct Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data *data;

	data=(struct Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data,return_code,0)
	{
		if (CMISS_VARIABLE_IS_TYPE(coordinates)(independent_variable))
		{
			if (Cmiss_value_FE_value_vector_get_type(value,&number_of_fe_values,
				&fe_value_vector)&&(3==number_of_fe_values)&&fe_value_vector)
			{
				data->lambda=(double)fe_value_vector[0];
				data->mu=(double)fe_value_vector[1];
				data->theta=(double)fe_value_vector[2];
				return_code=1;
			}
		}
		else if (CMISS_VARIABLE_IS_TYPE(spheroidal_coordinates_focus)(
			independent_variable))
		{
			if (Cmiss_value_FE_value_get_type(value,&fe_value))
			{
				data->focus=(double)fe_value;
				return_code=1;
			}
		}
	}
}
END_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)

/*
Global functions
----------------
*/
int Cmiss_variable_coordinates_set_type(Cmiss_variable_id variable,
	int dimension)
/*******************************************************************************
LAST MODIFIED : 30 June 2003

DESCRIPTION :
Converts the <variable> into a coordinates Cmiss_variable.

Only used to name independent variables and so can't be evaluated.
==============================================================================*/
{
	int return_code;
	struct Cmiss_variable_coordinates_type_specific_data *data;

	ENTER(Cmiss_variable_coordinates_set_type);
	return_code=0;
	/* check arguments */
	if (variable&&(0<=dimension))
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,struct Cmiss_variable_coordinates_type_specific_data,1))
		{
			if (data->coordinates=CREATE(Cmiss_value)())
			{
				ACCESS(Cmiss_value)(data->coordinates);
				if (Cmiss_value_FE_value_vector_set_type(data->coordinates,
					dimension,(FE_value *)NULL))
				{
					/* 2.  Clear current type-specific data */
					Cmiss_variable_clear_type(variable);
					/* 3.  Establish the new type */
					Cmiss_variable_set_type_specific_information(variable,
						Cmiss_variable_coordinates_type_string,(void *)data);
					/* set all the methods */
					return_code=CMISS_VARIABLE_ESTABLISH_METHODS(variable,
						coordinates);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_variable_coordinates_set_type.  "
						"Could not Cmiss_value_FE_value_vector_set_type");
					DEACCESS(Cmiss_value)(&(data->coordinates));
					DEALLOCATE(data);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_coordinates_set_type.  "
					"Could not CREATE(Cmiss_value)");
				DEALLOCATE(data);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_coordinates_set_type.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_coordinates_set_type.  "
			"Invalid argument(s).  %p\n",variable);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_coordinates_set_type */

DECLARE_CMISS_VARIABLE_IS_TYPE_FUNCTION(coordinates)

int Cmiss_variable_coordinates_get_type(Cmiss_variable_id variable,
	int *dimension_address)
/*******************************************************************************
LAST MODIFIED : 28 July 2003

DESCRIPTION :
If <variable> is of type coordinates gets its <*dimension_address>.
==============================================================================*/
{
	int return_code;
	struct Cmiss_variable_coordinates_type_specific_data *data;

	ENTER(Cmiss_variable_coordinates_set_type);
	return_code=0;
	/* check arguments */
	if (variable&&CMISS_VARIABLE_IS_TYPE(coordinates)(variable)&&
		dimension_address)
	{
		data=(struct Cmiss_variable_coordinates_type_specific_data *)
			Cmiss_variable_get_type_specific_data(variable);
		ASSERT_IF(data,return_code,0)
		{
			return_code=Cmiss_value_get_reals(data->coordinates,dimension_address,
				(FE_value **)NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_coordinates_set_type.  "
			"Invalid argument(s).  %p %p\n",variable,dimension_address);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_coordinates_set_type */

int Cmiss_variable_spheroidal_coordinates_focus_set_type(
	Cmiss_variable_id variable)
/*******************************************************************************
LAST MODIFIED : 27 June 2003

DESCRIPTION :
Converts the <variable> into a spheroidal_coordinates_focus Cmiss_variable.

Only used to name independent variables and so can't be evaluated.
==============================================================================*/
{
	int return_code;
	struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data *data;

	ENTER(Cmiss_variable_spheroidal_coordinates_focus_set_type);
	return_code=0;
	/* check arguments */
	if (variable)
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,
			struct Cmiss_variable_spheroidal_coordinates_focus_type_specific_data,1))
		{
			if (data->focus=CREATE(Cmiss_value)())
			{
				ACCESS(Cmiss_value)(data->focus);
				if (Cmiss_value_FE_value_set_type(data->focus,(FE_value)0))
				{
					/* 2.  Clear current type-specific data */
					Cmiss_variable_clear_type(variable);
					/* 3.  Establish the new type */
					Cmiss_variable_set_type_specific_information(variable,
						Cmiss_variable_spheroidal_coordinates_focus_type_string,
						(void *)data);
					/* set all the methods */
					return_code=CMISS_VARIABLE_ESTABLISH_METHODS(variable,
						spheroidal_coordinates_focus);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_variable_spheroidal_coordinates_focus_set_type.  "
						"Could not Cmiss_value_FE_value_set_type");
					DEACCESS(Cmiss_value)(&(data->focus));
					DEALLOCATE(data);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_spheroidal_coordinates_focus_set_type.  "
					"Could not CREATE(Cmiss_value)");
				DEALLOCATE(data);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_spheroidal_coordinates_focus_set_type.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_spheroidal_coordinates_focus_set_type.  "
			"Invalid argument(s).  %p\n",variable);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_spheroidal_coordinates_focus_set_type */

DECLARE_CMISS_VARIABLE_IS_TYPE_FUNCTION(spheroidal_coordinates_focus)

int Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_set_type(
	Cmiss_variable_id variable)
/*******************************************************************************
LAST MODIFIED : 27 June 2003

DESCRIPTION :
Converts the <variable> into a prolate_spheroidal_to_rectangular_cartesian
Cmiss_variable.

Independent variables are: coordinates and spheroidal_coordinates_focus.
==============================================================================*/
{
	int return_code;
	struct Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data *data;

	ENTER(Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_set_type);
	return_code=0;
	/* check arguments */
	if (variable)
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,
			struct Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_specific_data,
			1))
		{
			/* 2.  Clear current type-specific data */
			Cmiss_variable_clear_type(variable);
			/* 3.  Establish the new type */
			if (return_code=Cmiss_variable_set_type_specific_information(variable,
				Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_type_string,
				(void *)data))
			{
				data->focus=(FE_value)0;
				data->lambda=(FE_value)0;
				data->mu=(FE_value)0;
				data->theta=(FE_value)0;
				/* set all the methods */
				return_code=CMISS_VARIABLE_ESTABLISH_METHODS(variable,
					prolate_spheroidal_to_rectangular_cartesian);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_set_type.  "
					"Could not set type specific information");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_set_type.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_set_type.  "
			"Invalid argument(s).  %p\n",variable);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_set_type */

DECLARE_CMISS_VARIABLE_IS_TYPE_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian)
