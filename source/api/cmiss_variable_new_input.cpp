/*******************************************************************************
FILE : api/cmiss_variable_new_input.cpp

LAST MODIFIED : 21 October 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new_input object.

???DB.  new refers to
- combining Cmiss_variable and Cmiss_value
- adding a input specifier
- changing to C++
==============================================================================*/

#include <new>
#include "api/cmiss_variable_new_input.h"
#include "computed_variable/variable_input.hpp"
extern "C"
{
#include "general/debug.h"
}

/*
Global functions
----------------
*/
int Cmiss_variable_new_input_destroy(
	Cmiss_variable_new_input_id *input_address)
/*******************************************************************************
LAST MODIFIED : 20 October 2003

DESCRIPTION :
Destroys the input.
==============================================================================*/
{
	int return_code;
#if defined (USE_SMART_POINTER)
	Variable_input_handle **input_handle_address_address;
#else /* defined (USE_SMART_POINTER) */
	Variable_input **input_address_address;
#endif /* defined (USE_SMART_POINTER) */

	return_code=0;
	if (
#if defined (USE_SMART_POINTER)
		input_handle_address_address=
		reinterpret_cast<Variable_input_handle **>(input_address)
#else /* defined (USE_SMART_POINTER) */
		input_address_address=reinterpret_cast<Variable_input **>(input_address)
#endif /* defined (USE_SMART_POINTER) */
		)
	{
#if defined (USE_SMART_POINTER)
		// call the destructor for the object pointed to by
		//   *input_handle_address_address (a Variable_input_handle which is a smart
		//   pointer) and then free the memory pointed to by
		//   *input_handle_address_address
		delete *input_handle_address_address;
		*input_handle_address_address=0;
#else /* defined (USE_SMART_POINTER) */
		// call the destructor for the object pointed to by *input_address_address
		//   (a Variable_input) and then free the memory pointed to by
		//   *input_address_address
		delete *input_address_address;
		*input_address_address=0;
#endif /* defined (USE_SMART_POINTER) */
		return_code=1;
	}

	return (return_code);
}

Cmiss_variable_new_input_list_id Cmiss_variable_new_input_list_create(void)
/*******************************************************************************
LAST MODIFIED : 20 October 2003

DESCRIPTION :
Creates an input list.
==============================================================================*/
{
	return (reinterpret_cast<Cmiss_variable_new_input_list_id>(
		new std::list<Variable_input_handle>));
}

int Cmiss_variable_new_input_list_destroy(
	Cmiss_variable_new_input_list_id *list_address)
/*******************************************************************************
LAST MODIFIED : 6 October 2003

DESCRIPTION :
Destroys an input list.
==============================================================================*/
{
	int return_code;
	std::list<Variable_input_handle> **handle_list_address_address;

	return_code=0;
	if (handle_list_address_address=
		reinterpret_cast<std::list<Variable_input_handle> **>(list_address))
	{
		// call the destructor for the object pointed to by
		//   *handle_list_address_address (a Variable_input_handle list) and then
		//   free the memory pointed to by *handle_list_address_address
		delete *handle_list_address_address;
		*handle_list_address_address=0;
		return_code=1;
	}

	return (return_code);
}

int Cmiss_variable_new_input_list_add(
	Cmiss_variable_new_input_list_id input_list,
	Cmiss_variable_new_input_id input)
/*******************************************************************************
LAST MODIFIED : 6 October 2003

DESCRIPTION :
Adds an input to a list.
==============================================================================*/
{
	int return_code;
	std::list<Variable_input_handle> *input_list_address;
#if defined (USE_SMART_POINTER)
	Variable_input_handle *input_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable_input_handle input_handle;
#endif /* defined (USE_SMART_POINTER) */

	return_code=0;
	if ((input_list_address=
		reinterpret_cast<std::list<Variable_input_handle> *>(input_list))&&
#if defined (USE_SMART_POINTER)
		(input_handle_address=reinterpret_cast<Variable_input_handle *>(input))
#else /* defined (USE_SMART_POINTER) */
		(input_handle=reinterpret_cast<Variable_input_handle>(input))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		(input_list_address->push_back)(
#if defined (USE_SMART_POINTER)
			*input_handle_address
#else /* defined (USE_SMART_POINTER) */
			input_handle
#endif /* defined (USE_SMART_POINTER) */
			);
		return_code=1;
	}

	return (return_code);
}
