/*******************************************************************************
FILE : api/cmiss_function_integral.h

LAST MODIFIED : 4 November 2004

DESCRIPTION :
The public interface to the Cmiss_function integral object.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_INTEGRAL_H__
#define __API_CMISS_FUNCTION_INTEGRAL_H__

#include "api/cmiss_function.h"
#include "api/cmiss_function_variable.h"
#include "api/cmiss_region.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_id Cmiss_function_integral_create(
	Cmiss_function_variable_id integrand_output,
	Cmiss_function_variable_id integrand_input,
	Cmiss_function_variable_id independent_output,
	Cmiss_function_variable_id independent_input,
	Cmiss_region_id domain,char *quadrature_scheme);
/*******************************************************************************
LAST MODIFIED : 4 November 2004

DESCRIPTION :
Creates a Cmiss_function integral.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_INTEGRAL_H__ */
