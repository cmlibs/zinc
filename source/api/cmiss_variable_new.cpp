/*******************************************************************************
FILE : api/cmiss_variable_new.cpp

LAST MODIFIED : 25 November 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new object.

???DB.  new refers to
- combining Cmiss_variable and Cmiss_value
- adding a input specifier
- changing to C++
==============================================================================*/

#include <new>
#include <string>
#include <string.h>
#include <typeinfo>
#include "api/cmiss_variable_new.h"
#include "computed_variable/variable.hpp"
extern "C"
{
#include "general/debug.h"
}

/*
Global functions
----------------
*/
int Cmiss_variable_new_destroy(Cmiss_variable_new_id *variable_address)
/*******************************************************************************
LAST MODIFIED : 25 September 2003

DESCRIPTION :
Destroys the variable.
==============================================================================*/
{
	int return_code;
#if defined (USE_SMART_POINTER)
	Variable_handle **variable_handle_address_address;
#else /* defined (USE_SMART_POINTER) */
	Variable **variable_address_address;
#endif /* defined (USE_SMART_POINTER) */

	return_code=0;
	if (
#if defined (USE_SMART_POINTER)
		variable_handle_address_address=
		reinterpret_cast<Variable_handle **>(variable_address)
#else /* defined (USE_SMART_POINTER) */
		variable_address_address=reinterpret_cast<Variable **>(variable_address)
#endif /* defined (USE_SMART_POINTER) */
		)
	{
#if defined (USE_SMART_POINTER)
		// call the destructor for the object pointed to by
		//   *variable_handle_address_address (a Variable_handle which is a smart
		//   pointer) and then free the memory pointed to by
		//   *variable_handle_address_address
		delete *variable_handle_address_address;
		*variable_handle_address_address=0;
#else /* defined (USE_SMART_POINTER) */
		// call the destructor for the object pointed to by
		//   *variable_address_address (a Variable) and then free the memory pointed
		//   to by *variable_address_address
		delete *variable_address_address;
		*variable_address_address=0;
#endif /* defined (USE_SMART_POINTER) */
		return_code=1;
	}

	return (return_code);
}

const char *Cmiss_variable_new_get_type_id_string(
	Cmiss_variable_new_id variable)
/*******************************************************************************
LAST MODIFIED : 16 October 2003

DESCRIPTION :
Returns the string which identifies the type.  The calling function must not
DEALLOCATE the returned string.

???DB.  Using typeid().name() which is implementation dependent.  Could add a
	get_type method to the Variable class, but this shouldn't be needed in C++
	and so shouldn't really be there
==============================================================================*/
{
	const char *result;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
	Variable_handle variable_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(variable))&&
		(variable_address= *variable_handle_address)
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(variable))
#endif /* defined (USE_SMART_POINTER) */
		)
	if (variable)
	{
		result=typeid(*variable_address).name();
		/* character representation is implementation dependent */
		/* g++ has the length at the beginning */
		while (isdigit(*result))
		{
			result++;
		}
	}
	else
	{
		result=(char *)NULL;
	}

	return (result);
}

Cmiss_variable_new_id Cmiss_variable_new_evaluate(
	Cmiss_variable_new_id variable,
	Cmiss_variable_new_input_value_list_id values)
/*******************************************************************************
LAST MODIFIED : 20 October 2003

DESCRIPTION :
Calculates the <variable> with the specified <values> over-riding, but not
setting, the current values.
==============================================================================*/
{
	Cmiss_variable_new_id result;
	std::list<Variable_input_value_handle> *values_address;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(variable))&&
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(variable))&&
#endif /* defined (USE_SMART_POINTER) */
		(values_address=reinterpret_cast<std::list<Variable_input_value_handle> *>(
		values)))
	{
		result=reinterpret_cast<Cmiss_variable_new_id>(
#if defined (USE_SMART_POINTER)
			new Variable_handle(((*variable_handle_address)->evaluate)(
			*values_address))
#else /* defined (USE_SMART_POINTER) */
			(variable_address->evaluate)(*values_address)
#endif /* defined (USE_SMART_POINTER) */
			);
	}
	else
	{
		result=0;
	}

	return (result);
}

Cmiss_variable_new_id Cmiss_variable_new_evaluate_derivative(
	Cmiss_variable_new_id variable,
	Cmiss_variable_new_input_list_id independent_variables,
	Cmiss_variable_new_input_value_list_id values)
