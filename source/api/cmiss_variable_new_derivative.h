/*******************************************************************************
FILE : api/cmiss_variable_new_derivative.h

LAST MODIFIED : 14 November 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new derivative object.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_NEW_DERIVATIVE_H__
#define __API_CMISS_VARIABLE_NEW_DERIVATIVE_H__

#include "api/cmiss_variable_new.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_variable_new_id Cmiss_variable_new_derivative_create(
	Cmiss_variable_new_id dependent_variable,
	Cmiss_variable_new_input_list_id independent_variables);
/*******************************************************************************
LAST MODIFIED : 14 November 2003

DESCRIPTION :
Creates a Cmiss_variable_new derivative with the supplied <dependent_variable>
and <independent_variables>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_VARIABLE_NEW_DERIVATIVE_H__ */
