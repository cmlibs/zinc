/*******************************************************************************
FILE : computed_variable_standard_operations.c

LAST MODIFIED : 28 July 2003

DESCRIPTION :
Implements computed variables which carry out standard_operations.

???DB.  All of Dot_product_input_* could be shared with Coordinates and
	Spheroidal_coordinates_focus via a macro(s).
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#include <math.h>
#include "computed_variable/computed_value_derivative_matrix.h"
#include "computed_variable/computed_value_fe_value.h"
#include "computed_variable/computed_value_matrix.h"
#include "computed_variable/computed_variable_standard_operations.h"
#include "computed_variable/computed_variable_private.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Module methods
--------------
*/
static char Cmiss_variable_dot_product_input_1_type_string[]=
	"Dot_product_input_1";

struct Cmiss_variable_dot_product_input_1_type_specific_data
/*******************************************************************************
LAST MODIFIED : 27 July 2003

DESCRIPTION :
==============================================================================*/
{
	Cmiss_value_id fe_value_vector;
}; /* struct Cmiss_variable_dot_product_input_1_type_specific_data */

static START_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(dot_product_input_1)
{
	DEACCESS(Cmiss_value)(&(data->fe_value_vector));
	return_code=1;
}
END_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(dot_product_input_1)

static START_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_1)
{
	if (destination->fe_value_vector=CREATE(Cmiss_value)())
	{
		ACCESS(Cmiss_value)(destination->fe_value_vector);
		if (!Cmiss_value_copy(destination->fe_value_vector,source->fe_value_vector))
		{
			DEACCESS(Cmiss_value)(&(destination->fe_value_vector));
			DEALLOCATE(destination);
		}
	}
	else
	{
		DEALLOCATE(destination);
	}
}
END_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(dot_product_input_1)

static START_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_1)
{
	Cmiss_value_id *matrices,*matrix,value_type;
	Cmiss_variable_id independent_variable,*independent_variable_address;
	int i,j,number_of_columns,number_of_matrices,number_of_rows,number_of_values,
		offset;
	struct Cmiss_variable_dot_product_input_1_type_specific_data *data;
	struct Matrix *matrix_local;

	data=(struct Cmiss_variable_dot_product_input_1_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->fe_value_vector),
		return_code,0)
	{
		/* get number of rows in derivative matrix */
		if (Cmiss_value_FE_value_vector_get_type(data->fe_value_vector,
			&number_of_rows,(FE_value **)NULL)&&(0<number_of_rows))
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
							"Cmiss_variable_dot_product_input_1_evaluate_derivative_type_specific.  "
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
END_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_1)

static START_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(dot_product_input_1)
{
	struct Cmiss_variable_dot_product_input_1_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_input_1_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->fe_value_vector),
		return_code,0)
	{
		if (Cmiss_value_copy(value,data->fe_value_vector))
		{
			return_code=0;
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(dot_product_input_1)

static
	START_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_1)
{
	struct Cmiss_variable_dot_product_input_1_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_input_1_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->fe_value_vector),
		return_code,0)
	{
		if (Cmiss_variable_same_variable(dependent_variable,
			independent_variable))
		{
			return_code=Cmiss_value_copy(value,data->fe_value_vector);
		}
	}
}
END_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_1)

static
	START_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_1)
{
	Cmiss_value_id temp_value;
	struct Cmiss_variable_dot_product_input_1_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_input_1_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->fe_value_vector),
		return_code,0)
	{
		if (Cmiss_variable_same_variable(dependent_variable,
			independent_variable)&&CMISS_VALUE_IS_TYPE(FE_value_vector)(value))
		{
			if (temp_value=CREATE(Cmiss_value)())
			{
				if (Cmiss_value_copy(temp_value,data->fe_value_vector))
				{
					if (Cmiss_value_copy(data->fe_value_vector,value))
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
	dot_product_input_1)

static START_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_1)
{
	int number_of_fe_values;
	struct Cmiss_variable_dot_product_input_1_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_input_1_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->fe_value_vector),
		return_code,0)
	{
		if (Cmiss_value_FE_value_vector_get_type(data->fe_value_vector,
			&number_of_fe_values,(FE_value **)NULL))
		{
			return_code=Cmiss_value_FE_value_vector_set_type(type,number_of_fe_values,
				(FE_value *)NULL);
		}
	}
}
END_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(dot_product_input_1)

