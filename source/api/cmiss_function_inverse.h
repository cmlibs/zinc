/*******************************************************************************
FILE : api/cmiss_function_inverse.h

LAST MODIFIED : 23 July 2004

DESCRIPTION :
The public interface to the Cmiss_function inverse object.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_INVERSE_H__
#define __API_CMISS_FUNCTION_INVERSE_H__

#include "api/cmiss_function.h"
#include "api/cmiss_function_variable.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_id Cmiss_function_inverse_create(
	Cmiss_function_variable_id dependent_variable,
	Cmiss_function_variable_id independent_variable);
/*******************************************************************************
LAST MODIFIED : 23 July 2004

DESCRIPTION :
Creates a Cmiss_function inverse with the supplied <dependent_variable>
and <independent_variable>.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_inverse_independent(
	Cmiss_function_id function_inverse);
/*******************************************************************************
LAST MODIFIED : 23 July 2004

DESCRIPTION :
Returns the independent variable for the <function_inverse>.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_inverse_step_tolerance(
	Cmiss_function_id function_inverse);
/*******************************************************************************
LAST MODIFIED : 23 July 2004

DESCRIPTION :
Returns the step tolerance variable for the <function_inverse>.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_inverse_value_tolerance(
	Cmiss_function_id function_inverse);
/*******************************************************************************
LAST MODIFIED : 23 July 2004

DESCRIPTION :
Returns the value tolerance variable for the <function_inverse>.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_inverse_maximum_iterations(
	Cmiss_function_id function_inverse);
/*******************************************************************************
LAST MODIFIED : 23 July 2004

DESCRIPTION :
Returns the maximum_iterations variable for the <function_inverse>.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_inverse_dependent_estimate(
	Cmiss_function_id function_inverse);
/*******************************************************************************
LAST MODIFIED : 23 July 2004

DESCRIPTION :
Returns the dependent_estimate variable for the <function_inverse>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_INVERSE_H__ */
