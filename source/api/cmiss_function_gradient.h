/*******************************************************************************
FILE : api/cmiss_function_gradient.h

LAST MODIFIED : 26 August 2004

DESCRIPTION :
The public interface to the Cmiss_function gradient object.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_GRADIENT_H__
#define __API_CMISS_FUNCTION_GRADIENT_H__

#include "api/cmiss_function.h"
#include "api/cmiss_function_variable.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_id Cmiss_function_gradient_create(
	Cmiss_function_variable_id dependent_variable,
	Cmiss_function_variable_id independent_variable);
/*******************************************************************************
LAST MODIFIED : 26 August 2004

DESCRIPTION :
Creates a Cmiss_function gradient with the supplied <dependent_variable> and
<independent_variable>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_GRADIENT_H__ */
