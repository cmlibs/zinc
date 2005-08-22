/*******************************************************************************
FILE : api/cmiss_function_derivative.cpp

LAST MODIFIED : 11 May 2005

DESCRIPTION :
The public interface to the Cmiss_function derivative object.
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
#include "api/cmiss_function_derivative.h"
#include "computed_variable/function_derivative.hpp"
#include "computed_variable/function_variable.hpp"

/*
Global functions
----------------
*/

Cmiss_function_id Cmiss_function_derivative_create(
	Cmiss_function_variable_id dependent_variable,
	Cmiss_function_variable_list_id independent_variables)
/*******************************************************************************
LAST MODIFIED : 17 January 2005

DESCRIPTION :
Creates a Cmiss_function derivative with the supplied <dependent_variable>
and <independent_variables>.
==============================================================================*/
{
	Cmiss_function_id result;
	Function_variable_handle *dependent_variable_handle_address;
	std::list<Function_variable_handle> *independent_variables_address;

	result=0;
	if ((dependent_variable_handle_address=reinterpret_cast<
		Function_variable_handle *>(dependent_variable))&&
		(independent_variables_address=reinterpret_cast< std::list<
		Function_variable_handle> *>(independent_variables)))
	{
		result=reinterpret_cast<Cmiss_function_id>(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			new Function_derivative_handle(new Function_derivative(
			*dependent_variable_handle_address,*independent_variables_address))
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			new Function_handle((*dependent_variable_handle_address)->derivative(
			*independent_variables_address))
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			);
	}

	return (result);
}
