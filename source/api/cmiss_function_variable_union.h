/*******************************************************************************
FILE : api/cmiss_function_variable_union.h

LAST MODIFIED : 18 March 2004

DESCRIPTION :
The public interface to the Cmiss_function_variable union object.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_VARIABLE_UNION_H__
#define __API_CMISS_FUNCTION_VARIABLE_UNION_H__

#include "api/cmiss_function_variable.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_variable_id Cmiss_function_variable_union_create(
	Cmiss_function_variable_list_id variables);
/*******************************************************************************
LAST MODIFIED : 18 March 2004

DESCRIPTION :
Creates a Cmiss_function_variable union with the supplied <variables>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_VARIABLE_UNION_H__ */
