/*******************************************************************************
FILE : computed_value.c

LAST MODIFIED : 29 July 2003

DESCRIPTION :
A module intended to replace general/value .  Testing and developing in
conjunction with Cmiss_variables.

???DB.  Should _get_s try and get a representation if not specified type?

???DB.  Split GET_REALS into GET_REALS and GET_NUMBER_OF_REALS with GET_REALS
	filling in an array rather than allocating and filling in?
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#include "computed_variable/computed_value.h"
#include "computed_variable/computed_value_private.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"

/*
Module types
------------
*/
struct Cmiss_value
/*******************************************************************************
LAST MODIFIED : 28 July 2003

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
		- a field here in struct Cmiss_value
		- a function type declaration in computed_value_private.h
		- an argument and an assigment to Cmiss_value_establish_methods
		The compiler will force the other changes because
		Cmiss_value_establish_methods is used in
		- Cmiss_value_clear_type
		- CREATE(Cmiss_value) */
	Cmiss_value_clear_type_specific_function clear_type_specific_function;
	Cmiss_value_duplicate_data_type_specific_function
		duplicate_data_type_specific_function;
	Cmiss_value_get_reals_type_specific_function get_reals_type_specific_function;
	Cmiss_value_get_string_type_specific_function
		get_string_type_specific_function;
	Cmiss_value_increment_type_specific_function increment_type_specific_function;
	Cmiss_value_multiply_and_accumulate_type_specific_function
		multiply_and_accumulate_type_specific_function;
	Cmiss_value_same_sub_type_type_specific_function
		same_sub_type_type_specific_function;
		/*???DB.  More than just same type_string.  How to indicate this? */
	Cmiss_value_scalar_multiply_type_specific_function
		scalar_multiply_type_specific_function;
	int access_count;
};  /* struct Cmiss_value */

/*
Friend functions
----------------
*/
int Cmiss_value_establish_methods(Cmiss_value_id value,
	Cmiss_value_clear_type_specific_function clear_type_specific_function,
	Cmiss_value_duplicate_data_type_specific_function
	duplicate_data_type_specific_function,
	Cmiss_value_get_reals_type_specific_function get_reals_type_specific_function,
	Cmiss_value_get_string_type_specific_function
	get_string_type_specific_function,
	Cmiss_value_increment_type_specific_function increment_type_specific_function,
	Cmiss_value_multiply_and_accumulate_type_specific_function
	multiply_and_accumulate_type_specific_function,
	Cmiss_value_same_sub_type_type_specific_function
	same_sub_type_type_specific_function,
	Cmiss_value_scalar_multiply_type_specific_function
	scalar_multiply_type_specific_function)
/*******************************************************************************
LAST MODIFIED : 28 July 2003

DESCRIPTION :
Sets the methods for the <value>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_value_establish_methods);
	return_code=0;
	/* check arguments */
	if (value)
	{
		value->clear_type_specific_function=clear_type_specific_function;
		value->duplicate_data_type_specific_function=
			duplicate_data_type_specific_function;
		value->multiply_and_accumulate_type_specific_function=
			multiply_and_accumulate_type_specific_function;
		value->get_reals_type_specific_function=get_reals_type_specific_function;
		value->get_string_type_specific_function=get_string_type_specific_function;
		value->increment_type_specific_function=increment_type_specific_function;
		value->same_sub_type_type_specific_function=
			same_sub_type_type_specific_function;
		value->scalar_multiply_type_specific_function=
			scalar_multiply_type_specific_function;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_establish_methods.  "
			"Missing value");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_establish_methods */

Cmiss_value_type_specific_data *Cmiss_value_get_type_specific_data(
	Cmiss_value_id value)
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Returns the type specific data for the <value>.
==============================================================================*/
{
	Cmiss_value_type_specific_data *data;

	ENTER(Cmiss_value_get_type_specific_data);
	data=(Cmiss_value_type_specific_data *)NULL;
	ASSERT_IF(value,data,(Cmiss_value_type_specific_data *)NULL)
	{
		data=value->type_specific_data;
	}
	LEAVE;

	return (data);
} /* Cmiss_value_get_type_specific_data */

int Cmiss_value_set_type_specific_information(Cmiss_value_id value,
	char *type_string,void *type_specific_data)
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
Sets the type specific information for the <value>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_value_set_type_specific_information);
	return_code=0;
	/* check arguments */
	if (value&&type_string&&type_specific_data)
	{
		value->type_specific_data=type_specific_data;
		value->type_string=type_string;
		return_code=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_value_set_type_specific_information.  "
			"Invalid argument(s).  %p %p %p",value,type_string,type_specific_data);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_set_type_specific_information */

