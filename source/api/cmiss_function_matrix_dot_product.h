/*******************************************************************************
FILE : api/cmiss_function_matrix_dot_product.h

LAST MODIFIED : 20 October 2004

DESCRIPTION :
The public interface to the Cmiss_function matrix dot product object.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_MATRIX_DOT_PRODUCT_H__
#define __API_CMISS_FUNCTION_MATRIX_DOT_PRODUCT_H__

#include "api/cmiss_function.h"
#include "api/cmiss_function_variable.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_id Cmiss_function_matrix_dot_product_create(
	Cmiss_function_variable_id variable_1,Cmiss_function_variable_id variable_2);
/*******************************************************************************
LAST MODIFIED : 20 October 2004

DESCRIPTION :
Creates a Cmiss_function matrix which is the dot product of <multiplier> and
<multiplicand>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_MATRIX_DOT_PRODUCT_H__ */
