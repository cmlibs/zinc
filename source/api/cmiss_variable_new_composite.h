/*******************************************************************************
FILE : api/cmiss_variable_new_composite.h

LAST MODIFIED : 16 November 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new composite object.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_NEW_COMPOSITE_H__
#define __API_CMISS_VARIABLE_NEW_COMPOSITE_H__

#include "api/cmiss_variable_new.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_variable_new_id Cmiss_variable_new_composite_create(
	Cmiss_variable_new_list_id variables);
/*******************************************************************************
LAST MODIFIED : 16 November 2003

DESCRIPTION :
Creates a Cmiss_variable_new composite with the supplied <variables>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_VARIABLE_NEW_COMPOSITE_H__ */