static START_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_1)
{
	FE_value *fe_values;
	int number_of_fe_values;
	struct Cmiss_variable_dot_product_input_1_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_input_1_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->fe_value_vector),
		return_code,0)
	{
		if (Cmiss_value_FE_value_vector_get_type(data->fe_value_vector,
			&number_of_fe_values,&fe_values)&&(0<number_of_fe_values)&&fe_values)
		{
			return_code=1;
		}
	}
}
END_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(dot_product_input_1)

static
	START_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_1)
{
	return_code=Cmiss_variable_same_variable(dependent_variable,
		independent_variable);
}
END_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_1)

static START_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_1)
{
	return_code=1;
}
END_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(dot_product_input_1)

static START_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(dot_product_input_1)
{
	return_code=Cmiss_variable_same_variable(variable_1,variable_2);
}
END_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(dot_product_input_1)

static START_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_1)
{
	int number_of_fe_values_1,number_of_fe_values_2;
	struct Cmiss_variable_dot_product_input_1_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_variable_dot_product_input_1_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_1);
	data_2=(struct Cmiss_variable_dot_product_input_1_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&CMISS_VALUE_IS_TYPE(FE_value_vector)(
		data_1->fe_value_vector)&&data_2&&CMISS_VALUE_IS_TYPE(FE_value_vector)(
		data_2->fe_value_vector),return_code,0)
	{
		if (Cmiss_value_FE_value_vector_get_type(data_1->fe_value_vector,
			&number_of_fe_values_1,(FE_value **)NULL)&&
			Cmiss_value_FE_value_vector_get_type(data_2->fe_value_vector,
			&number_of_fe_values_2,(FE_value **)NULL)&&
			(number_of_fe_values_1==number_of_fe_values_2))
		{
			return_code=1;
		}
	}
}
END_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(dot_product_input_1)

static
	START_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_1)
{
	struct Cmiss_variable_dot_product_input_1_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_input_1_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->fe_value_vector),
		return_code,0)
	{
		if (Cmiss_variable_same_variable(dependent_variable,
			independent_variable))
		{
			/*???DB.  Copy into <independent_variable> as well? */
			return_code=Cmiss_value_copy(data->fe_value_vector,value);
		}
	}
}
END_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_1)

static char Cmiss_variable_dot_product_input_2_type_string[]=
	"Dot_product_input_2";

struct Cmiss_variable_dot_product_input_2_type_specific_data
/*******************************************************************************
LAST MODIFIED : 27 July 2003

DESCRIPTION :
==============================================================================*/
{
	Cmiss_value_id fe_value_vector;
}; /* struct Cmiss_variable_dot_product_input_2_type_specific_data */

static START_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(dot_product_input_2)
{
	DEACCESS(Cmiss_value)(&(data->fe_value_vector));
	return_code=1;
}
END_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(dot_product_input_2)

static START_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_2)
{
	if (destination->fe_value_vector=CREATE(Cmiss_value)())
	{
		ACCESS(Cmiss_value)(destination->fe_value_vector);
		if (!Cmiss_value_copy(destination->fe_value_vector,source->fe_value_vector))
		{
			DEACCESS(Cmiss_value)(&(destination->fe_value_vector));
			DEALLOCATE(destination);
		}
	}
	else
	{
		DEALLOCATE(destination);
	}
}
END_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(dot_product_input_2)

static START_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_2)
{
	Cmiss_value_id *matrices,*matrix,value_type;
	Cmiss_variable_id independent_variable,*independent_variable_address;
	int i,j,number_of_columns,number_of_matrices,number_of_rows,number_of_values,
		offset;
	struct Cmiss_variable_dot_product_input_2_type_specific_data *data;
	struct Matrix *matrix_local;

	data=(struct Cmiss_variable_dot_product_input_2_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->fe_value_vector),
		return_code,0)
	{
		/* get number of rows in derivative matrix */
		if (Cmiss_value_FE_value_vector_get_type(data->fe_value_vector,
			&number_of_rows,(FE_value **)NULL)&&(0<number_of_rows))
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
							"Cmiss_variable_dot_product_input_2_evaluate_derivative_type_specific.  "
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
END_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_2)

