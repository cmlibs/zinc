/*******************************************************************************
FILE : api/cmiss_function_matrix_divide_by_scalar.h

LAST MODIFIED : 8 March 2005

DESCRIPTION :
The public interface to the Cmiss_function matrix divide by scalar object.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_MATRIX_DIVIDE_BY_SCALAR_H__
#define __API_CMISS_FUNCTION_MATRIX_DIVIDE_BY_SCALAR_H__

#include "api/cmiss_function.h"
#include "api/cmiss_function_variable.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_id Cmiss_function_matrix_divide_by_scalar_create(
	Cmiss_function_variable_id dividend,Cmiss_function_variable_id divisor);
/*******************************************************************************
LAST MODIFIED : 8 March 2005

DESCRIPTION :
Creates a Cmiss_function matrix which is the <dividend> (has matrix value)
divided by <divisor> (has a 1x1 matrix value).
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_MATRIX_DIVIDE_BY_SCALAR_H__ */
