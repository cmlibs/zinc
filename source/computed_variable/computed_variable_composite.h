/*******************************************************************************
FILE : computed_variable_composite.h

LAST MODIFIED : 17 July 2003

DESCRIPTION :
Implements the composite computed variable - its result is a vector containing
the results of the variables it is made up of.
==============================================================================*/
#if !defined (__CMISS_VARIABLE_COMPOSITE_H__)
#define __CMISS_VARIABLE_COMPOSITE_H__

#include "computed_variable/computed_variable.h"

/*
Global functions
----------------
*/
int Cmiss_variable_composite_set_type(Cmiss_variable_id composite,
	int number_of_variables,Cmiss_variable_id *variables);
/*******************************************************************************
LAST MODIFIED : 14 July 2003

DESCRIPTION :
Sets <composite> to be a variable whose result is a vector containing the
results of the <variables>.

This function ACCESSes the <variables>.  After success, the <composite> is
responsible for DEACCESS/DEALLOCATEing <variables>.
==============================================================================*/

PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(composite);

int Cmiss_variable_composite_get_type(Cmiss_variable_id composite,
	int *number_of_variables_address,Cmiss_variable_id **variables_address);
/*******************************************************************************
LAST MODIFIED : 17 July 2003

DESCRIPTION :
Sets <composite> to be a variable whose result is a vector containing the
results of the <variables>.

This function ACCESSes the <variables>.  After success, the <composite> is
responsible for DEACCESS/DEALLOCATEing <variables>.
==============================================================================*/

#endif /* !defined (__CMISS_VARIABLE_COMPOSITE_H__) */
