/*******************************************************************************
FILE : api/cmiss_variable_new_input.h

LAST MODIFIED : 21 October 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new_input object.

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
#ifndef __API_CMISS_VARIABLE_NEW_INPUT_H__
#define __API_CMISS_VARIABLE_NEW_INPUT_H__

/*
Global types
------------
*/
typedef struct Cmiss_variable_new_input * Cmiss_variable_new_input_id;
/*******************************************************************************
LAST MODIFIED : 25 September 2003

DESCRIPTION :
An object that specifies an input for a Cmiss_variable_new_id.
==============================================================================*/

typedef struct Cmiss_variable_new_input_list * Cmiss_variable_new_input_list_id;
/*******************************************************************************
LAST MODIFIED : 25 September 2003

DESCRIPTION :
An object that specifies a list of inputs.
==============================================================================*/

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int Cmiss_variable_new_input_destroy(
	Cmiss_variable_new_input_id *input_address);
/*******************************************************************************
LAST MODIFIED : 20 October 2003

DESCRIPTION :
Destroys an input.
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

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_VARIABLE_NEW_INPUT_H__ */