static START_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(dot_product_input_2)
{
	struct Cmiss_variable_dot_product_input_2_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_input_2_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->fe_value_vector),
		return_code,0)
	{
		if (Cmiss_value_copy(value,data->fe_value_vector))
		{
			return_code=0;
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(dot_product_input_2)

static
	START_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_2)
{
	struct Cmiss_variable_dot_product_input_2_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_input_2_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->fe_value_vector),
		return_code,0)
	{
		if (Cmiss_variable_same_variable(dependent_variable,
			independent_variable))
		{
			return_code=Cmiss_value_copy(value,data->fe_value_vector);
		}
	}
}
END_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_2)

static
	START_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_2)
{
	Cmiss_value_id temp_value;
	struct Cmiss_variable_dot_product_input_2_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_input_2_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->fe_value_vector),
		return_code,0)
	{
		if (Cmiss_variable_same_variable(dependent_variable,
			independent_variable)&&CMISS_VALUE_IS_TYPE(FE_value_vector)(value))
		{
			if (temp_value=CREATE(Cmiss_value)())
			{
				if (Cmiss_value_copy(temp_value,data->fe_value_vector))
				{
					if (Cmiss_value_copy(data->fe_value_vector,value))
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
	dot_product_input_2)

static START_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_2)
{
	int number_of_fe_values;
	struct Cmiss_variable_dot_product_input_2_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_input_2_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->fe_value_vector),
		return_code,0)
	{
		if (Cmiss_value_FE_value_vector_get_type(data->fe_value_vector,
			&number_of_fe_values,(FE_value **)NULL))
		{
			return_code=Cmiss_value_FE_value_vector_set_type(type,number_of_fe_values,
				(FE_value *)NULL);
		}
	}
}
END_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(dot_product_input_2)

static START_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_2)
{
	FE_value *fe_values;
	int number_of_fe_values;
	struct Cmiss_variable_dot_product_input_2_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_input_2_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->fe_value_vector),
		return_code,0)
	{
		if (Cmiss_value_FE_value_vector_get_type(data->fe_value_vector,
			&number_of_fe_values,&fe_values)&&(0<number_of_fe_values)&&fe_values)
		{
			return_code=1;
		}
	}
}
END_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(dot_product_input_2)

static
	START_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_2)
{
	return_code=Cmiss_variable_same_variable(dependent_variable,
		independent_variable);
}
END_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_2)

static START_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_2)
{
	return_code=1;
}
END_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(dot_product_input_2)

static START_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(dot_product_input_2)
{
	return_code=Cmiss_variable_same_variable(variable_1,variable_2);
}
END_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(dot_product_input_2)

static START_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_2)
{
	int number_of_fe_values_1,number_of_fe_values_2;
	struct Cmiss_variable_dot_product_input_2_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_variable_dot_product_input_2_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_1);
	data_2=(struct Cmiss_variable_dot_product_input_2_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&CMISS_VALUE_IS_TYPE(FE_value_vector)(
		data_1->fe_value_vector)&&data_2&&CMISS_VALUE_IS_TYPE(FE_value_vector)(
		data_2->fe_value_vector),return_code,0)
	{
		if (Cmiss_value_FE_value_vector_get_type(data_1->fe_value_vector,
			&number_of_fe_values_1,(FE_value **)NULL)&&
			Cmiss_value_FE_value_vector_get_type(data_2->fe_value_vector,
			&number_of_fe_values_2,(FE_value **)NULL)&&
			(number_of_fe_values_1==number_of_fe_values_2))
		{
			return_code=1;
		}
	}
}
END_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(dot_product_input_2)

static
	START_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_2)
{
	struct Cmiss_variable_dot_product_input_2_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_input_2_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&CMISS_VALUE_IS_TYPE(FE_value_vector)(data->fe_value_vector),
		return_code,0)
	{
		if (Cmiss_variable_same_variable(dependent_variable,
			independent_variable))
		{
			/*???DB.  Copy into <independent_variable> as well? */
			return_code=Cmiss_value_copy(data->fe_value_vector,value);
		}
	}
}
END_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	dot_product_input_2)

static char Cmiss_variable_dot_product_type_string[]="Dot_product";

struct Cmiss_variable_dot_product_type_specific_data
/*******************************************************************************
LAST MODIFIED : 3 August 2003

DESCRIPTION :
==============================================================================*/
{
	Cmiss_value_id fe_value_vector_1,fe_value_vector_2;
}; /* struct Cmiss_variable_dot_product_type_specific_data */

