/*******************************************************************************
FILE : api/cmiss_function_matrix_sum.h

LAST MODIFIED : 2 September 2004

DESCRIPTION :
The public interface to the Cmiss_function matrix sum object.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_MATRIX_SUM_H__
#define __API_CMISS_FUNCTION_MATRIX_SUM_H__

#include "api/cmiss_function.h"
#include "api/cmiss_function_variable.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_id Cmiss_function_matrix_sum_create(
	Cmiss_function_variable_id summand_1,Cmiss_function_variable_id summand_2);
/*******************************************************************************
LAST MODIFIED : 2 September 2004

DESCRIPTION :
Creates a Cmiss_function matrix which is the sum of <summand_1> and <summand_2>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_MATRIX_SUM_H__ */
