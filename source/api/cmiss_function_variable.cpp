/*******************************************************************************
FILE : api/cmiss_function_variable.cpp

LAST MODIFIED : 13 January 2005

DESCRIPTION :
The public interface to the Cmiss_function_variable object.  Variables specify
inputs and/or outputs of functions.  Variables can be evaluated, differentiated
or set to another value.
==============================================================================*/

#include <new>
#include "api/cmiss_function_variable.h"
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_derivative.hpp"
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_variable.hpp"
extern "C"
{
#include "general/debug.h"
}

/*
Global functions
----------------
*/
int Cmiss_function_variable_destroy(
	Cmiss_function_variable_id *variable_address)
/*******************************************************************************
LAST MODIFIED : 25 February 2004

DESCRIPTION :
Destroys the variable.  Returns a non-zero if successful and zero otherwise.
==============================================================================*/
{
	int return_code;
	Function_variable_handle **variable_handle_address_address;

	return_code=0;
	if ((variable_handle_address_address=
		reinterpret_cast<Function_variable_handle **>(variable_address))&&
		(*variable_handle_address_address))
	{
		// call the destructor for the object pointed to by
		//   *variable_handle_address_address (a Function_variable_handle which is
		//   a smart pointer) and then free the memory pointed to by
		//   *variable_handle_address_address
		delete *variable_handle_address_address;
		*variable_handle_address_address=0;
		return_code=1;
	}

	return (return_code);
}

int Cmiss_function_variable_get_string_representation(
	Cmiss_function_variable_id variable,char **result)