static START_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(dot_product)
{
	DEACCESS(Cmiss_value)(&(data->fe_value_vector_1));
	DEACCESS(Cmiss_value)(&(data->fe_value_vector_2));
	return_code=1;
}
END_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(dot_product)

static START_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(dot_product)
{
	if (destination->fe_value_vector_1=CREATE(Cmiss_value)())
	{
		ACCESS(Cmiss_value)(destination->fe_value_vector_1);
		if (destination->fe_value_vector_2=CREATE(Cmiss_value)())
		{
			ACCESS(Cmiss_value)(destination->fe_value_vector_2);
			if (!(Cmiss_value_copy(destination->fe_value_vector_1,
				source->fe_value_vector_1)&&Cmiss_value_copy(
				destination->fe_value_vector_2,source->fe_value_vector_2)))
			{
				DEACCESS(Cmiss_value)(&(destination->fe_value_vector_1));
				DEACCESS(Cmiss_value)(&(destination->fe_value_vector_2));
				DEALLOCATE(destination);
			}
		}
		else
		{
			DEACCESS(Cmiss_value)(&(destination->fe_value_vector_1));
			DEALLOCATE(destination);
		}
	}
	else
	{
		DEALLOCATE(destination);
	}
}
END_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(dot_product)

enum Cmiss_variable_evaluate_derivative_dot_product_type
{
	VECTOR_INPUT_1_TYPE,
	VECTOR_INPUT_2_TYPE,
	ZERO_DERIVATIVE_TYPE
}; /* enum Cmiss_variable_evaluate_derivative_dot_product_type */

