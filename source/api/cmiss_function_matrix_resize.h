/*******************************************************************************
FILE : api/cmiss_function_matrix_resize.h

LAST MODIFIED : 7 October 2004

DESCRIPTION :
The public interface to the Cmiss_function matrix resize object.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_MATRIX_RESIZE_H__
#define __API_CMISS_FUNCTION_MATRIX_RESIZE_H__

#include "api/cmiss_function.h"
#include "api/cmiss_function_variable.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_id Cmiss_function_matrix_resize_create(
	Cmiss_function_variable_id matrix,unsigned int number_of_columns);
/*******************************************************************************
LAST MODIFIED : 7 October 2004

DESCRIPTION :
Creates a Cmiss_function matrix which is the resize of <matrix>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_MATRIX_RESIZE_H__ */