int Cmiss_value_clear_type(Cmiss_value_id value)
/*******************************************************************************
LAST MODIFIED : 28 July 2003

DESCRIPTION :
Used internally by DESTROY and Cmiss_value_set_type_*() functions to
deallocate or deaccess data specific to any Cmiss_value_type.  Functions
changing the type of the Cmiss_value should
- allocate any dynamic data needed for the type
- call this function to clear what is currently in the value
- then set values
to ensure that the value is not left in an invalid state.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_value_clear_type);
	return_code=0;
	/* check arguments */
	if (value)
	{
		if (value->type_specific_data)
		{
			if (value->clear_type_specific_function)
			{
				return_code=(value->clear_type_specific_function)(value);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_value_clear_type.  "
					"Type specific data but no function to clear it");
				return_code=0;
			}
			DEALLOCATE(value->type_specific_data);
		}
		else
		{
			return_code=1;
		}
		/* clear all methods */
		Cmiss_value_establish_methods(value,
			(Cmiss_value_clear_type_specific_function)NULL,
			(Cmiss_value_duplicate_data_type_specific_function)NULL,
			(Cmiss_value_get_reals_type_specific_function)NULL,
			(Cmiss_value_get_string_type_specific_function)NULL,
			(Cmiss_value_increment_type_specific_function)NULL,
			(Cmiss_value_multiply_and_accumulate_type_specific_function)NULL,
			(Cmiss_value_same_sub_type_type_specific_function)NULL,
			(Cmiss_value_scalar_multiply_type_specific_function)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_establish_methods.  "
			"Missing value");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_clear_type */

/*
Global functions
----------------
*/
Cmiss_value_id CREATE(Cmiss_value)(void)
/*******************************************************************************
LAST MODIFIED : 28 July 2003

DESCRIPTION :
Creates an empty value with no type.  Each type of value has its own "set_type"
function.
==============================================================================*/
{
	Cmiss_value_id value;

	ENTER(CREATE(Cmiss_value));
	if (ALLOCATE(value,struct Cmiss_value,1))
	{
		/* initialise data */
		value->type_string=(char *)NULL;
		value->type_specific_data=NULL;
		/* initialise methods */
		Cmiss_value_establish_methods(value,
			(Cmiss_value_clear_type_specific_function)NULL,
			(Cmiss_value_duplicate_data_type_specific_function)NULL,
			(Cmiss_value_get_reals_type_specific_function)NULL,
			(Cmiss_value_get_string_type_specific_function)NULL,
			(Cmiss_value_increment_type_specific_function)NULL,
			(Cmiss_value_multiply_and_accumulate_type_specific_function)NULL,
			(Cmiss_value_same_sub_type_type_specific_function)NULL,
			(Cmiss_value_scalar_multiply_type_specific_function)NULL);
		/* initialise access_count */
		value->access_count=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Cmiss_value).  "
			"Insufficient memory");
	}
	LEAVE;

	return (value);
} /* CREATE(Cmiss_value) */

int DESTROY(Cmiss_value)(Cmiss_value_id *value_address)
/*******************************************************************************
LAST MODIFIED : 2 February 2003

DESCRIPTION :
Frees memory/deaccess objects for Cmiss_value at <*value_address>.
==============================================================================*/
{
	int return_code;
	Cmiss_value_id value;

	ENTER(DESTROY(Cmiss_value));
	return_code=0;
	if (value_address&&(value= *value_address))
	{
		Cmiss_value_clear_type(value);
		DEALLOCATE(*value_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Cmiss_value).  Missing value");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Cmiss_value) */

int Cmiss_value_copy(Cmiss_value_id destination,
	Cmiss_value_id source)
/*******************************************************************************
LAST MODIFIED : 28 July 2003

DESCRIPTION :
Copies the type and contents from <source> to <destination>.

???DB.  What if the access_count>0?  Put in Cmiss_value_clear_type?
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_value_copy);
	return_code=0;
	/* check arguments */
	if (destination&&source)
	{
		/* clear destination */
		if (Cmiss_value_clear_type(destination))
		{
			/* initialise data */
			destination->type_string=source->type_string;
			if (source->duplicate_data_type_specific_function)
			{
				destination->type_specific_data=
					(source->duplicate_data_type_specific_function)(source);
			}
			/* initialise methods */
			Cmiss_value_establish_methods(destination,
				source->clear_type_specific_function,
				source->duplicate_data_type_specific_function,
				source->get_reals_type_specific_function,
				source->get_string_type_specific_function,
				source->increment_type_specific_function,
				source->multiply_and_accumulate_type_specific_function,
				source->same_sub_type_type_specific_function,
				source->scalar_multiply_type_specific_function);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_copy.  "
			"Invalid argument(s).  %p %p",destination,source);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_copy */

DECLARE_OBJECT_FUNCTIONS(Cmiss_value)

int Cmiss_value_same_sub_type(Cmiss_value_id value_1,
	Cmiss_value_id value_2)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Returns nonzero if <value_1> and <value_2> have the same sub-type and zero
otherwise.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_value_same_sub_type);
	return_code=0;
	if (value_1&&value_2)
	{
		if (value_1->same_sub_type_type_specific_function)
		{
			return_code=(value_1->same_sub_type_type_specific_function)(value_1,
				value_2);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_value_same_sub_type.  "
				"Missing method");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_same_sub_type.  "
			"Invalid argument(s).  %p %p",value_1,value_2);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_same_sub_type */

char *Cmiss_value_get_type_id_string(Cmiss_value_id value)
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Returns the string which identifies the type.  The calling function must not
DEALLOCATE the returned string.
==============================================================================*/
{
	char *return_string;

	ENTER(Cmiss_value_get_type_id_string);
	return_string=(char *)NULL;
	if (value)
	{
		return_string=value->type_string;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_get_type_id_string.  "
			"Missing value");
	}
	LEAVE;

	return (return_string);
} /* Cmiss_value_get_type_id_string */

int Cmiss_value_get_reals(Cmiss_value_id value,int *number_of_reals_address,
	FE_value **reals_address)
/*******************************************************************************
LAST MODIFIED : 18 July 2003

DESCRIPTION :
Gets the <*number_of_reals_address> for the <value>.  This is needed when
calculating derivatives.  If <real_address> is not NULL, then an array is
allocated and the reals put in it.  A zero return code means that <value> is not
represented by reals or the array could not be allocated or the arguments are
invalid.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_value_get_reals);
	return_code=0;
	if (value&&number_of_reals_address)
	{
		if (value->get_reals_type_specific_function)
		{
			return_code=(value->get_reals_type_specific_function)(value,
				number_of_reals_address,reals_address);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_value_get_reals.  "
				"Missing method");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_get_reals.  "
			"Invalid argument(s).  %p %p",value,number_of_reals_address);
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_get_reals */

int Cmiss_value_multiply_and_accumulate(Cmiss_value_id total,
	Cmiss_value_id value_1,Cmiss_value_id value_2)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Calculates <total>+<value_1>*<value_2> and puts in <total>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_value_multiply_and_accumulate);
	return_code=0;
	/* check arguments */
	if (value_1&&value_2&&total)
	{
		if (total->multiply_and_accumulate_type_specific_function)
		{
			return_code=(total->multiply_and_accumulate_type_specific_function)(
				total,value_1,value_2);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_value_multiply_and_accumulate.  "
				"Missing method");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_multiply_and_accumulate.  "
			"Invalid argument(s).  %p %p %p",total,value_1,value_2);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_multiply_and_accumulate */

