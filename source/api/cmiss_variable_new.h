/*******************************************************************************
FILE : api/cmiss_variable_new.h

LAST MODIFIED : 10 September 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new object.

???DB.  new refers to
- combining Cmiss_variable and Cmiss_value
- adding a input specifier
- changing to C++
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_NEW_H__
#define __API_CMISS_VARIABLE_NEW_H__

#ifdef __cplusplus
#include <list>
using namespace std;
#endif /* __cplusplus */

/*
Global types
------------
*/
#ifdef __cplusplus
class Variable;
typedef Variable *
#else /* __cplusplus */
typedef struct Variable *
#endif /* __cplusplus */
	Cmiss_variable_new_id;
/*******************************************************************************
LAST MODIFIED : 7 September 2003

DESCRIPTION :
An object that can be evaluated from and differentiated with respect to other
variables.  It can be displayed, minimized or used in an equation.
==============================================================================*/

#ifdef __cplusplus
class Variable_input;
typedef Variable_input *
#else /* __cplusplus */
typedef struct Variable_input *
#endif /* __cplusplus */
	Cmiss_variable_new_input_id;
/*******************************************************************************
LAST MODIFIED : 7 September 2003

DESCRIPTION :
An object that specifies an input for a Cmiss_variable_new_id.
==============================================================================*/

#ifdef __cplusplus
typedef list<Variable_input> *
#else /* __cplusplus */
typedef struct Variable_input_list *
#endif /* __cplusplus */
	Cmiss_variable_new_input_list_id;
/*******************************************************************************
LAST MODIFIED : 7 September 2003

DESCRIPTION :
An object that specifies a list of inputs.
==============================================================================*/

#ifdef __cplusplus
class Variable_input_value;
typedef Variable_input_value *
#else /* __cplusplus */
typedef struct Variable_input_value *
#endif /* __cplusplus */
	Cmiss_variable_new_input_value_id;
/*******************************************************************************
LAST MODIFIED : 7 September 2003

DESCRIPTION :
An object that specifies an input value for a Cmiss_variable_new_id.
==============================================================================*/

#ifdef __cplusplus
typedef list<Variable_input_value> *
#else /* __cplusplus */
typedef struct Variable_input_value_list *
#endif /* __cplusplus */
	Cmiss_variable_new_input_value_list_id;
/*******************************************************************************
LAST MODIFIED : 7 September 2003

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

Cmiss_variable_new_id Cmiss_variable_new_evaluate(
	Cmiss_variable_new_id variable,
	Cmiss_variable_new_input_value_list_id values);
/*******************************************************************************
LAST MODIFIED : 7 September 2003

DESCRIPTION :
Calculates the <variable> with the specified <values> over-riding, but not
setting, the current values.
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

Cmiss_variable_new_input_id Cmiss_variable_new_input_create(
	char *specification_string);
/*******************************************************************************
LAST MODIFIED : 8 September 2003

DESCRIPTION :
Creates the variable input.
==============================================================================*/

int Cmiss_variable_new_input_destroy(
	Cmiss_variable_new_input_id *input_address);
/*******************************************************************************
LAST MODIFIED : 10 September 2003

DESCRIPTION :
Destroys the variable input.
==============================================================================*/

Cmiss_variable_new_input_list_id Cmiss_variable_new_input_list_create(void);
/*******************************************************************************
LAST MODIFIED : 7 September 2003

DESCRIPTION :
Creates an input list.
==============================================================================*/

int Cmiss_variable_new_input_list_destroy(
	Cmiss_variable_new_input_list_id *list_address);
/*******************************************************************************
LAST MODIFIED : 10 September 2003

DESCRIPTION :
Destroys an input list.
==============================================================================*/

int Cmiss_variable_new_input_list_add(Cmiss_variable_new_input_list_id list,
	Cmiss_variable_new_input_id input);
/*******************************************************************************
LAST MODIFIED : 7 September 2003

DESCRIPTION :
Adds an input to a list.
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
