/*******************************************************************************
FILE : api/cmiss_function_variable_exclusion.h

LAST MODIFIED : 19 March 2004

DESCRIPTION :
The public interface to the Cmiss_function_variable exclusion object.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_VARIABLE_EXCLUSION_H__
#define __API_CMISS_FUNCTION_VARIABLE_EXCLUSION_H__

#include "api/cmiss_function_variable.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_variable_id Cmiss_function_variable_exclusion_create(
	Cmiss_function_variable_id universe,Cmiss_function_variable_id exclusion);
/*******************************************************************************
LAST MODIFIED : 19 March 2004

DESCRIPTION :
Creates a Cmiss_function_variable exclusion which is <universe>-<exclusion>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_VARIABLE_EXCLUSION_H__ */
