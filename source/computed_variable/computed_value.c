/*******************************************************************************
FILE : computed_value.c

LAST MODIFIED : 2 February 2003

DESCRIPTION :
A module intended to replace general/value .  Testing and developing in
conjunction with Computed_variables.

???DB.  Should _get_s try and get a representation if not specified type?
==============================================================================*/
#include "computed_variable/computed_value.h"
#include "computed_variable/computed_value_private.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global types
------------
*/
struct Computed_value
/*******************************************************************************
LAST MODIFIED : 2 February 2003

DESCRIPTION :
A value that knows what type it is.

???DB.  An update of Value_type (general/value).  Want to be able to add new
	value types (in the same way that Computed_field types can be added).  Will
	need a package?  Will replace Value_type?
???DB.  At present just extend?
==============================================================================*/
{
	/* the type string identifies the value's type.  It points to a string that
		is shared by all variables of the same type and should not be copied */
	char *type_string;
	/* information that is specific to the type */
	void *type_specific_data;
	/* methods.   To add a new method, add
		- a field here in struct Computed_value
		- a function type declaration in computed_value_private.h
		- an argument and an assigment to Computed_value_establish_methods
		The compiler will force the other changes because
		Computed_value_establish_methods is used in
		- Computed_value_clear_type
		- CREATE(Computed_value) */
	Computed_value_clear_type_specific_function
		computed_value_clear_type_specific_function;
	Computed_value_same_type_specific_function
		computed_value_same_type_specific_function;
		/*???DB.  More than just same type_string.  How to indicate this? */

	/*???DB.  Sufficiently light weight that access_count not needed? */
	int access_count;
};  /* struct Computed_value */

/*
Friend functions
----------------
*/
int Computed_value_establish_methods(struct Computed_value *value,
	Computed_value_clear_type_specific_function
	computed_value_clear_type_specific_function,
	Computed_value_same_type_specific_function
	computed_value_same_type_specific_function)
/*******************************************************************************
LAST MODIFIED : 2 February 2003

DESCRIPTION :
Sets the methods for the <value>.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_value_establish_methods);
	return_code=0;
	/* check arguments */
	if (value)
	{
		value->computed_value_clear_type_specific_function=
			computed_value_clear_type_specific_function;
		value->computed_value_same_type_specific_function=
			computed_value_same_type_specific_function;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_value_establish_methods.  "
			"Missing value");
	}
	LEAVE;

	return (return_code);
} /* Computed_value_establish_methods */

int Computed_value_clear_type(struct Computed_value *value)
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
{
	int return_code;

	ENTER(Computed_value_clear_type);
	return_code=0;
	/* check arguments */
	if (value)
	{
		if (value->type_specific_data)
		{
			if (value->computed_value_clear_type_specific_function)
			{
				return_code=(value->computed_value_clear_type_specific_function)(value);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_value_clear_type.  "
					"Type specific data but no function to clear it");
				return_code=0;
			}
			value->type_specific_data=(void *)NULL;
		}
		/* clear all methods */
		Computed_value_establish_methods(value,
			(Computed_value_clear_type_specific_function)NULL,
			(Computed_value_same_type_specific_function)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_value_establish_methods.  "
			"Missing value");
	}
	LEAVE;

	return (return_code);
} /* Computed_value_clear_type */

/*
Global functions
----------------
*/
struct Computed_value *CREATE(Computed_value)(void)
/*******************************************************************************
LAST MODIFIED : 2 February 2003

DESCRIPTION :
Creates an empty value with no type.  Each type of value has its own "set_type"
function.
==============================================================================*/
{
	struct Computed_value *value;

	ENTER(CREATE(Computed_value));
	if (ALLOCATE(value,struct Computed_value,1))
	{
		/* initialise data */
		value->type_string=(char *)NULL;
		value->type_specific_data=NULL;
		/* initialise methods */
		Computed_value_establish_methods(value,
			(Computed_value_clear_type_specific_function)NULL,
			(Computed_value_same_type_specific_function)NULL);
		/* initialise access_count */
		value->access_count=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Computed_value).  "
			"Insufficient memory");
	}
	LEAVE;

	return (value);
} /* CREATE(Computed_value) */

int DESTROY(Computed_value)(struct Computed_value **value_address)
/*******************************************************************************
LAST MODIFIED : 2 February 2003

DESCRIPTION :
Frees memory/deaccess objects for Computed_value at <*value_address>.
==============================================================================*/
{
	int return_code;
	struct Computed_value *value;

	ENTER(DESTROY(Computed_value));
	return_code=0;
	if (value_address&&(value= *value_address))
	{
		Computed_value_clear_type(value);
		DEALLOCATE(*value_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Computed_value).  Missing value");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Computed_value) */

int Computed_value_same_type(struct Computed_value *value_1,
	struct Computed_value *value_2)
/*******************************************************************************
LAST MODIFIED : 2 February 2003

DESCRIPTION :
Returns nonzero if <value_1> and <value_2> have the same type and zero
otherwise.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_value_same_type);
	return_code=0;
	if (value_1&&value_2)
	{
		if (value_1->computed_value_same_type_specific_function)
		{
			return_code=(value_1->computed_value_same_type_specific_function)(value_1,
				value_2);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_value_clear_type.  "
				"Type specific data but no function to clear it");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Computed_value).  "
			"Invalid argument(s).  %p %p",value_1,value_2);
	}
	LEAVE;

	return (return_code);
} /* Computed_value_same_type */

char *Computed_value_get_type(struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Returns the string which identifies the type.  The calling function must not
DEALLOCATE the returned string.
==============================================================================*/

int Computed_value_set_type_FE_value(struct Computed_value *value,
	FE_value fe_value);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Makes <value> of type FE_value and sets its <fe_value>.
==============================================================================*/

int Computed_value_is_type_FE_value(struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Returns a non-zero if <value> is a FE_value and zero otherwise.
==============================================================================*/

int Computed_value_get_type_FE_value(struct Computed_value *value,
	FE_value *fe_value_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <value> is of type FE_value, gets its <*fe_value_address>.
==============================================================================*/

int Computed_value_set_type_FE_value_array(struct Computed_value *value,
	int number_of_fe_values,FE_value *fe_value_array);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Makes <value> of type FE_value_array and sets its <number_of_fe_values> and
<fe_value_array>.  After success, the <value> is responsible for DEALLOCATEing
<fe_value_array>.
==============================================================================*/

int Computed_value_is_type_FE_value_array(struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Returns a non-zero if <value> is a FE_value_array and zero otherwise.
==============================================================================*/

int Computed_value_get_type_FE_value_array(struct Computed_value *value,
	int *number_of_fe_values_address,FE_value **fe_value_array_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <value> is of type FE_value_array, gets its <*number_of_fe_values_address>
and <*fe_value_array_address>.

The calling program must not DEALLOCATE the returned <*fe_value_array_address>.
==============================================================================*/

int Computed_value_set_type_string(struct Computed_value *value,
	char *string);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Makes <value> of type string and sets its <string>.  After success, the <value>
is responsible for DEALLOCATEing <string>.
==============================================================================*/

int Computed_value_is_type_string(struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Returns a non-zero if <value> is a string and zero otherwise.
==============================================================================*/

int Computed_value_get_type_string(struct Computed_value *value,
	char **string_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <value> is of type string, gets its <*string_address>.

The calling program must not DEALLOCATE the returned <*string_address>.
==============================================================================*/