static START_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	dot_product)
/*******************************************************************************
LAST MODIFIED : 3 August 2003

DESCRIPTION :
==============================================================================*/
{
	Cmiss_value_id *matrices,*matrix,value_type;
	Cmiss_variable_id independent_variable,*local_independent_variables;
	enum Cmiss_variable_evaluate_derivative_dot_product_type
		*independent_variables_type;
	FE_value *input_vector_values;
	int *derivative_independent_variables,derivative_number_of_values,
		derivative_order,i,j,independent_variables_index,
		*independent_variables_number_of_values,number_of_components,
		number_of_matrices,number_of_values,vector_input_1_order,
		vector_input_2_order,zero_derivative;
	struct Cmiss_variable_dot_product_type_specific_data *data;
	struct Matrix *matrix_local;

	data=(struct Cmiss_variable_dot_product_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data,return_code,0)
	{
		/* set up temporary storage */
		value_type=CREATE(Cmiss_value)();
		ALLOCATE(independent_variables_number_of_values,int,order);
		ALLOCATE(independent_variables_type,
			enum Cmiss_variable_evaluate_derivative_dot_product_type,
			order);
		ALLOCATE(derivative_independent_variables,int,order+1);
		if (value_type&&independent_variables_number_of_values&&
			independent_variables_type&&derivative_independent_variables)
		{
			return_code=1;
			/* get number of rows in derivative matrix (<number_of_components>) */
			number_of_components=1;
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
				if (CMISS_VARIABLE_IS_TYPE(dot_product_input_1)(independent_variable))
				{
					/* differentiating with respect to input */
					independent_variables_type[independent_variables_index]=
						VECTOR_INPUT_1_TYPE;
				}
				else if (CMISS_VARIABLE_IS_TYPE(dot_product_input_2)(
					independent_variable))
				{
					/* differentiating with respect to input */
					independent_variables_type[independent_variables_index]=
						VECTOR_INPUT_2_TYPE;
				}
				else
				{
					independent_variables_type[independent_variables_index]=
						ZERO_DERIVATIVE_TYPE;
				}
				if (return_code=Cmiss_variable_get_value_type(independent_variable,
					value_type))
				{
					return_code=Cmiss_value_get_reals(value_type,&number_of_values,
						(FE_value **)NULL);
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
							vector_input_1_order=0;
							vector_input_2_order=0;
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
										case VECTOR_INPUT_1_TYPE:
										{
											vector_input_1_order++;
										} break;
										case VECTOR_INPUT_2_TYPE:
										{
											vector_input_2_order++;
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
							if ((vector_input_1_order>1)||(vector_input_2_order>1))
							{
								zero_derivative=1;
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
							if (return_code&&!zero_derivative)
							{
								if (1==vector_input_1_order)
								{
									if (1==vector_input_2_order)
									{
										/* second derivative */
										i=1;
										j=1;
										while (return_code&&(i<=number_of_values))
										{
											return_code=Cmiss_value_matrix_set_value(*matrix,1,j,1);
											i++;
											j += number_of_values+1;
										}
									}
									else
									{
										/* first derivative */
										return_code=Cmiss_value_FE_value_vector_get_type(
											data->fe_value_vector_2,(int *)NULL,&input_vector_values);
										i=0;
										while (return_code&&(i<number_of_values))
										{
											return_code=Cmiss_value_matrix_set_value(*matrix,1,i+1,
												(Matrix_value)(input_vector_values[i]));
											i++;
										}
									}
								}
								else if (1==vector_input_2_order)
								{
									/* first derivative */
									return_code=Cmiss_value_FE_value_vector_get_type(
										data->fe_value_vector_1,(int *)NULL,&input_vector_values);
									i=0;
									while (return_code&&(i<number_of_values))
									{
										return_code=Cmiss_value_matrix_set_value(*matrix,1,i+1,
											(Matrix_value)(input_vector_values[i]));
										i++;
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
							"Cmiss_variable_dot_product_evaluate_derivative_type_specific.  "
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
						"Cmiss_variable_dot_product_evaluate_derivative_type_specific.  "
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
				"Cmiss_variable_dot_product_evaluate_derivative_type_specific.  "
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
END_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(dot_product)

static START_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(dot_product)
{
	FE_value fe_value,*input_vector_values_1,*input_vector_values_2;
	int i,number_of_values_1,number_of_values_2;
	struct Cmiss_variable_dot_product_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data,return_code,0)
	{
		if (Cmiss_value_FE_value_vector_get_type(data->fe_value_vector_1,
			&number_of_values_1,&input_vector_values_1)&&
			Cmiss_value_FE_value_vector_get_type(data->fe_value_vector_2,
			&number_of_values_2,&input_vector_values_2)&&
			(number_of_values_1==number_of_values_2))
		{
			fe_value=0;
			for (i=0;i<number_of_values_1;i++)
			{
				fe_value += input_vector_values_1[i]*input_vector_values_2[i];
			}
			return_code=Cmiss_value_FE_value_set_type(value,fe_value);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_dot_product_evaluate_type_specific.  "
				"Invalid or missing vectors");
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(dot_product)

static
	START_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	dot_product)
{
	struct Cmiss_variable_dot_product_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data,return_code,0)
	{
		if (CMISS_VARIABLE_IS_TYPE(dot_product_input_1)(independent_variable))
		{
			return_code=Cmiss_value_copy(value,data->fe_value_vector_1);
		}
		else if (CMISS_VARIABLE_IS_TYPE(dot_product_input_2)(independent_variable))
		{
			return_code=Cmiss_value_copy(value,data->fe_value_vector_2);
		}
	}
}
END_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	dot_product)

static
	START_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	dot_product)
{
	Cmiss_value_id temp_value;
	struct Cmiss_variable_dot_product_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data,return_code,0)
	{
		if (CMISS_VARIABLE_IS_TYPE(dot_product_input_1)(independent_variable))
		{
			if (temp_value=CREATE(Cmiss_value)())
			{
				if (Cmiss_value_copy(temp_value,data->fe_value_vector_1))
				{
					if (Cmiss_value_copy(data->fe_value_vector_1,value))
					{
						return_code=Cmiss_value_copy(value,temp_value);
					}
				}
				DESTROY(Cmiss_value)(&temp_value);
			}
		}
		else if (CMISS_VARIABLE_IS_TYPE(dot_product_input_2)(independent_variable))
		{
			if (temp_value=CREATE(Cmiss_value)())
			{
				if (Cmiss_value_copy(temp_value,data->fe_value_vector_2))
				{
					if (Cmiss_value_copy(data->fe_value_vector_2,value))
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
	dot_product)

static START_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(dot_product)
{
	struct Cmiss_variable_dot_product_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&
		CMISS_VALUE_IS_TYPE(FE_value_vector)(data->fe_value_vector_1)&&
		CMISS_VALUE_IS_TYPE(FE_value_vector)(data->fe_value_vector_2),return_code,0)
	{
		return_code=Cmiss_value_FE_value_set_type(type,(FE_value)0);
	}
}
END_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(dot_product)

static START_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(dot_product)
{
	struct Cmiss_variable_dot_product_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data,return_code,0)
	{
		return_code=1;
	}
}
END_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(dot_product)

static START_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	dot_product)
{
	if (CMISS_VARIABLE_IS_TYPE(dot_product_input_1)(dependent_variable)||
		CMISS_VARIABLE_IS_TYPE(dot_product_input_2)(dependent_variable))
	{
		return_code=1;
	}
}
END_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	dot_product)

static START_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(dot_product)
{
	struct Cmiss_variable_dot_product_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data,return_code,0)
	{
		return_code=1;
	}
}
END_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(dot_product)

static START_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(dot_product)
{
	return_code=Cmiss_variable_same_variable(variable_1,variable_2);
}
END_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(dot_product)

static START_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(dot_product)
{
	return_code=1;
}
END_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(dot_product)

static
	START_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	dot_product)
{
	struct Cmiss_variable_dot_product_type_specific_data *data;

	data=(struct Cmiss_variable_dot_product_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data,return_code,0)
	{
		if (CMISS_VARIABLE_IS_TYPE(dot_product_input_1)(independent_variable))
		{
			return_code=Cmiss_value_copy(data->fe_value_vector_1,value);
		}
		else if (CMISS_VARIABLE_IS_TYPE(dot_product_input_2)(independent_variable))
		{
			return_code=Cmiss_value_copy(data->fe_value_vector_2,value);
		}
	}
}
END_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	dot_product)

/*
Global functions
----------------
*/
int Cmiss_variable_dot_product_input_1_set_type(Cmiss_variable_id variable,
	int dimension)
/*******************************************************************************
LAST MODIFIED : 3 August 2003

DESCRIPTION :
Converts the <variable> into the first input for a dot product (=1.2).

Only used to name independent variables and so can't be evaluated.
==============================================================================*/
{
	int return_code;
	struct Cmiss_variable_dot_product_input_1_type_specific_data *data;

	ENTER(Cmiss_variable_dot_product_input_1_set_type);
	return_code=0;
	/* check arguments */
	if (variable&&(0<=dimension))
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,
			struct Cmiss_variable_dot_product_input_1_type_specific_data,1))
		{
			if (data->fe_value_vector=CREATE(Cmiss_value)())
			{
				ACCESS(Cmiss_value)(data->fe_value_vector);
				if (Cmiss_value_FE_value_vector_set_type(data->fe_value_vector,
					dimension,(FE_value *)NULL))
				{
					/* 2.  Clear current type-specific data */
					Cmiss_variable_clear_type(variable);
					/* 3.  Establish the new type */
					Cmiss_variable_set_type_specific_information(variable,
						Cmiss_variable_dot_product_input_1_type_string,(void *)data);
					/* set all the methods */
					return_code=CMISS_VARIABLE_ESTABLISH_METHODS(variable,
						dot_product_input_1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_variable_dot_product_input_1_set_type.  "
						"Could not Cmiss_value_FE_value_vector_set_type");
					DEACCESS(Cmiss_value)(&(data->fe_value_vector));
					DEALLOCATE(data);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_dot_product_input_1_set_type.  "
					"Could not CREATE(Cmiss_value)");
				DEALLOCATE(data);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_dot_product_input_1_set_type.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_dot_product_input_1_set_type.  "
			"Invalid argument(s).  %p %d\n",variable,dimension);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_dot_product_input_1_set_type */

DECLARE_CMISS_VARIABLE_IS_TYPE_FUNCTION(dot_product_input_1)

int Cmiss_variable_dot_product_input_2_set_type(Cmiss_variable_id variable,
	int dimension)
/*******************************************************************************
LAST MODIFIED : 3 August 2003

DESCRIPTION :
Converts the <variable> into the second input for a dot product (=1.2).

Only used to name independent variables and so can't be evaluated.
==============================================================================*/
{
	int return_code;
	struct Cmiss_variable_dot_product_input_2_type_specific_data *data;

	ENTER(Cmiss_variable_dot_product_input_2_set_type);
	return_code=0;
	/* check arguments */
	if (variable&&(0<=dimension))
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,
			struct Cmiss_variable_dot_product_input_2_type_specific_data,1))
		{
			if (data->fe_value_vector=CREATE(Cmiss_value)())
			{
				ACCESS(Cmiss_value)(data->fe_value_vector);
				if (Cmiss_value_FE_value_vector_set_type(data->fe_value_vector,
					dimension,(FE_value *)NULL))
				{
					/* 2.  Clear current type-specific data */
					Cmiss_variable_clear_type(variable);
					/* 3.  Establish the new type */
					Cmiss_variable_set_type_specific_information(variable,
						Cmiss_variable_dot_product_input_2_type_string,(void *)data);
					/* set all the methods */
					return_code=CMISS_VARIABLE_ESTABLISH_METHODS(variable,
						dot_product_input_2);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_variable_dot_product_input_2_set_type.  "
						"Could not Cmiss_value_FE_value_vector_set_type");
					DEACCESS(Cmiss_value)(&(data->fe_value_vector));
					DEALLOCATE(data);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_dot_product_input_2_set_type.  "
					"Could not CREATE(Cmiss_value)");
				DEALLOCATE(data);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_dot_product_input_2_set_type.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_dot_product_input_2_set_type.  "
			"Invalid argument(s).  %p %d\n",variable,dimension);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_dot_product_input_2_set_type */

DECLARE_CMISS_VARIABLE_IS_TYPE_FUNCTION(dot_product_input_2)

int Cmiss_variable_dot_product_set_type(Cmiss_variable_id variable,
	int dimension)
/*******************************************************************************
LAST MODIFIED : 3 August 2003

DESCRIPTION :
Converts the <variable> into a variable that calculates the dot product of its
inputs.

Independent variables are: dot_product_input_1, dot_product_input_2.
==============================================================================*/
{
	int return_code;
	struct Cmiss_variable_dot_product_type_specific_data *data;

	ENTER(Cmiss_variable_dot_product_set_type);
	return_code=0;
	/* check arguments */
	if (variable)
	{
		/* 1.  Make dynamic allocations for any new type-specific data */
		if (ALLOCATE(data,struct Cmiss_variable_dot_product_type_specific_data,1))
		{
			if (data->fe_value_vector_1=CREATE(Cmiss_value)())
			{
				ACCESS(Cmiss_value)(data->fe_value_vector_1);
				if (data->fe_value_vector_2=CREATE(Cmiss_value)())
				{
					ACCESS(Cmiss_value)(data->fe_value_vector_2);
					if (Cmiss_value_FE_value_vector_set_type(data->fe_value_vector_1,
						dimension,(FE_value *)NULL)&&Cmiss_value_FE_value_vector_set_type(
						data->fe_value_vector_2,dimension,(FE_value *)NULL))
					{
						/* 2.  Clear current type-specific data */
						Cmiss_variable_clear_type(variable);
						/* 3.  Establish the new type */
						if (return_code=Cmiss_variable_set_type_specific_information(
							variable,Cmiss_variable_dot_product_type_string,(void *)data))
						{
							/* set all the methods */
							return_code=CMISS_VARIABLE_ESTABLISH_METHODS(variable,
								dot_product);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_variable_dot_product_set_type.  "
								"Could not set type specific information");
							DEACCESS(Cmiss_value)(&(data->fe_value_vector_1));
							DEACCESS(Cmiss_value)(&(data->fe_value_vector_2));
							DEALLOCATE(data);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_variable_dot_product_set_type.  "
							"Could not Cmiss_value_FE_value_vector_set_type");
						DEACCESS(Cmiss_value)(&(data->fe_value_vector_1));
						DEACCESS(Cmiss_value)(&(data->fe_value_vector_2));
						DEALLOCATE(data);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Cmiss_variable_dot_product_set_type.  "
						"Could not fe_value_vector_2");
					DEACCESS(Cmiss_value)(&(data->fe_value_vector_1));
					DEALLOCATE(data);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Cmiss_variable_dot_product_set_type.  "
					"Could not fe_value_vector_1");
				DEALLOCATE(data);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_dot_product_set_type.  "
				"Could not ALLOCATE type specific data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_dot_product_set_type.  "
			"Invalid argument(s).  %p\n",variable);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_dot_product_set_type */

DECLARE_CMISS_VARIABLE_IS_TYPE_FUNCTION(dot_product)