/*******************************************************************************
LAST MODIFIED : 22 October 2003

DESCRIPTION :
Calculates the derivative of the <variable> with the respect to the
<independent_variables>, with the specified <values> over-riding, but not
setting, the current values.
==============================================================================*/
{
	Cmiss_variable_new_id result;
	std::list<Variable_input_handle> *independent_variables_address;
	std::list<Variable_input_value_handle> *values_address;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(variable))&&
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(variable))&&
#endif /* defined (USE_SMART_POINTER) */
		(independent_variables_address=reinterpret_cast<std::list<
		Variable_input_handle> *>(independent_variables))&&
		(values_address=reinterpret_cast<std::list<Variable_input_value_handle> *>(
		values)))
	{
		result=reinterpret_cast<Cmiss_variable_new_id>(
#if defined (USE_SMART_POINTER)
			new Variable_handle(((*variable_handle_address)->evaluate_derivative)(
			*independent_variables_address,*values_address))
#else /* defined (USE_SMART_POINTER) */
			(variable_address->evaluate_derivative)(*independent_variables_address,
			*values_address)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}

Cmiss_variable_new_id Cmiss_variable_new_get_input_value(
	Cmiss_variable_new_id variable,Cmiss_variable_new_input_id input)
/*******************************************************************************
LAST MODIFIED : 23 October 2003

DESCRIPTION :
Gets the specified <input> for the <variable>.
==============================================================================*/
{
	Cmiss_variable_new_id result;
	Variable_handle value;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
	Variable_input_handle *variable_input_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
	Variable_input *variable_input_address;
#endif /* defined (USE_SMART_POINTER) */

	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(variable))&&
		(variable_input_handle_address=reinterpret_cast<Variable_input_handle *>(
		input))&&
		(value=((*variable_handle_address)->get_input_value)(
		*variable_input_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(variable))&&
		(variable_input_address=reinterpret_cast<Variable_input *>(input))&&
		(value=(variable_address->get_input_value)(variable_input_address))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		result=reinterpret_cast<Cmiss_variable_new_id>(
#if defined (USE_SMART_POINTER)
			new Variable_handle(value)
#else /* defined (USE_SMART_POINTER) */
			value
#endif /* defined (USE_SMART_POINTER) */
			);
	}
	else
	{
		result=0;
	}

	return (result);
}

int Cmiss_variable_new_set_input_value(Cmiss_variable_new_id variable,
	Cmiss_variable_new_input_id input,Cmiss_variable_new_id value)
/*******************************************************************************
LAST MODIFIED : 23 October 2003

DESCRIPTION :
Sets the specified <input> for the <variable> with the given <value>.
==============================================================================*/
{
	int return_code;
#if defined (USE_SMART_POINTER)
	Variable_handle *value_handle_address,*variable_handle_address;
	Variable_input_handle *variable_input_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *value_address,*variable_address;
	Variable_input *variable_input_address;
#endif /* defined (USE_SMART_POINTER) */

	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(variable))&&
		(value_handle_address=reinterpret_cast<Variable_handle *>(value))&&
		(variable_input_handle_address=reinterpret_cast<Variable_input_handle *>(
		input))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(variable))&&
		(value_address=reinterpret_cast<Variable *>(value))&&
		(variable_input_address=reinterpret_cast<Variable_input *>(input))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		return_code=
#if defined (USE_SMART_POINTER)
			((*variable_handle_address)->set_input_value)(
			*variable_input_handle_address,*value_handle_address)
#else /* defined (USE_SMART_POINTER) */
			(variable_address->set_input_value)(variable_input_address,value_address)
#endif /* defined (USE_SMART_POINTER) */
			;
	}
	else
	{
		return_code=0;
	}

	return (return_code);
}

int Cmiss_variable_new_get_string_representation(Cmiss_variable_new_id variable,
	char **result)
/*******************************************************************************
LAST MODIFIED : 6 October 2003

DESCRIPTION :
Creates a string representation of the Cmiss_value useful for output.  If
successful <*result> contains an ALLOCATED string, it is up to the calling
function to DEALLOCATE the string when it is no longer required.
==============================================================================*/
{
	const char *result_c_string;
	int return_code;
	std::string *result_string;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	return_code=0;
	if (
#if defined (USE_SMART_POINTER)
		variable_handle_address=reinterpret_cast<Variable_handle *>(variable)
#else /* defined (USE_SMART_POINTER) */
		variable_address=reinterpret_cast<Variable *>(variable)
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		if (result_string=(
#if defined (USE_SMART_POINTER)
			(*variable_handle_address)
#else /* defined (USE_SMART_POINTER) */
			variable_address
#endif /* defined (USE_SMART_POINTER) */
			->get_string_representation)())
		{
			if (result_c_string=result_string->c_str())
			{
				if (ALLOCATE(*result,char,strlen(result_c_string)+1))
				{
					strcpy(*result,result_c_string);
					return_code=1;
				}
			}
			delete result_string;
		}
	}

	return (return_code);
}

Cmiss_variable_new_list_id Cmiss_variable_new_list_create(void)
/*******************************************************************************
LAST MODIFIED : 16 November 2003

DESCRIPTION :
Creates an variable list.
==============================================================================*/
{
	return (reinterpret_cast<Cmiss_variable_new_list_id>(
		new std::list<Variable_handle>));
}

