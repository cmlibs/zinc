/*******************************************************************************
FILE : api/cmiss_variable_new.h

LAST MODIFIED : 16 November 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new object.

???DB.  new refers to
- combining Cmiss_variable and Cmiss_value
- adding a input specifier
- changing to C++

???DB.  Will have to change *_id when make *_handle classes

???DB.  Lose all the smart pointer abilities and end up memory leaks?
	- this is why have destroy functions

???DB.  id is not right because many ids (different values) can be for the same
	Variable
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_NEW_H__
#define __API_CMISS_VARIABLE_NEW_H__

// needs to be the same as typedef in computed_variable/variable.hpp, but will
//   not compile if differ
typedef double Scalar;

#include "api/cmiss_variable_new_input.h"

/*
Global types
------------
*/
typedef struct Cmiss_variable_new * Cmiss_variable_new_id;
/*******************************************************************************
LAST MODIFIED : 25 September 2003

DESCRIPTION :
An object that can be evaluated from and differentiated with respect to other
variables.  It can be displayed, minimized or used in an equation.
==============================================================================*/

typedef struct Cmiss_variable_new_list *
	Cmiss_variable_new_list_id;
/*******************************************************************************
LAST MODIFIED : 16 November 2003

DESCRIPTION :
An object that specifies a list of variables.
==============================================================================*/

typedef struct Cmiss_variable_new_input_value *
	Cmiss_variable_new_input_value_id;
/*******************************************************************************
LAST MODIFIED : 25 September 2003

DESCRIPTION :
An object that specifies an input value for a Cmiss_variable_new_id.
==============================================================================*/

typedef struct Cmiss_variable_new_input_value_list *
	Cmiss_variable_new_input_value_list_id;
/*******************************************************************************
LAST MODIFIED : 25 September 2003

DESCRIPTION :
An object that specifies a list of input values.
==============================================================================*/

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int Cmiss_variable_new_destroy(Cmiss_variable_new_id *variable_address);
/*******************************************************************************
LAST MODIFIED : 9 September 2003

DESCRIPTION :
Destroys the variable.
==============================================================================*/

const char *Cmiss_variable_new_get_type_id_string(
	Cmiss_variable_new_id variable);
/*******************************************************************************
LAST MODIFIED : 16 October 2003

DESCRIPTION :
Returns the string which identifies the type.  The calling function must not
DEALLOCATE the returned string.
==============================================================================*/

Cmiss_variable_new_id Cmiss_variable_new_evaluate(
	Cmiss_variable_new_id variable,
	Cmiss_variable_new_input_value_list_id values);
/*******************************************************************************
LAST MODIFIED : 7 September 2003

DESCRIPTION :
Calculates the <variable> with the specified <values> over-riding, but not
setting, the current values.
==============================================================================*/

Cmiss_variable_new_id Cmiss_variable_new_evaluate_derivative(
	Cmiss_variable_new_id variable,
	Cmiss_variable_new_input_list_id independent_variables,
	Cmiss_variable_new_input_value_list_id values);
/*******************************************************************************
LAST MODIFIED : 16 October 2003

DESCRIPTION :
Calculates the derivative of the <variable> with the respect to the
<independent_variables>, with the specified <values> over-riding, but not
setting, the current values.
==============================================================================*/

Cmiss_variable_new_id Cmiss_variable_new_get_input_value(
	Cmiss_variable_new_id variable,Cmiss_variable_new_input_id input);
/*******************************************************************************
LAST MODIFIED : 23 October 2003

DESCRIPTION :
Gets the specified <input> for the <variable>.
==============================================================================*/

int Cmiss_variable_new_set_input_value(Cmiss_variable_new_id variable,
	Cmiss_variable_new_input_id input,Cmiss_variable_new_id value);
/*******************************************************************************
LAST MODIFIED : 23 October 2003

DESCRIPTION :
Sets the specified <input> for the <variable> with the given <value>.
==============================================================================*/

int Cmiss_variable_new_get_string_representation(Cmiss_variable_new_id variable,
	char **result);
/*******************************************************************************
LAST MODIFIED : 8 September 2003

DESCRIPTION :
Creates a string representation of the Cmiss_value useful for output.  If
successful <*result> contains an ALLOCATED string, it is up to the calling
function to DEALLOCATE the string when it is no longer required.
==============================================================================*/

Cmiss_variable_new_list_id Cmiss_variable_new_list_create(void);
/*******************************************************************************
LAST MODIFIED : 16 November 2003

DESCRIPTION :
Creates an variable list.
==============================================================================*/

int Cmiss_variable_new_list_destroy(Cmiss_variable_new_list_id *list_address);
/*******************************************************************************
LAST MODIFIED : 16 November 2003

DESCRIPTION :
Destroys an variable list.
==============================================================================*/

int Cmiss_variable_new_list_add(Cmiss_variable_new_list_id list,
	Cmiss_variable_new_id variable);
/*******************************************************************************
LAST MODIFIED : 16 November 2003

DESCRIPTION :
Adds an variable to a list.
==============================================================================*/

Cmiss_variable_new_input_value_list_id
	Cmiss_variable_new_input_value_list_create(void);
/*******************************************************************************
LAST MODIFIED : 7 September 2003

DESCRIPTION :
Creates an input value list.
==============================================================================*/

int Cmiss_variable_new_input_value_list_destroy(
	Cmiss_variable_new_input_value_list_id *list_address);
/*******************************************************************************
LAST MODIFIED : 10 September 2003

DESCRIPTION :
Destroys an input value list.
==============================================================================*/

int Cmiss_variable_new_input_value_list_add(
	Cmiss_variable_new_input_value_list_id list,
	Cmiss_variable_new_input_id input,Cmiss_variable_new_id value);
/*******************************************************************************
LAST MODIFIED : 7 September 2003

DESCRIPTION :
Adds an input value to a list.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_VARIABLE_NEW_H__ */
