/*******************************************************************************
FILE : api/cmiss_variable_new_derivative_matrix.cpp

LAST MODIFIED : 2 February 2004

DESCRIPTION :
The public interface to the Cmiss_variable_new derivative matrix object.
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
#include "api/cmiss_variable_new_derivative_matrix.h"
#include "computed_variable/variable_derivative_matrix.hpp"

/*
Global functions
----------------
*/

Cmiss_variable_new_id Cmiss_variable_new_derivative_matrix_get_matrix(
	Cmiss_variable_new_id variable_derivative_matrix,
	Cmiss_variable_new_input_list_id independent_variables)
/*******************************************************************************
LAST MODIFIED : 2 February 2004

DESCRIPTION :
If <variable_derivative_matrix> is of type derivative_matrix, this function
returns the specified partial derivative (<independent_variables>).

???DB.  Extend so that can have an independent variable that is a subset of
	one of the independent variables for the derivative matrix.  eg nodal values
	for a particular node as a subset of all nodal values
==============================================================================*/
{
	Cmiss_variable_new_id result;
	std::list<
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		> *independent_variables_address;
#if defined (USE_SMART_POINTER)
	Variable_derivative_matrix_handle *variable_derivative_matrix_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable_derivative_matrix *variable_derivative_matrix_address;
#endif /* defined (USE_SMART_POINTER) */

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(variable_derivative_matrix_handle_address=
		reinterpret_cast<Variable_derivative_matrix_handle *>(
		variable_derivative_matrix))&&
#else /* defined (USE_SMART_POINTER) */
		(variable_derivative_matrix_address=
		reinterpret_cast<Variable_derivative_matrix *>(
		variable_derivative_matrix))&&
#endif /* defined (USE_SMART_POINTER) */
		(independent_variables_address=reinterpret_cast<std::list<
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		> *>(independent_variables)))
	{
		result=reinterpret_cast<Cmiss_variable_new_id>(
#if defined (USE_SMART_POINTER)
			new Variable_handle(((*variable_derivative_matrix_handle_address)->
			matrix)(*independent_variables_address))
#else /* defined (USE_SMART_POINTER) */
			(variable_derivative_matrix_address->matrix)(
			*independent_variables_address)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}
