/*******************************************************************************
FILE : api/cmiss_variable_new_inverse.cpp

LAST MODIFIED : 2 February 2004

DESCRIPTION :
The public interface to the Cmiss_variable_new inverse object.
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
#include "api/cmiss_variable_new_inverse.h"
#include "computed_variable/variable_inverse.hpp"

/*
Global functions
----------------
*/
Cmiss_variable_new_id Cmiss_variable_new_inverse_create(
	Cmiss_variable_new_input_id dependent_variable,
	Cmiss_variable_new_id independent_variable)
/*******************************************************************************
LAST MODIFIED : 2 February 2004

DESCRIPTION :
Creates a Cmiss_variable_new inverse with the supplied <dependent_variable>
and <independent_variable>.
==============================================================================*/
{
	Cmiss_variable_new_id result;
#if defined (USE_SMART_POINTER)
	Variable_handle *independent_variable_handle_address;
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		*dependent_variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable_handle independent_variable_handle;
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		dependent_variable_handle;
#endif /* defined (USE_SMART_POINTER) */

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(dependent_variable_handle_address=
		reinterpret_cast<
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		*>(dependent_variable))&&
		(independent_variable_handle_address=reinterpret_cast<Variable_handle *>(
		independent_variable))
#else /* defined (USE_SMART_POINTER) */
		(dependent_variable_handle=reinterpret_cast<
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		>(dependent_variable))&&
		(independent_variable_handle=reinterpret_cast<Variable_handle>(
		independent_variable))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		result=reinterpret_cast<Cmiss_variable_new_id>(
#if defined (USE_SMART_POINTER)
			new Variable_inverse_handle(new Variable_inverse(
			*dependent_variable_handle_address,*independent_variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
			new Variable_inverse(dependent_variable_handle,
			*independent_variable_address)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}

Cmiss_variable_new_input_id Cmiss_variable_new_input_inverse_independent(
	Cmiss_variable_new_id variable_inverse)
/*******************************************************************************
LAST MODIFIED : 2 February 2004

DESCRIPTION :
Returns the independent input for the <variable_inverse>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	Variable_inverse_handle variable_inverse_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_inverse))&&
		(variable_inverse_handle=boost::dynamic_pointer_cast<Variable_inverse,
		Variable>(*variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(variable_inverse))&&
		(variable_inverse_handle=dynamic_cast<Variable_inverse *>(variable_address))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
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
			variable_inverse_handle->input_independent()
#if defined (USE_SMART_POINTER)
			)
#endif /* defined (USE_SMART_POINTER) */
			);
	}
	else
	{
		result=0;
	}

	return (result);
}

Cmiss_variable_new_input_id Cmiss_variable_new_input_inverse_step_tolerance(
	Cmiss_variable_new_id variable_inverse)
/*******************************************************************************
LAST MODIFIED : 2 February 2004

DESCRIPTION :
Returns the step tolerance input for the <variable_inverse>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	Variable_inverse_handle variable_inverse_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_inverse))&&
		(variable_inverse_handle=boost::dynamic_pointer_cast<Variable_inverse,
		Variable>(*variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(variable_inverse))&&
		(variable_inverse_handle=dynamic_cast<Variable_inverse *>(variable_address))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
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
			variable_inverse_handle->input_step_tolerance()
#if defined (USE_SMART_POINTER)
			)
#endif /* defined (USE_SMART_POINTER) */
			);
	}
	else
	{
		result=0;
	}

	return (result);
}

Cmiss_variable_new_input_id Cmiss_variable_new_input_inverse_value_tolerance(
	Cmiss_variable_new_id variable_inverse)
/*******************************************************************************
LAST MODIFIED : 2 February 2004

DESCRIPTION :
Returns the value tolerance input for the <variable_inverse>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	Variable_inverse_handle variable_inverse_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_inverse))&&
		(variable_inverse_handle=boost::dynamic_pointer_cast<Variable_inverse,
		Variable>(*variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(variable_inverse))&&
		(variable_inverse_handle=dynamic_cast<Variable_inverse *>(variable_address))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
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
			variable_inverse_handle->input_value_tolerance()
#if defined (USE_SMART_POINTER)
			)
#endif /* defined (USE_SMART_POINTER) */
			);
	}
	else
	{
		result=0;
	}

	return (result);
}

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_inverse_maximum_iterations(
	Cmiss_variable_new_id variable_inverse)
/*******************************************************************************
LAST MODIFIED : 2 February 2004

DESCRIPTION :
Returns the maximum_iterations input for the <variable_inverse>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	Variable_inverse_handle variable_inverse_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_inverse))&&
		(variable_inverse_handle=boost::dynamic_pointer_cast<Variable_inverse,
		Variable>(*variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(variable_inverse))&&
		(variable_inverse_handle=dynamic_cast<Variable_inverse *>(variable_address))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
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
			variable_inverse_handle->input_maximum_iterations()
#if defined (USE_SMART_POINTER)
			)
#endif /* defined (USE_SMART_POINTER) */
			);
	}
	else
	{
		result=0;
	}

	return (result);
}

Cmiss_variable_new_input_id Cmiss_variable_new_input_inverse_dependent_estimate(
	Cmiss_variable_new_id variable_inverse)
/*******************************************************************************
LAST MODIFIED : 2 February 2004

DESCRIPTION :
Returns the dependent_estimate input for the <variable_inverse>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	Variable_inverse_handle variable_inverse_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_inverse))&&
		(variable_inverse_handle=boost::dynamic_pointer_cast<Variable_inverse,
		Variable>(*variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(variable_inverse))&&
		(variable_inverse_handle=dynamic_cast<Variable_inverse *>(variable_address))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
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
			variable_inverse_handle->input_dependent_estimate()
#if defined (USE_SMART_POINTER)
			)
#endif /* defined (USE_SMART_POINTER) */
			);
	}
	else
	{
		result=0;
	}

	return (result);
}
