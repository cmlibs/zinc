/*******************************************************************************
FILE : api/cmiss_function_linear_span.h

LAST MODIFIED : 10 November 2004

DESCRIPTION :
The public interface to the Cmiss_function linear span object.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_LINEAR_SPAN_H__
#define __API_CMISS_FUNCTION_LINEAR_SPAN_H__

#include "api/cmiss_function.h"
#include "api/cmiss_function_variable.h"
#include "api/cmiss_region.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_id Cmiss_function_linear_span_create(
	Cmiss_function_variable_id spanned_variable,
	Cmiss_function_variable_id spanning_variable);
/*******************************************************************************
LAST MODIFIED : 10 November 2004

DESCRIPTION :
Creates a Cmiss_function linear span.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_LINEAR_SPAN_H__ */
