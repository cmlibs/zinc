/*******************************************************************************
FILE : api/cmiss_function_matrix_product.h

LAST MODIFIED : 7 September 2004

DESCRIPTION :
The public interface to the Cmiss_function matrix product object.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_MATRIX_PRODUCT_H__
#define __API_CMISS_FUNCTION_MATRIX_PRODUCT_H__

#include "api/cmiss_function.h"
#include "api/cmiss_function_variable.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_id Cmiss_function_matrix_product_create(
	Cmiss_function_variable_id multiplier,
	Cmiss_function_variable_id multiplicand);
/*******************************************************************************
LAST MODIFIED : 7 September 2004

DESCRIPTION :
Creates a Cmiss_function matrix which is the product of <multiplier> and
<multiplicand>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_MATRIX_PRODUCT_H__ */
