/*******************************************************************************
FILE : api/cmiss_function_inverse.cpp

LAST MODIFIED : 23 July 2004

DESCRIPTION :
The public interface to the Cmiss_function inverse object.
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
#include "api/cmiss_function_inverse.h"
#include "computed_variable/function_inverse.hpp"

/*
Global functions
----------------
*/
Cmiss_function_id Cmiss_function_inverse_create(
	Cmiss_function_variable_id dependent_variable,
	Cmiss_function_variable_id independent_variable)
/*******************************************************************************
LAST MODIFIED : 23 July 2004

DESCRIPTION :
Creates a Cmiss_function inverse with the supplied <dependent_variable>
and <independent_variable>.
==============================================================================*/
{
	Cmiss_function_id result;
	Function_variable_handle *independent_variable_handle_address;
	Function_variable_handle *dependent_variable_handle_address;

	result=0;
	if ((dependent_variable_handle_address=reinterpret_cast<
		Function_variable_handle *>(dependent_variable))&&
		(independent_variable_handle_address=reinterpret_cast<
		Function_variable_handle *>(independent_variable)))
	{
		result=reinterpret_cast<Cmiss_function_id>(new Function_inverse_handle(
			new Function_inverse(*dependent_variable_handle_address,
			*independent_variable_handle_address)));
	}

	return (result);
}

Cmiss_function_variable_id Cmiss_function_inverse_independent(
	Cmiss_function_id function_inverse)
/*******************************************************************************
LAST MODIFIED : 23 July 2004

DESCRIPTION :
Returns the independent variable for the <function_inverse>.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	Function_inverse_handle function_inverse_handle;
	Function_handle *function_handle_address;

	if ((function_handle_address=reinterpret_cast<Function_handle *>(
		function_inverse))&&
		(function_inverse_handle=boost::dynamic_pointer_cast<Function_inverse,
		Function>(*function_handle_address)))
	{
		result=reinterpret_cast<Cmiss_function_variable_id>(
			new Function_variable_handle(function_inverse_handle->independent()));
	}
	else
	{
		result=0;
	}

	return (result);
}

Cmiss_function_variable_id Cmiss_function_inverse_step_tolerance(
	Cmiss_function_id function_inverse)
/*******************************************************************************
LAST MODIFIED : 23 July 2004

DESCRIPTION :
Returns the step tolerance variable for the <function_inverse>.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	Function_inverse_handle function_inverse_handle;
	Function_handle *function_handle_address;

	if ((function_handle_address=reinterpret_cast<Function_handle *>(
		function_inverse))&&
		(function_inverse_handle=boost::dynamic_pointer_cast<Function_inverse,
		Function>(*function_handle_address)))
	{
		result=reinterpret_cast<Cmiss_function_variable_id>(
			new Function_variable_handle(function_inverse_handle->step_tolerance()));
	}
	else
	{
		result=0;
	}

	return (result);
}

Cmiss_function_variable_id Cmiss_function_inverse_value_tolerance(
	Cmiss_function_id function_inverse)
/*******************************************************************************
LAST MODIFIED : 23 July 2004

DESCRIPTION :
Returns the value tolerance variable for the <function_inverse>.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	Function_inverse_handle function_inverse_handle;
	Function_handle *function_handle_address;

	if ((function_handle_address=reinterpret_cast<Function_handle *>(
		function_inverse))&&
		(function_inverse_handle=boost::dynamic_pointer_cast<Function_inverse,
		Function>(*function_handle_address)))
	{
		result=reinterpret_cast<Cmiss_function_variable_id>(
			new Function_variable_handle(function_inverse_handle->value_tolerance()));
	}
	else
	{
		result=0;
	}

	return (result);
}

Cmiss_function_variable_id Cmiss_function_inverse_maximum_iterations(
	Cmiss_function_id function_inverse)
/*******************************************************************************
LAST MODIFIED : 23 July 2004

DESCRIPTION :
Returns the maximum_iterations variable for the <function_inverse>.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	Function_inverse_handle function_inverse_handle;
	Function_handle *function_handle_address;

	if ((function_handle_address=reinterpret_cast<Function_handle *>(
		function_inverse))&&
		(function_inverse_handle=boost::dynamic_pointer_cast<Function_inverse,
		Function>(*function_handle_address)))
	{
		result=reinterpret_cast<Cmiss_function_variable_id>(
			new Function_variable_handle(function_inverse_handle->
			maximum_iterations()));
	}
	else
	{
		result=0;
	}

	return (result);
}

Cmiss_function_variable_id Cmiss_function_inverse_dependent_estimate(
	Cmiss_function_id function_inverse)
/*******************************************************************************
LAST MODIFIED : 23 July 2004

DESCRIPTION :
Returns the dependent_estimate variable for the <function_inverse>.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	Function_inverse_handle function_inverse_handle;
	Function_handle *function_handle_address;

	if ((function_handle_address=reinterpret_cast<Function_handle *>(
		function_inverse))&&
		(function_inverse_handle=boost::dynamic_pointer_cast<Function_inverse,
		Function>(*function_handle_address)))
	{
		result=reinterpret_cast<Cmiss_function_variable_id>(
			new Function_variable_handle(function_inverse_handle->
			dependent_estimate()));
	}
	else
	{
		result=0;
	}

	return (result);
}
