/*******************************************************************************
FILE : api/cmiss_variable_new_input_composite.h

LAST MODIFIED : 20 October 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new_input composite objects.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_NEW_INPUT_COMPOSITE_H__
#define __API_CMISS_VARIABLE_NEW_INPUT_COMPOSITE_H__

#include "api/cmiss_variable_new.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_variable_new_input_id Cmiss_variable_new_input_composite_create(
	Cmiss_variable_new_input_value_list_id inputs);
/*******************************************************************************
LAST MODIFIED : 20 October 2003

DESCRIPTION :
Creates a Cmiss_variable_new_input composite with the supplied <inputs>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_VARIABLE_NEW_INPUT_COMPOSITE_H__ */
