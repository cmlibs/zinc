/*******************************************************************************
FILE : computed_value_private.h

LAST MODIFIED : 2 February 2003

DESCRIPTION :
==============================================================================*/
#if !defined (COMPUTED_VALUE_PRIVATE_H)
#define COMPUTED_VALUE_PRIVATE_H

#include "computed_variable/computed_value.h"

/*
Method types
------------
*/
typedef int (*Computed_value_clear_type_specific_function)(
	struct Computed_value *value);
typedef int (*Computed_value_same_type_specific_function)(
	struct Computed_value *value_1,struct Computed_value *value_2);

/*
Friend macros
-------------
*/
#define COMPUTED_VALUE_ESTABLISH_METHODS( value, value_type ) \
/***************************************************************************** \
LAST MODIFIED : 2 February 2003 \
\
DESCRIPTION : \
Each Computed_value_set_type function should call this macro to establish the \
virtual functions that give the value its particular behaviour.  Each function \
must therefore be defined for each value type, even if it is set to NULL or \
some default function. \
============================================================================*/ \
Computed_value_establish_methods(value, \
	Computed_value_ ## value_type ## _clear_type_specific, \
	Computed_value_ ## value_type ## _same_type_specific)

/*
Friend functions
----------------
*/
int Computed_value_establish_methods(struct Computed_value *value,
	Computed_value_clear_type_specific_function
	computed_value_clear_type_specific_function,
	Computed_value_same_type_specific_function
	computed_value_same_type_specific_function);
/*******************************************************************************
LAST MODIFIED : 2 February 2003

DESCRIPTION :
Sets the methods for the <value>.
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
