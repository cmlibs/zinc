/*******************************************************************************
FILE : api/cmiss_function.cpp

LAST MODIFIED : 6 August 2004

DESCRIPTION :
The public interface to the Cmiss_function object.
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

#include <new>
#include <string>
#include <string.h>

#include "api/cmiss_function.h"
#include "computed_variable/function.hpp"
extern "C"
{
#include "general/debug.h"
}

/*
Global functions
----------------
*/
int Cmiss_function_destroy(Cmiss_function_id *function_address)
/*******************************************************************************
LAST MODIFIED : 25 February 2004

DESCRIPTION :
Destroys the function.  Returns a non-zero if successful and zero otherwise.
==============================================================================*/
{
	int return_code;
	Function_handle **function_handle_address_address;

	return_code=0;
	if ((function_handle_address_address=
		reinterpret_cast<Function_handle **>(function_address))&&
		(*function_handle_address_address))
	{
		// call the destructor for the object pointed to by
		//   *function_handle_address_address (a Function_handle which is a smart
		//   pointer) and then free the memory pointed to by
		//   *function_handle_address_address
		delete *function_handle_address_address;
		*function_handle_address_address=0;
		return_code=1;
	}

	return (return_code);
}

const char *Cmiss_function_get_type_id_string(Cmiss_function_id function)
/*******************************************************************************
LAST MODIFIED : 6 August 2004

DESCRIPTION :
Returns the string which identifies the type.  The calling function must not
DEALLOCATE the returned string.

???DB.  Using typeid().name() which is implementation dependent.  Could add a
	get_type method to the Function class, but this shouldn't be needed in C++
	and so shouldn't really be there
==============================================================================*/
{
	const char *result;
	Function_handle function_address,*function_handle_address;

	if ((function_handle_address=reinterpret_cast<Function_handle *>(function))&&
		(function_address= *function_handle_address))
	{
		result=typeid(*function_address).name();
		/* character representation is implementation dependent */
		/* g++ has the length at the beginning */
		while (isdigit(*result))
		{
			result++;
		}
		/* kludge because for a template class, the name seems to be mangled
			eg. Function_matrix<Scalar> gives "15Function_matrixIdE" (note the extra
			characters after the 15 for Function_matrix */
		{
			static char *static_Function_matrix_string="Function_matrix";

			if (0==strncmp(static_Function_matrix_string,result,
				strlen(static_Function_matrix_string)))
			{
				result=static_Function_matrix_string;
			}
		}
	}
	else
	{
		result=(char *)NULL;
	}

	return (result);
}

int Cmiss_function_get_string_representation(Cmiss_function_id function,
	char **result)
/*******************************************************************************
LAST MODIFIED : 25 February 2004

DESCRIPTION :
Creates a string representation of the <function> useful for output.  Returns a
non-zero if successful and zero otherwise.  If successful <*result> contains an
ALLOCATED string, it is up to the calling function to DEALLOCATE the string when
it is no longer required.
==============================================================================*/
{
	const char *result_c_string;
	int return_code;
	std::string *result_string;
	Function_handle *function_handle_address;

	return_code=0;
	if ((function_handle_address=reinterpret_cast<Function_handle *>(function))&&
		(*function_handle_address))
	{
		if (result_string=((*function_handle_address)->get_string_representation)())
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

Cmiss_function_variable_id Cmiss_function_input(Cmiss_function_id function)
/*******************************************************************************
LAST MODIFIED : 25 February 2004

DESCRIPTION :
Returns all the inputs of the <function> as a variable.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	Function_handle *function_handle_address;

	if ((function_handle_address=reinterpret_cast<Function_handle *>(function))&&
		(*function_handle_address))
	{
		result=reinterpret_cast<Cmiss_function_variable_id>(
			new Function_variable_handle(((*function_handle_address)->input)()));
	}
	else
	{
		result=0;
	}

	return (result);
}

Cmiss_function_variable_id Cmiss_function_output(Cmiss_function_id function)
/*******************************************************************************
LAST MODIFIED : 25 February 2004

DESCRIPTION :
Returns all the outputs of the <function> as a variable.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	Function_handle *function_handle_address;

	if ((function_handle_address=reinterpret_cast<Function_handle *>(function))&&
		(*function_handle_address))
	{
		result=reinterpret_cast<Cmiss_function_variable_id>(
			new Function_variable_handle(((*function_handle_address)->output)()));
	}
	else
	{
		result=0;
	}

	return (result);
}

Cmiss_function_list_id Cmiss_function_list_create(void)
/*******************************************************************************
LAST MODIFIED : 14 June 2004

DESCRIPTION :
Creates an function list.
==============================================================================*/
{
	return (reinterpret_cast<Cmiss_function_list_id>(
		new std::list<Function_handle>));
}

int Cmiss_function_list_destroy(Cmiss_function_list_id *list_address)
/*******************************************************************************
LAST MODIFIED : 14 June 2004

DESCRIPTION :
Destroys an function list.
==============================================================================*/
{
	int return_code;
	std::list<Function_handle> **handle_list_address_address;

	return_code=0;
	if (handle_list_address_address=
		reinterpret_cast<std::list<Function_handle> **>(list_address))
	{
		// call the destructor for the object pointed to by
		//   *handle_list_address_address (a Function_handle list) and then
		//   free the memory pointed to by *handle_list_address_address
		delete *handle_list_address_address;
		*handle_list_address_address=0;
		return_code=1;
	}

	return (return_code);
}

int Cmiss_function_list_add(Cmiss_function_list_id list,
	Cmiss_function_id function)
/*******************************************************************************
LAST MODIFIED : 14 June 2004

DESCRIPTION :
Adds a <function> to a <list>.
==============================================================================*/
{
	int return_code;
	std::list<Function_handle> *list_address;
	Function_handle *function_handle_address;

	return_code=0;
	if ((list_address=reinterpret_cast<std::list<Function_handle> *>(list))&&
		(function_handle_address=reinterpret_cast<Function_handle *>(function)))
	{
		(list_address->push_back)(*function_handle_address);
		return_code=1;
	}

	return (return_code);
}
