/*******************************************************************************
FILE : api/cmiss_function_matrix_determinant.h

LAST MODIFIED : 15 September 2004

DESCRIPTION :
The public interface to the Cmiss_function matrix determinant object.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_MATRIX_DETERMINANT_H__
#define __API_CMISS_FUNCTION_MATRIX_DETERMINANT_H__

#include "api/cmiss_function.h"
#include "api/cmiss_function_variable.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_id Cmiss_function_matrix_determinant_create(
	Cmiss_function_variable_id matrix);
/*******************************************************************************
LAST MODIFIED : 15 September 2004

DESCRIPTION :
Creates a Cmiss_function matrix which is the determinant of <matrix>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_MATRIX_DETERMINANT_H__ */
