/*******************************************************************************
FILE : computed_variable_identity.c

LAST MODIFIED : 20 July 2003

DESCRIPTION :
Implements the identity computed variable - takes a variable to itself.  Used
for calculating derivatives.

???DB.  Needs to know about composite?
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
#include "computed_variable/computed_value_derivative_matrix.h"
#include "computed_variable/computed_value_matrix.h"
#include "computed_variable/computed_variable_identity.h"
#include "computed_variable/computed_variable_private.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Module constants
----------------
*/
static char Cmiss_variable_identity_type_string[]="Identity";

/*
Module types
------------
*/
struct Cmiss_variable_identity_type_specific_data
/*******************************************************************************
LAST MODIFIED : 17 July 2003

DESCRIPTION :
Takes the <variable> too itself.
==============================================================================*/
{
	Cmiss_value_id value;
	Cmiss_variable_id variable;
}; /* struct Cmiss_variable_identity_type_specific_data */

/*
Module functions
----------------
*/

/*
Module methods
--------------
*/
static START_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(identity)
{
	DEACCESS(Cmiss_value)(&(data->value));
	DEACCESS(Cmiss_variable)(&(data->variable));
	return_code=1;
}
END_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(identity)

static START_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(identity)
{
	if (destination->value=CREATE(Cmiss_value)())
	{
		ACCESS(Cmiss_value)(destination->value);
		if (Cmiss_value_copy(destination->value,source->value))
		{
			ACCESS(Cmiss_value)(destination->value);
			destination->variable=ACCESS(Cmiss_variable)(source->variable);
		}
		else
		{
			DEALLOCATE(destination);
		}
		DEACCESS(Cmiss_value)(&(destination->value));
	}
	else
	{
		DEALLOCATE(destination);
	}
}
END_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(identity)

static START_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	identity)
{
	Cmiss_value_id *matrices,*matrix,value_type;
	Cmiss_variable_id independent_variable,*independent_variable_address;
	int i,j,number_of_columns,number_of_matrices,number_of_rows,number_of_values,
		offset;
	struct Cmiss_variable_identity_type_specific_data *data;
	struct Matrix *matrix_local;

	data=(struct Cmiss_variable_identity_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->variable)&&(data->value),return_code,0)
	{
		/* get number of rows in derivative matrix */
		if ((Cmiss_value_get_reals(data->value,&number_of_rows,(FE_value **)NULL))&&
			(0<number_of_rows))
		{
			if (value_type=CREATE(Cmiss_value)())
			{
				/* set up the matrices in the derivative */
				i=order;
				number_of_matrices=0;
				matrices=(Cmiss_value_id *)NULL;
				independent_variable_address=independent_variables;
				return_code=1;
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
											if (Cmiss_variable_same_variable(data->variable,
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
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(identity)

static START_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(identity)
{
	struct Cmiss_variable_identity_type_specific_data *data;

	data=(struct Cmiss_variable_identity_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->variable)&&(data->value),return_code,0)
	{
		return_code=Cmiss_value_copy(value,data->value);
	}
}
END_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(identity)

static
	START_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	identity)
{
	struct Cmiss_variable_identity_type_specific_data *data;

	data=(struct Cmiss_variable_identity_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->variable)&&(data->value),return_code,0)
	{
		if (Cmiss_variable_same_variable(data->variable,independent_variable))
		{
			return_code=Cmiss_value_copy(value,data->value);
		}
	}
}
END_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	identity)

static
	START_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	identity)
{
	Cmiss_value_id temp_value;
	struct Cmiss_variable_identity_type_specific_data *data;

	data=(struct Cmiss_variable_identity_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->variable)&&(data->value),return_code,0)
	{
		if (Cmiss_variable_same_variable(data->variable,independent_variable))
		{
			if (temp_value=CREATE(Cmiss_value)())
			{
				ACCESS(Cmiss_value)(temp_value);
				if (Cmiss_value_copy(temp_value,data->value))
				{
					if (Cmiss_value_copy(data->value,value))
					{
						if (Cmiss_value_copy(value,temp_value))
						{
							return_code=1;
						}
						else
						{
							Cmiss_value_copy(value,temp_value);
						}
					}
				}
				DEACCESS(Cmiss_value)(&(temp_value));
			}
		}
	}
}
END_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	identity)

