/*******************************************************************************
FILE : computed_value_private.h

LAST MODIFIED : 12 February 2003

DESCRIPTION :
==============================================================================*/
#if !defined (COMPUTED_VALUE_PRIVATE_H)
#define COMPUTED_VALUE_PRIVATE_H

#include "computed_variable/computed_value.h"

/*
Method types
------------
*/
typedef void Computed_value_type_specific_data;

typedef int (*Computed_value_clear_type_specific_function)(
	struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Clear the type specific data for <value> (passed into the _set_type_
function), but don't DEALLOCATE the data.
==============================================================================*/

#define START_COMPUTED_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION( value_type ) \
int Computed_value_ ## value_type ## _clear_type_specific( \
	struct Computed_value *value) \
{ \
	int return_code; \
	struct Computed_value_ ## value_type ## _type_specific_data *data; \
\
	ENTER(Computed_value_ ## value_type ## _clear_type_specific); \
	return_code=0; \
	data=(struct Computed_value_ ## value_type ## _type_specific_data *) \
		Computed_value_get_type_specific_data(value); \
	ASSERT_IF(data,return_code,0) \
	ASSERT_IF(computed_value_ ## value_type ## _type_string== \
		Computed_value_get_type_id_string(value),return_code,0)

#define END_COMPUTED_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION( value_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Computed_value_ ## value_type ## _clear_type_specific */

typedef int (*Computed_value_multiply_and_accumulate_type_specific_function)(
	struct Computed_value *value_1,struct Computed_value *value_2,
	struct Computed_value *total);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Check that <value_1> and <value_2> are of the same type.

???DB.  Allows more than same type_strin
==============================================================================*/

#define START_COMPUTED_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION( \
	value_type ) \
int Computed_value_ ## value_type ## _multiply_and_accumulate_type_specific( \
	struct Computed_value *value_1,struct Computed_value *value_2, \
	struct Computed_value *total) \
{ \
	int return_code; \
\
	ENTER(Computed_value_ ## value_type ## \
		_multiply_and_accumulater_type_specific); \
	return_code=0; \
	ASSERT_IF(value_1&&(computed_value_ ## value_type ## _type_string== \
		Computed_value_get_type_id_string(value_1))&&value_2&&(computed_value_ ## \
		value_type ## _type_string==Computed_value_get_type_id_string(value_2))&& \
		total&&(computed_value_ ## value_type ## _type_string== \
		Computed_value_get_type_id_string(total)),return_code,0)

#define END_COMPUTED_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION( \
	value_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Computed_value_ ## value_type ## \
	_multiply_and_accumulate_type_specific */

typedef int (*Computed_value_same_sub_type_type_specific_function)(
	struct Computed_value *value_1,struct Computed_value *value_2);
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Check that <value_1> and <value_2> are of the same sub-type (same type plus
extra restrictions such as length for a vector).
==============================================================================*/

#define START_COMPUTED_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION( \
	value_type ) \
int Computed_value_ ## value_type ## _same_sub_type_type_specific( \
	struct Computed_value *value_1,struct Computed_value *value_2) \
{ \
	int return_code; \
\
	ENTER(Computed_value_ ## value_type ## _same_sub_type_type_specific); \
	return_code=0; \
	ASSERT_IF(value_1&&(computed_value_ ## value_type ## _type_string== \
		Computed_value_get_type_id_string(value_1))&&value_2&&computed_value_ ## \
		value_type ## _type_string==Computed_value_get_type_id_string(value_2), \
		return_code,0)

#define END_COMPUTED_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION( value_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Computed_value_ ## value_type ## _same_sub_type_type_specific */


/*
Friend macros
-------------
*/
#define DECLARE_COMPUTED_VALUE_IS_TYPE_FUNCTION( value_type ) \
PROTOTYPE_COMPUTED_VALUE_IS_TYPE_FUNCTION(value_type) \
{ \
	int return_code; \
\
	ENTER(Computed_value_is_type_ ## value_type); \
	return_code=0; \
	/* check argument */ \
	if (value) \
	{ \
		if (computed_value_ ## value_type ## _type_string==value->type_string) \
		{ \
			return_code=1; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"Computed_value_is_type_" #value_type ".  " \
			"Missing value"); \
	} \
	LEAVE; \
\
	return (return_code); \
} /* Computed_value_is_type_ ## value_type */

#define COMPUTED_VALUE_ESTABLISH_METHODS( value, value_type ) \
/***************************************************************************** \
LAST MODIFIED : 12 February 2003 \
\
DESCRIPTION : \
Each Computed_value_set_type function should call this macro to establish the \
virtual functions that give the value its particular behaviour.  Each function \
must therefore be defined for each value type, even if it is set to NULL or \
some default function. \
============================================================================*/ \
Computed_value_establish_methods(value, \
	Computed_value_ ## value_type ## _clear_type_specific, \
	Computed_value_ ## value_type ## _multiply_and_accumulate_type_specific, \
	Computed_value_ ## value_type ## _same_sub_type_type_specific)

/*
Friend functions
----------------
*/
int Computed_value_establish_methods(struct Computed_value *value,
	Computed_value_clear_type_specific_function
	computed_value_clear_type_specific_function,
	Computed_value_multiply_and_accumulate_type_specific_function
	computed_value_multiply_and_accumulate_type_specific_function,
	Computed_value_same_sub_type_type_specific_function
	computed_value_same_sub_type_type_specific_function);
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Sets the methods for the <value>.
==============================================================================*/

Computed_value_type_specific_data *Computed_value_get_type_specific_data(
	struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Returns the type specific data for the <value>.
==============================================================================*/

int Computed_value_clear_type(struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 2 February 2003

DESCRIPTION :
Used internally by DESTROY and Computed_value_set_type_*() functions to
deallocate or deaccess data specific to any Computed_value_type.  Functions
changing the type of the Computed_value should
- allocate any dynamic data needed for the type
- call this function to clear what is currently in the value
- then set values
to ensure that the value is not left in an invalid state.
==============================================================================*/
#endif /* !defined (COMPUTED_VALUE_PRIVATE_H) */
