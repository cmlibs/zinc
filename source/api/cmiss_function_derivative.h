/*******************************************************************************
FILE : api/cmiss_function_derivative.h

LAST MODIFIED : 9 June 2004

DESCRIPTION :
The public interface to the Cmiss_function derivative object.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_DERIVATIVE_H__
#define __API_CMISS_FUNCTION_DERIVATIVE_H__

#include "api/cmiss_function.h"
#include "api/cmiss_function_variable.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_id Cmiss_function_derivative_create(
	Cmiss_function_variable_id dependent_variable,
	Cmiss_function_variable_list_id independent_variables);
/*******************************************************************************
LAST MODIFIED : 9 June 2004

DESCRIPTION :
Creates a Cmiss_function derivative with the supplied <dependent_variable>
and <independent_variables>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_DERIVATIVE_H__ */