static START_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(identity)
{
	struct Cmiss_variable_identity_type_specific_data *data;

	data=(struct Cmiss_variable_identity_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->variable)&&(data->value),return_code,0)
	{
		return_code=Cmiss_value_copy(type,data->value);
	}
}
END_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(identity)

static START_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(identity)
{
	return_code=1;
}
END_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(identity)

static START_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(
	identity)
{
	struct Cmiss_variable_identity_type_specific_data *data;

	data=(struct Cmiss_variable_identity_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->variable)&&(data->value),return_code,0)
	{
		return_code=Cmiss_variable_same_variable(data->variable,
			independent_variable);
	}
}
END_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(identity)

static START_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(identity)
{
	struct Cmiss_variable_identity_type_specific_data *data;

	data=(struct Cmiss_variable_identity_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->variable)&&(data->value),return_code,0)
	{
		return_code=Cmiss_variable_not_in_use(data->variable);
	}
}
END_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(identity)

static START_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(identity)
{
	return_code=Cmiss_variable_same_variable(variable_1,variable_2);
}
END_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(identity)

static START_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(identity)
{
	struct Cmiss_variable_identity_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_variable_identity_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_1);
	data_2=(struct Cmiss_variable_identity_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&(data_1->variable)&&(data_1->value)&&data_2&&
		(data_2->variable)&&(data_2->value),return_code,0)
	{
		return_code=Cmiss_variable_same_variable(data_1->variable,data_2->variable);
	}
}
END_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(identity)

static
	START_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	identity)
{
	struct Cmiss_variable_identity_type_specific_data *data;

	data=(struct Cmiss_variable_identity_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->variable)&&(data->value),return_code,0)
	{
		if (Cmiss_variable_same_variable(data->variable,independent_variable))
		{
			return_code=Cmiss_value_copy(data->value,value);
		}
	}
}
END_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(
	identity)

/*
Global functions
----------------
*/
int Cmiss_variable_identity_set_type(Cmiss_variable_id identity,
	Cmiss_variable_id variable)
/*******************************************************************************
LAST MODIFIED : 17 July 2003

DESCRIPTION :
Sets <identity> to be a variable whose dependent variable is <variable> and
whose independent variable is <variable>.
==============================================================================*/
{
	Cmiss_value_id value;
	int return_code;
	struct Cmiss_variable_identity_type_specific_data *data;

	ENTER(Cmiss_variable_identity_set_type);
	return_code=0;
	/* check arguments */
	if (identity&&variable)
	{
		if (value=CREATE(Cmiss_value)())
		{
			ACCESS(Cmiss_value)(value);
			if (Cmiss_variable_get_value_type(variable,value))
			{
				/* 1.  Make dynamic allocations for any new type-specific data */
				if (ALLOCATE(data,struct Cmiss_variable_identity_type_specific_data,1))
				{
					/* 2.  Clear current type-specific data */
					Cmiss_variable_clear_type(identity);
					/* 3.  Establish the new type */
					if (return_code=Cmiss_variable_set_type_specific_information(
						identity,Cmiss_variable_identity_type_string,(void *)data))
					{
						data->variable=ACCESS(Cmiss_variable)(variable);
						data->value=ACCESS(Cmiss_value)(value);
						/* set all the methods */
						return_code=CMISS_VARIABLE_ESTABLISH_METHODS(identity,identity);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Cmiss_variable_identity_set_type.  "
							"Could not set type specific information");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Cmiss_variable_identity_set_type.  "
						"Could not ALLOCATE type specific data");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Cmiss_variable_identity_set_type.  "
					"Could not get variable value type");
			}
			DEACCESS(Cmiss_value)(&value);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_variable_identity_set_type.  "
				"Could not create value");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_identity_set_type.  "
			"Invalid argument(s).  %p %p\n",identity,variable);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_identity_set_type */

DECLARE_CMISS_VARIABLE_IS_TYPE_FUNCTION(identity)
