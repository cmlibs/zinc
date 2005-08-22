/*******************************************************************************
FILE : api/cmiss_variable_new_vector.cpp

LAST MODIFIED : 2 February 2004

DESCRIPTION :
The public interface to the Cmiss_variable_new vector object.
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
#include "api/cmiss_variable_new_vector.h"
#include "computed_variable/variable_vector.hpp"

/*
Global functions
----------------
*/

Cmiss_variable_new_id Cmiss_variable_new_vector_create(
	unsigned int number_of_values,Scalar *values)
/*******************************************************************************
LAST MODIFIED : 24 October 2003

DESCRIPTION :
Creates a Cmiss_variable_new vector with the specified <number_of_values> and
<values>.  If <values> is NULL then the vector is initialized to zero.
==============================================================================*/
{
	Cmiss_variable_new_id result;

	result=0;
	if (0<number_of_values)
	{
		Vector values_vector(number_of_values);
		unsigned int i;

		if (values)
		{
			for (i=0;i<number_of_values;i++)
			{
				values_vector[i]=values[i];
			}
		}
		else
		{
			for (i=0;i<number_of_values;i++)
			{
				values_vector[i]=(Scalar)0;
			}
		}
#if defined (USE_SMART_POINTER)
		result=reinterpret_cast<Cmiss_variable_new_id>(new Variable_handle(
			new Variable_vector(values_vector)));
#else /* defined (USE_SMART_POINTER) */
		result=reinterpret_cast<Cmiss_variable_new_id>(Variable_handle(
			new Variable_vector(values_vector)));
#endif /* defined (USE_SMART_POINTER) */
	}

	return (result);
}

Cmiss_variable_new_input_id Cmiss_variable_new_input_vector_values(
	Cmiss_variable_new_id variable_vector,unsigned int number_of_indices,
	unsigned int *indices)
/*******************************************************************************
LAST MODIFIED : 2 February 2004

DESCRIPTION :
Returns the values input made up of the specified <indices> for the
<variable_vector>.  If <number_of_indices> is zero or <indices> is NULL then
the input refers to all values.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		input_values;
	Variable_vector_handle variable_vector_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_vector))&&
		(variable_vector_handle=boost::dynamic_pointer_cast<Variable_vector,
		Variable>(*variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(variable_vector))&&
		(variable_vector_handle=dynamic_cast<Variable_vector *>(variable_address))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		if ((0<number_of_indices)&&indices)
		{
			boost::numeric::ublas::vector<Variable_size_type> indices_vector(
				number_of_indices);
			unsigned int i;

			for (i=0;i<number_of_indices;i++)
			{
				indices_vector[i]=indices[i];
			}
			input_values=variable_vector_handle->input_values(indices_vector);
		}
		else
		{
			input_values=variable_vector_handle->input_values();
		}
		result=reinterpret_cast<Cmiss_variable_new_input_id>(
#if defined (USE_SMART_POINTER)
			new
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			(
#endif /* defined (USE_SMART_POINTER) */
			input_values
#if defined (USE_SMART_POINTER)
			)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}
