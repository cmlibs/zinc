/*******************************************************************************
FILE : api/cmiss_function_composite.h

LAST MODIFIED : 14 June 2004

DESCRIPTION :
The public interface to the Cmiss_function composite object.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_COMPOSITE_H__
#define __API_CMISS_FUNCTION_COMPOSITE_H__

#include "api/cmiss_function.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_id Cmiss_function_composite_create(
	Cmiss_function_list_id functions);
/*******************************************************************************
LAST MODIFIED : 14 June 2004

DESCRIPTION :
Creates a Cmiss_function composite with the supplied <functions>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_COMPOSITE_H__ */