int Cmiss_value_get_string(Cmiss_value_id value,
	char **result)
/*******************************************************************************
LAST MODIFIED : 25 July 2003

DESCRIPTION :
Creates a string representation of the Cmiss_value useful for output.
If successful <result> contains an ALLOCATED string, it is up to the calling
function to DEALLOCATE the string when it is no longer required.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_value_get_string);
	return_code=0;
	if (value&&result)
	{
		*result = (char *)NULL;
		if (value->get_string_type_specific_function)
		{
			return_code=(value->get_string_type_specific_function)(
				value, result);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_value_get_string.  "
				"Missing method");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_get_string.  "
			"Invalid argument(s).");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_get_string */

int Cmiss_value_default_get_string(Cmiss_value_id value,
	char **result)
/*******************************************************************************
LAST MODIFIED : 25 July 2003

DESCRIPTION :
A default implementation of Cmiss_value_get string which calls the
Cmiss_value_get_reals method and concatenates these into a list of numbers.
==============================================================================*/
{
	char tmp_string[50]; /* Large enough for one %g plus a few characters */
	FE_value *values;
	int error, i, number_of_values, return_code;

	ENTER(Cmiss_value_default_get_string);
	return_code=0;
	if (value&&result)
	{
		*result = (char *)NULL;
		error = 0;
		if (Cmiss_value_get_reals(value, &number_of_values,
			&values))
		{
			append_string(result,"[",&error);
			if (number_of_values > 0)
			{
				sprintf(tmp_string,"%g", values[0]);
				append_string(result,tmp_string,&error);
			}
			for (i = 1 ; i < number_of_values ; i++)
			{
				sprintf(tmp_string,",%g", values[i]);
				append_string(result,tmp_string,&error);
			}
			append_string(result,"]",&error);
			DEALLOCATE(values);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_default_get_string.  "
			"Invalid argument(s).");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_default_get_string */

int Cmiss_value_increment(Cmiss_value_id value,Cmiss_value_id increment)
/*******************************************************************************
LAST MODIFIED : 29 July 2003

DESCRIPTION :
Calculates <value>+<increment> and puts in <value>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_value_increment);
	return_code=0;
	/* check arguments */
	if (value&&increment)
	{
		if (value->increment_type_specific_function)
		{
			return_code=(value->increment_type_specific_function)(value,increment);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_value_increment.  "
				"Missing method");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_increment.  "
			"Invalid argument(s).  %p %p",value,increment);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_increment */

int Cmiss_value_scalar_multiply(Cmiss_value_id value,FE_value scalar)
/*******************************************************************************
LAST MODIFIED : 29 July 2003

DESCRIPTION :
Calculates <scalar>*<value> and puts in <value>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_value_scalar_multiply);
	return_code=0;
	/* check arguments */
	if (value)
	{
		if (value->scalar_multiply_type_specific_function)
		{
			return_code=(value->scalar_multiply_type_specific_function)(value,scalar);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_value_scalar_multiply.  "
				"Missing method");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_scalar_multiply.  "
			"Missing <value>");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_scalar_multiply */