/*******************************************************************************
LAST MODIFIED : 25 February 2004

DESCRIPTION :
Creates a string representation of the <variable> useful for output.  Returns a
non-zero if successful and zero otherwise.  If successful <*result> contains an
ALLOCATED string, it is up to the calling function to DEALLOCATE the string when
it is no longer required.
==============================================================================*/
{
	const char *result_c_string;
	int return_code;
	std::string *result_string;
	Function_variable_handle *variable_handle_address;

	return_code=0;
	if ((variable_handle_address=reinterpret_cast<Function_variable_handle *>(
		variable))&&(*variable_handle_address))
	{
		if (result_string=((*variable_handle_address)->get_string_representation)())
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

Cmiss_function_id Cmiss_function_variable_evaluate(
	Cmiss_function_variable_id variable,Cmiss_function_variable_id input,
	Cmiss_function_id value)
/*******************************************************************************
LAST MODIFIED : 13 January 2005

DESCRIPTION :
See the include file.
==============================================================================*/
{
	Cmiss_function_id result;
	Function_handle temp,*value_handle_address;
	Function_variable_handle *input_handle_address,*variable_handle_address;

	result=0;
	if ((variable_handle_address=reinterpret_cast<Function_variable_handle *>(
		variable))&&(*variable_handle_address))
	{
#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
		Function_handle current_value(0);
#endif // defined (EVALUATE_RETURNS_VALUE)

		input_handle_address=reinterpret_cast<Function_variable_handle *>(input);
		value_handle_address=reinterpret_cast<Function_handle *>(value);
#if defined (EVALUATE_RETURNS_VALUE)
		if (input_handle_address&&value_handle_address)
		{
			temp=(*variable_handle_address)->evaluate(
				*input_handle_address,*value_handle_address);
		}
		else
		{
			temp=(*variable_handle_address)->evaluate();
		}
#else // defined (EVALUATE_RETURNS_VALUE)
		if (input_handle_address&&value_handle_address)
		{
			current_value=(*input_handle_address)->get_value();
			(*input_handle_address)->set_value(*value_handle_address);
		}
		if ((*variable_handle_address)->evaluate())
		{
			temp=(*variable_handle_address)->get_value();
		}
		if (current_value)
		{
			(*input_handle_address)->rset_value(current_value);
		}
#endif // defined (EVALUATE_RETURNS_VALUE)
		if (temp)
		{
			result=reinterpret_cast<Cmiss_function_id>(
				new Function_handle(temp));
		}
	}

	return (result);
}

Cmiss_function_id Cmiss_function_variable_evaluate_derivative(
	Cmiss_function_variable_id variable,
	Cmiss_function_variable_list_id independent_variables,
	Cmiss_function_variable_id input,Cmiss_function_id value)
/*******************************************************************************
LAST MODIFIED : 5 January 2005

DESCRIPTION :
See the include file.
==============================================================================*/
{
	Cmiss_function_id result;
	Function_handle *value_handle_address;
	Function_variable_handle *input_handle_address,*variable_handle_address;
	std::list<Function_variable_handle> *independent_variables_address;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
	Function_derivatnew_handle function_derivative;
	Function_variable_handle variable_derivative;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

	result=0;
	if ((variable_handle_address=reinterpret_cast<Function_variable_handle *>(
		variable))&&(*variable_handle_address)&&
		(independent_variables_address=reinterpret_cast<std::list<
		Function_variable_handle> *>(independent_variables))
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		&&(function_derivative=boost::dynamic_pointer_cast<Function_derivatnew,
		Function>((*variable_handle_address)->derivative(
		*independent_variables_address)))&&
		(variable_derivative=function_derivative->output())
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		)
	{
		input_handle_address=reinterpret_cast<Function_variable_handle *>(input);
		value_handle_address=reinterpret_cast<Function_handle *>(value);
		if (input_handle_address&&value_handle_address)
		{
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			result=reinterpret_cast<Cmiss_function_id>(
				new Function_handle(((*variable_handle_address)->
				evaluate_derivative)(*independent_variables_address,
				*input_handle_address,*value_handle_address)));
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#if defined (EVALUATE_RETURNS_VALUE)
			if ((variable_derivative->evaluate)(*input_handle_address,
				*value_handle_address))
			{
				result=reinterpret_cast<Cmiss_function_id>(new Function_handle(
					(function_derivative->matrix(*independent_variables_address))->
					get_value()));
			}
#else // defined (EVALUATE_RETURNS_VALUE)
			Function_handle current_value(0);

			if (input_handle_address&&value_handle_address)
			{
				current_value=(*input_handle_address)->get_value();
				(*input_handle_address)->set_value(*value_handle_address);
			}
			if (variable_derivative->evaluate())
			{
				result=reinterpret_cast<Cmiss_function_id>(new Function_handle(
					(function_derivative->matrix(*independent_variables_address))->
					get_value()));
			}
			if (current_value)
			{
				(*input_handle_address)->rset_value(current_value);
			}
#endif // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		}
		else
		{
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			result=reinterpret_cast<Cmiss_function_id>(
				new Function_handle(((*variable_handle_address)->
				evaluate_derivative)(*independent_variables_address)));
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			if ((variable_derivative->evaluate)())
			{
				result=reinterpret_cast<Cmiss_function_id>(new Function_handle(
					(function_derivative->matrix(*independent_variables_address))->
					get_value()));
			}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		}
	}

	return (result);
}

int Cmiss_function_variable_set_value(Cmiss_function_variable_id variable,
	Cmiss_function_id value)
/*******************************************************************************
LAST MODIFIED : 5 March 2004

DESCRIPTION :
See the include file.
==============================================================================*/
{
	int return_code;
	Function_handle *value_handle_address;
	Function_variable_handle *variable_handle_address;

	return_code=0;
	if ((variable_handle_address=reinterpret_cast<Function_variable_handle *>(
		variable))&&(*variable_handle_address)&&
		(value_handle_address=reinterpret_cast<Function_handle *>(value)))
	{
		if (((*variable_handle_address)->set_value)(*value_handle_address))
		{
			return_code=1;
		}
	}

	return (return_code);
}

Cmiss_function_variable_list_id Cmiss_function_variable_list_create(void)
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
Creates an variable list.
==============================================================================*/
{
	return (reinterpret_cast<Cmiss_function_variable_list_id>(
		new std::list<Function_variable_handle>));
}

int Cmiss_function_variable_list_destroy(
	Cmiss_function_variable_list_id *list_address)
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
Destroys an variable list.
==============================================================================*/
{
	int return_code;
	std::list<Function_variable_handle> **handle_list_address_address;

	return_code=0;
	if (handle_list_address_address=
		reinterpret_cast<std::list<Function_variable_handle> **>(list_address))
	{
		// call the destructor for the object pointed to by
		//   *handle_list_address_address (a Function_variable_handle list) and then
		//   free the memory pointed to by *handle_list_address_address
		delete *handle_list_address_address;
		*handle_list_address_address=0;
		return_code=1;
	}

	return (return_code);
}

int Cmiss_function_variable_list_add(Cmiss_function_variable_list_id list,
	Cmiss_function_variable_id variable)
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
Adds a <variable> to a <list>.
==============================================================================*/
{
	int return_code;
	std::list<Function_variable_handle> *list_address;
	Function_variable_handle *variable_handle_address;

	return_code=0;
	if ((list_address=reinterpret_cast<std::list<Function_variable_handle> *>(
		list))&&
		(variable_handle_address=reinterpret_cast<Function_variable_handle *>(
		variable)))
	{
		(list_address->push_back)(*variable_handle_address);
		return_code=1;
	}

	return (return_code);
}
