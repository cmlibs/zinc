/*******************************************************************************
FILE : api/cmiss_function_composition.h

LAST MODIFIED : 11 June 2004

DESCRIPTION :
The public interface to the Cmiss_function composition object.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_COMPOSITION_H__
#define __API_CMISS_FUNCTION_COMPOSITION_H__

#include "api/cmiss_function.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_id Cmiss_function_composition_create(
	Cmiss_function_variable_id output_variable,
	Cmiss_function_variable_id input_variable,
	Cmiss_function_variable_id value_variable);
/*******************************************************************************
LAST MODIFIED : 11 June 2004

DESCRIPTION :
Creates a Cmiss_function composition
output_variable(input_variable=value_variable).
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_COMPOSITION_H__ */
