/*******************************************************************************
FILE : api/cmiss_function_variable_intersection.h

LAST MODIFIED : 19 March 2004

DESCRIPTION :
The public interface to the Cmiss_function_variable intersection object.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_VARIABLE_INTERSECTION_H__
#define __API_CMISS_FUNCTION_VARIABLE_INTERSECTION_H__

#include "api/cmiss_function_variable.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_variable_id Cmiss_function_variable_intersection_create(
	Cmiss_function_variable_list_id variables);
/*******************************************************************************
LAST MODIFIED : 19 March 2004

DESCRIPTION :
Creates a Cmiss_function_variable intersection with the supplied <variables>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_VARIABLE_INTERSECTION_H__ */
