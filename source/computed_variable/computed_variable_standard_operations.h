/*******************************************************************************
FILE : computed_variable_standard_operations.h

LAST MODIFIED : 3 August 2003

DESCRIPTION :
Implements computed variables which carry out standard operations.
==============================================================================*/
#if !defined (__CMISS_VARIABLE_STANDARD_OPERATIONS_H__)
#define __CMISS_VARIABLE_STANDARD_OPERATIONS_H__

#include "computed_variable/computed_variable.h"

/*
Global functions
----------------
*/
int Cmiss_variable_dot_product_input_1_set_type(Cmiss_variable_id variable,
	int dimension);
/*******************************************************************************
LAST MODIFIED : 3 August 2003

DESCRIPTION :
Converts the <variable> into the first input for a dot product (=1.2).

Only used to name independent variables and so can't be evaluated.
==============================================================================*/

PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(dot_product_input_1);

int Cmiss_variable_dot_product_input_2_set_type(Cmiss_variable_id variable,
	int dimension);
/*******************************************************************************
LAST MODIFIED : 3 August 2003

DESCRIPTION :
Converts the <variable> into the second input for a dot product (=1.2).

Only used to name independent variables and so can't be evaluated.
==============================================================================*/

PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(dot_product_input_2);

int Cmiss_variable_dot_product_set_type(Cmiss_variable_id variable,
	int dimension);
/*******************************************************************************
LAST MODIFIED : 3 August 2003

DESCRIPTION :
Converts the <variable> into a variable that calculates the dot product of its
inputs.

Independent variables are: dot_product_input_1, dot_product_input_2.
==============================================================================*/

PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(dot_product);

#endif /* !defined (__CMISS_VARIABLE_STANDARD_OPERATIONS_H__) */
