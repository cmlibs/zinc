/*******************************************************************************
FILE : api/cmiss_variable_new_scalar.h

LAST MODIFIED : 24 October 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new scalar object.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_NEW_SCALAR_H__
#define __API_CMISS_VARIABLE_NEW_SCALAR_H__

#include "api/cmiss_variable_new.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_variable_new_id Cmiss_variable_new_scalar_create(Scalar value);
/*******************************************************************************
LAST MODIFIED : 11 October 2003

DESCRIPTION :
Creates a Cmiss_variable_new scalar with the supplied <value>.
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_scalar_value(
	Cmiss_variable_new_id variable_scalar);
/*******************************************************************************
LAST MODIFIED : 24 October 2003

DESCRIPTION :
Returns the value input for the <variable_scalar>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_VARIABLE_NEW_SCALAR_H__ */
