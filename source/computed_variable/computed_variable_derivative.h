/*******************************************************************************
FILE : computed_variable_derivative.h

LAST MODIFIED : 9 July 2003

DESCRIPTION :
Implements the derivative computed variable.

???DB.  divergence and inverse are here to get them out of computed_variable and
	they will end up in their own modules
==============================================================================*/
#if !defined (__CMISS_VARIABLE_DERIVATIVE_H__)
#define __CMISS_VARIABLE_DERIVATIVE_H__

#include "computed_variable/computed_value.h"
#include "computed_variable/computed_variable.h"

/*
Global functions
----------------
*/
int Cmiss_variable_derivative_set_type(Cmiss_variable_id derivative,
	Cmiss_variable_id dependent_variable,int order,
	Cmiss_variable_id *independent_variables);
/*******************************************************************************
LAST MODIFIED : 21 March 2003

DESCRIPTION :
Sets <derivative> to be the derivative of the <dependent_variable> with respect
to the <independent_variables>.  This function ACCESSes the <dependent_variable>
and <independent_variables>.  After success, the <derivative> is responsible for
DEACCESS/DEALLOCATEing <dependent_variable> and <independent_variables>.
==============================================================================*/

int Cmiss_variable_divergence_set_type(Cmiss_variable_id divergence,
	Cmiss_variable_id dependent_variable,Cmiss_variable_id independent_variable);
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Sets <divergence> to be the divergence of the <dependent_variable> with respect
to the <independent_variable>.
==============================================================================*/

int Cmiss_variable_inverse_set_type(Cmiss_variable_id inverse_variable,
	Cmiss_variable_id variable,struct LIST(Cmiss_variable) *dependent_variables);
/*******************************************************************************
LAST MODIFIED : 24 January 2003

DESCRIPTION :
Sets <inverse_variable> to be the inverse of the <variable>.  Its independent
variables are the dependent variables of the <variable> and its
<dependent_variables> are independent variables of the <variable>.
==============================================================================*/
#endif /* !defined (__CMISS_VARIABLE_DERIVATIVE_H__) */
