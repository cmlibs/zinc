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
