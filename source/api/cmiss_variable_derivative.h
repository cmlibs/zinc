/*******************************************************************************
FILE : api/cmiss_variable_derivative.h

LAST MODIFIED : 13 August 2003

DESCRIPTION :
The public interface to the Cmiss_variable_derivative object.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_DERIVATIVE_H__
#define __API_CMISS_VARIABLE_DERIVATIVE_H__

/* If this is going to be in the API then it needs to have an interface there */
#include "general/object.h"
#include "api/cmiss_variable.h"

Cmiss_variable_id CREATE(Cmiss_variable_derivative)(char *name,
	Cmiss_variable_id dependent_variable, int order,
	Cmiss_variable_id *independent_variables);
/*******************************************************************************
LAST MODIFIED : 14 August 2003

DESCRIPTION :
Creates a Cmiss_variable which represents the derivative of the dependent variable
with respect to the independent variables.  There must be <order> variables in
the <independent_variables> array.
==============================================================================*/
#endif /* __API_CMISS_VARIABLE_DERIVATIVE_H__ */
