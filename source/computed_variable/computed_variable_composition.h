/*******************************************************************************
FILE : computed_variable_composition.h

LAST MODIFIED : 14 July 2003

DESCRIPTION :
Implements the composition computed variable - independent variables, for a
dependent variable, are calculated from source variables.
==============================================================================*/
#if !defined (__CMISS_VARIABLE_COMPOSITION_H__)
#define __CMISS_VARIABLE_COMPOSITION_H__

#include "computed_variable/computed_variable.h"

/*
Global functions
----------------
*/
int Cmiss_variable_composition_set_type(Cmiss_variable_id composition,
	Cmiss_variable_id dependent_variable,int number_of_source_variables,
	Cmiss_variable_id *source_variables,Cmiss_variable_id *independent_variables);
/*******************************************************************************
LAST MODIFIED : 10 July 2003

DESCRIPTION :
Sets <composition> to be the <dependent_variable> with its
<independent_variables> calculated using the corresponding <source_variables>.

This function ACCESSes the <dependent_variable>, <source_variables> and
<independent_variables>.  After success, the <composition> is responsible for
DEACCESS/DEALLOCATEing <dependent_variable>, <source_variables> and
<independent_variables>.
==============================================================================*/

PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(composition);

#endif /* !defined (__CMISS_VARIABLE_COMPOSITION_H__) */
