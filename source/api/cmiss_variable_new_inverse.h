/*******************************************************************************
FILE : api/cmiss_variable_new_inverse.h

LAST MODIFIED : 16 December 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new inverse object.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_NEW_INVERSE_H__
#define __API_CMISS_VARIABLE_NEW_INVERSE_H__

#include "api/cmiss_variable_new.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_variable_new_id Cmiss_variable_new_inverse_create(
	Cmiss_variable_new_input_id dependent_variable,
	Cmiss_variable_new_id independent_variable);
/*******************************************************************************
LAST MODIFIED : 11 December 2003

DESCRIPTION :
Creates a Cmiss_variable_new inverse with the supplied <dependent_variable>
and <independent_variable>.
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_inverse_independent(
	Cmiss_variable_new_id variable_inverse);
/*******************************************************************************
LAST MODIFIED : 11 December 2003

DESCRIPTION :
Returns the independent input for the <variable_inverse>.
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_inverse_step_tolerance(
	Cmiss_variable_new_id variable_inverse);
/*******************************************************************************
LAST MODIFIED : 16 December 2003

DESCRIPTION :
Returns the step tolerance input for the <variable_inverse>.
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_inverse_value_tolerance(
	Cmiss_variable_new_id variable_inverse);
/*******************************************************************************
LAST MODIFIED : 16 December 2003

DESCRIPTION :
Returns the value tolerance input for the <variable_inverse>.
==============================================================================*/

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_inverse_maximum_iterations(
	Cmiss_variable_new_id variable_inverse);
/*******************************************************************************
LAST MODIFIED : 11 December 2003

DESCRIPTION :
Returns the maximum_iterations input for the <variable_inverse>.
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_inverse_dependent_estimate(
	Cmiss_variable_new_id variable_inverse);
/*******************************************************************************
LAST MODIFIED : 11 December 2003

DESCRIPTION :
Returns the dependent_estimate input for the <variable_inverse>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_VARIABLE_NEW_INVERSE_H__ */
