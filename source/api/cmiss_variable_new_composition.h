/*******************************************************************************
FILE : api/cmiss_variable_new_composition.h

LAST MODIFIED : 11 June 2004

DESCRIPTION :
The public interface to the Cmiss_variable_new composition object.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_NEW_COMPOSITION_H__
#define __API_CMISS_VARIABLE_NEW_COMPOSITION_H__

#include "api/cmiss_variable_new.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_variable_new_id Cmiss_variable_new_composition_create(
	Cmiss_variable_new_id dependent_variable,
	Cmiss_variable_new_input_value_list_id input_source_list);
/*******************************************************************************
LAST MODIFIED : 11 June 2004

DESCRIPTION :
Creates a Cmiss_variable_new composition with the supplied <dependent_variable>
and inputs <input_source_list>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_VARIABLE_NEW_COMPOSITION_H__ */