int Cmiss_variable_new_list_destroy(
	Cmiss_variable_new_list_id *list_address)
/*******************************************************************************
LAST MODIFIED : 16 November 2003

DESCRIPTION :
Destroys an variable list.
==============================================================================*/
{
	int return_code;
	std::list<Variable_handle> **handle_list_address_address;

	return_code=0;
	if (handle_list_address_address=
		reinterpret_cast<std::list<Variable_handle> **>(list_address))
	{
		// call the destructor for the object pointed to by
		//   *handle_list_address_address (a Variable_handle list) and
		//   then free the memory pointed to by *handle_list_address_address
		delete *handle_list_address_address;
		*handle_list_address_address=0;
		return_code=1;
	}

	return (return_code);
}

int Cmiss_variable_new_list_add(Cmiss_variable_new_list_id list,
	Cmiss_variable_new_id variable)
/*******************************************************************************
LAST MODIFIED : 16 November 2003

DESCRIPTION :
Adds an variable to a list.
==============================================================================*/
{
	int return_code;
	std::list<Variable_handle> *list_address;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	return_code=0;
	if ((list_address=reinterpret_cast<std::list<Variable_handle> *>(list))&&
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(variable))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(variable))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		(list_address->push_back)(
#if defined (USE_SMART_POINTER)
			*variable_handle_address
#else /* defined (USE_SMART_POINTER) */
			variable_address
#endif /* defined (USE_SMART_POINTER) */
			);
		return_code=1;
	}

	return (return_code);
}

Cmiss_variable_new_input_value_list_id
	Cmiss_variable_new_input_value_list_create(void)
/*******************************************************************************
LAST MODIFIED : 20 October 2003

DESCRIPTION :
Creates an input value list.
==============================================================================*/
{
	return (reinterpret_cast<Cmiss_variable_new_input_value_list_id>(
		new std::list<Variable_input_value_handle>));
}

int Cmiss_variable_new_input_value_list_destroy(
	Cmiss_variable_new_input_value_list_id *list_address)
/*******************************************************************************
LAST MODIFIED : 6 October 2003

DESCRIPTION :
Destroys an input value list.
==============================================================================*/
{
	int return_code;
	std::list<Variable_input_value_handle> **handle_list_address_address;

	return_code=0;
	if (handle_list_address_address=
		reinterpret_cast<std::list<Variable_input_value_handle> **>(list_address))
	{
		// call the destructor for the object pointed to by
		//   *handle_list_address_address (a Variable_input_value_handle list) and
		//   then free the memory pointed to by *handle_list_address_address
		delete *handle_list_address_address;
		*handle_list_address_address=0;
		return_code=1;
	}

	return (return_code);
}

int Cmiss_variable_new_input_value_list_add(
	Cmiss_variable_new_input_value_list_id input_value_list,
	Cmiss_variable_new_input_id input,Cmiss_variable_new_id value)
/*******************************************************************************
LAST MODIFIED : 6 October 2003

DESCRIPTION :
Adds an input value to a list.
==============================================================================*/
{
	int return_code;
	std::list<Variable_input_value_handle> *input_value_list_address;
#if defined (USE_SMART_POINTER)
	Variable_handle *value_handle_address;
	Variable_input_handle *input_handle_address;
	Variable_input_value_handle *input_value_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *value_address;
	Variable_input *input_address;
	Variable_input_value *input_value_address;
#endif /* defined (USE_SMART_POINTER) */

	return_code=0;
	if ((input_value_list_address=reinterpret_cast<std::list<
		Variable_input_value_handle> *>(input_value_list))&&
#if defined (USE_SMART_POINTER)
		(input_handle_address=reinterpret_cast<Variable_input_handle *>(input))&&
		(value_handle_address=reinterpret_cast<Variable_handle *>(value))
#else /* defined (USE_SMART_POINTER) */
		(input_address=reinterpret_cast<Variable_input *>(input))&&
		(value_address=reinterpret_cast<Variable *>(value))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		if (
#if defined (USE_SMART_POINTER)
			input_value_handle_address=new Variable_input_value_handle(
			new Variable_input_value(*input_handle_address,
			*value_handle_address))
#else /* defined (USE_SMART_POINTER) */
			input_value_address=
			new Variable_input_value(input_address,value_address)
#endif /* defined (USE_SMART_POINTER) */
			)
		{
			(input_value_list_address->push_back)(
#if defined (USE_SMART_POINTER)
				*input_value_handle_address
#else /* defined (USE_SMART_POINTER) */
				input_value_address
#endif /* defined (USE_SMART_POINTER) */
				);
			return_code=1;
		}
	}

	return (return_code);
}
