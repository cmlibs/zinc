/*******************************************************************************
FILE : api/cmiss_function_variable.h

LAST MODIFIED : 5 March 2004

DESCRIPTION :
The public interface to the Cmiss_function_variable object.  Variables specify
inputs and/or outputs of functions.  Variables can be evaluated, differentiated
or set to another value.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_VARIABLE_H__
#define __API_CMISS_FUNCTION_VARIABLE_H__

#include "api/cmiss_function_base.h"

/*
Global types
------------
*/
typedef struct Cmiss_function_variable_list * Cmiss_function_variable_list_id;
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
An identifier for a list of variables.
==============================================================================*/

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int Cmiss_function_variable_destroy(
	Cmiss_function_variable_id *variable_address);
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
Destroys the variable.  Returns a non-zero if successful and zero otherwise.
==============================================================================*/

int Cmiss_function_variable_get_string_representation(
	Cmiss_function_variable_id variable,char **result);
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
Creates a string representation of the <variable> useful for output.  Returns a
non-zero if successful and zero otherwise.  If successful <*result> contains an
ALLOCATED string, it is up to the calling function to DEALLOCATE the string when
it is no longer required.
==============================================================================*/

Cmiss_function_id Cmiss_function_variable_evaluate(
	Cmiss_function_variable_id variable,Cmiss_function_variable_id input,
	Cmiss_function_id value);
/*******************************************************************************
LAST MODIFIED : 5 March 2004

DESCRIPTION :
Calculates and returns the value of the <variable> with the <input> set to the
<value> during the calculation and then reset to its original value.

For an input <variable>, this function will get its current value.
==============================================================================*/

Cmiss_function_id Cmiss_function_variable_evaluate_derivative(
	Cmiss_function_variable_id variable,
	Cmiss_function_variable_list_id independent_variables,
	Cmiss_function_variable_id input,Cmiss_function_id value);
/*******************************************************************************
LAST MODIFIED : 5 March 2004

DESCRIPTION :
Calculates and returns the derivative of the <variable> with the respect to the
<independent_variables>, with the <input> set to the <value> during the
calculation and then reset to its original value.
==============================================================================*/

int Cmiss_function_variable_set_value(Cmiss_function_variable_id variable,
	Cmiss_function_id value);
/*******************************************************************************
LAST MODIFIED : 5 March 2004

DESCRIPTION :
Changes the <variable> to have the given <value>.  Returns a non-zero if the
<variable> was changed and zero otherwise.
==============================================================================*/

Cmiss_function_variable_list_id Cmiss_function_variable_list_create(void);
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
Creates a variable list.
==============================================================================*/

int Cmiss_function_variable_list_destroy(
	Cmiss_function_variable_list_id *list_address);
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
Destroys a variable list.
==============================================================================*/

int Cmiss_function_variable_list_add(Cmiss_function_variable_list_id list,
	Cmiss_function_variable_id variable);
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
Adds a <variable> to a <list>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_VARIABLE_H__ */
