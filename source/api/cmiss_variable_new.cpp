/*******************************************************************************
FILE : api/cmiss_variable_new.cpp

LAST MODIFIED : 10 September 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new object.

???DB.  new refers to
- combining Cmiss_variable and Cmiss_value
- adding a input specifier
- changing to C++
==============================================================================*/

#include <new>
#include <string>
using namespace std;
#include <string.h>
#include "api/cmiss_variable_new.h"
#include "computed_variable/variable.h"
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
LAST MODIFIED : 9 September 2003

DESCRIPTION :
Destroys the variable.
==============================================================================*/
{
	int return_code;

	return_code=0;
	if (variable_address)
	{
		delete *variable_address;
		*variable_address=0;
		return_code=1;
	}

	return (return_code);
}

Cmiss_variable_new_id Cmiss_variable_new_evaluate(
	Cmiss_variable_new_id variable,
	Cmiss_variable_new_input_value_list_id values)
/*******************************************************************************
LAST MODIFIED : 7 September 2003

DESCRIPTION :
Calculates the <variable> with the specified <values> over-riding, but not
setting, the current values.
==============================================================================*/
{
	return ((variable->evaluate)(values));
}

int Cmiss_variable_new_get_string_representation(Cmiss_variable_new_id variable,
	char **result)
/*******************************************************************************
LAST MODIFIED : 8 September 2003

DESCRIPTION :
Creates a string representation of the Cmiss_value useful for output.  If
successful <*result> contains an ALLOCATED string, it is up to the calling
function to DEALLOCATE the string when it is no longer required.
==============================================================================*/
{
	const char *result_c_string;
	int return_code;
	string *result_string;

	return_code=0;
	if (result_string=(variable->get_string_representation)())
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

	return (return_code);
}

Cmiss_variable_new_input_id Cmiss_variable_new_input_create(
	char *specification_string)
/*******************************************************************************
LAST MODIFIED : 8 September 2003

DESCRIPTION :
Creates the variable input.
==============================================================================*/
{
	return (new(nothrow) Variable_input(specification_string));
}

int Cmiss_variable_new_input_destroy(
	Cmiss_variable_new_input_id *input_address)
/*******************************************************************************
LAST MODIFIED : 10 September 2003

DESCRIPTION :
Destroys the variable input.
==============================================================================*/
{
	int return_code;

	return_code=0;
	if (input_address)
	{
		delete *input_address;
		*input_address=0;
		return_code=1;
	}

	return (return_code);
}

Cmiss_variable_new_input_list_id Cmiss_variable_new_input_list_create(void)
/*******************************************************************************
LAST MODIFIED : 7 September 2003

DESCRIPTION :
Creates an input list.
==============================================================================*/
{
	return (new(nothrow) list<Variable_input>);
}

int Cmiss_variable_new_input_list_destroy(
	Cmiss_variable_new_input_list_id *list_address)
/*******************************************************************************
LAST MODIFIED : 10 September 2003

DESCRIPTION :
Destroys an input list.
==============================================================================*/
{
	int return_code;

	return_code=0;
	if (list_address)
	{
		delete *list_address;
		*list_address=0;
		return_code=1;
	}

	return (return_code);
}

int Cmiss_variable_new_input_list_add(
	Cmiss_variable_new_input_list_id list,
	Cmiss_variable_new_input_id input)
/*******************************************************************************
LAST MODIFIED : 7 September 2003

DESCRIPTION :
Adds an input to a list.
==============================================================================*/
{
	int return_code;

	(list->push_back)(*input);
	return_code=1;

	return (return_code);
}

Cmiss_variable_new_input_value_list_id
	Cmiss_variable_new_input_value_list_create(void)
/*******************************************************************************
LAST MODIFIED : 7 September 2003

DESCRIPTION :
Creates an input value list.
==============================================================================*/
{
	return (new(nothrow) list<Variable_input_value>);
}

int Cmiss_variable_new_input_value_list_destroy(
	Cmiss_variable_new_input_value_list_id *list_address)
/*******************************************************************************
LAST MODIFIED : 10 September 2003

DESCRIPTION :
Destroys an input value list.
==============================================================================*/
{
	int return_code;

	return_code=0;
	if (list_address)
	{
		delete *list_address;
		*list_address=0;
		return_code=1;
	}

	return (return_code);
}

int Cmiss_variable_new_input_value_list_add(
	Cmiss_variable_new_input_value_list_id list,
	Cmiss_variable_new_input_id input,Cmiss_variable_new_id value)
/*******************************************************************************
LAST MODIFIED : 7 September 2003

DESCRIPTION :
Adds an input value to a list.
==============================================================================*/
{
	int return_code;
	Variable_input_value *input_value;

	return_code=0;
	if (input_value=new(nothrow) Variable_input_value(input,value))
	{
		(list->push_back)(*input_value);
		return_code=1;
	}

	return (return_code);
}
