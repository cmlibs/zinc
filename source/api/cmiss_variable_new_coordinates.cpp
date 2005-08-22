/*******************************************************************************
FILE : api/cmiss_variable_new_coordinates.cpp

LAST MODIFIED : 2 February 2004

DESCRIPTION :
The public interface to the Cmiss_variable_new coordinates objects.
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
#include "api/cmiss_variable_new_coordinates.h"
#include "computed_variable/variable_coordinates.hpp"

/*
Global functions
----------------
*/

Cmiss_variable_new_id
	Cmiss_variable_new_prolate_spheroidal_to_rectangular_cartesian_create(
	Scalar lambda,Scalar mu,Scalar theta,Scalar focus)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Creates a Cmiss_variable_new prolate_spheroidal_to_rectangular_cartesian.
==============================================================================*/
{
	return (
#if defined (USE_SMART_POINTER)
		reinterpret_cast<Cmiss_variable_new_id>(new Variable_handle(
			new Variable_prolate_spheroidal_to_rectangular_cartesian(lambda,mu,theta,
			focus)))
#else /* defined (USE_SMART_POINTER) */
		reinterpret_cast<Cmiss_variable_new_id>(Variable_handle(
			new Variable_prolate_spheroidal_to_rectangular_cartesian(lambda,mu,theta,
			focus)))
#endif /* defined (USE_SMART_POINTER) */
		);
}

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_prolate(
	Cmiss_variable_new_id variable_prolate_spheroidal_to_rectangular_cartesian)
/*******************************************************************************
LAST MODIFIED : 2 February 2004

DESCRIPTION :
Returns the prolate input for the
<variable_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	Variable_prolate_spheroidal_to_rectangular_cartesian_handle
		variable_prolate_spheroidal_to_rectangular_cartesian_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		boost::dynamic_pointer_cast<
		Variable_prolate_spheroidal_to_rectangular_cartesian,Variable>(*
		variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		dynamic_cast<Variable_prolate_spheroidal_to_rectangular_cartesian *>(
		variable_address))
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
			variable_prolate_spheroidal_to_rectangular_cartesian_handle->
			input_prolate()
#if defined (USE_SMART_POINTER)
			)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_lambda(
	Cmiss_variable_new_id variable_prolate_spheroidal_to_rectangular_cartesian)
/*******************************************************************************
LAST MODIFIED : 2 February 2004

DESCRIPTION :
Returns the lambda input for the
<variable_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	Variable_prolate_spheroidal_to_rectangular_cartesian_handle
		variable_prolate_spheroidal_to_rectangular_cartesian_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		boost::dynamic_pointer_cast<
		Variable_prolate_spheroidal_to_rectangular_cartesian,Variable>(*
		variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		dynamic_cast<Variable_prolate_spheroidal_to_rectangular_cartesian *>(
		variable_address))
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
			variable_prolate_spheroidal_to_rectangular_cartesian_handle->
			input_lambda()
#if defined (USE_SMART_POINTER)
			)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_mu(
	Cmiss_variable_new_id variable_prolate_spheroidal_to_rectangular_cartesian)
/*******************************************************************************
LAST MODIFIED : 2 February 2004

DESCRIPTION :
Returns the mu input for the
<variable_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	Variable_prolate_spheroidal_to_rectangular_cartesian_handle
		variable_prolate_spheroidal_to_rectangular_cartesian_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		boost::dynamic_pointer_cast<
		Variable_prolate_spheroidal_to_rectangular_cartesian,Variable>(*
		variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		dynamic_cast<Variable_prolate_spheroidal_to_rectangular_cartesian *>(
		variable_address))
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
			variable_prolate_spheroidal_to_rectangular_cartesian_handle->
			input_mu()
#if defined (USE_SMART_POINTER)
			)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_theta(
	Cmiss_variable_new_id variable_prolate_spheroidal_to_rectangular_cartesian)
/*******************************************************************************
LAST MODIFIED : 2 February 2004

DESCRIPTION :
Returns the theta input for the
<variable_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	Variable_prolate_spheroidal_to_rectangular_cartesian_handle
		variable_prolate_spheroidal_to_rectangular_cartesian_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		boost::dynamic_pointer_cast<
		Variable_prolate_spheroidal_to_rectangular_cartesian,Variable>(*
		variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		dynamic_cast<Variable_prolate_spheroidal_to_rectangular_cartesian *>(
		variable_address))
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
			variable_prolate_spheroidal_to_rectangular_cartesian_handle->
			input_theta()
#if defined (USE_SMART_POINTER)
			)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_focus(
	Cmiss_variable_new_id variable_prolate_spheroidal_to_rectangular_cartesian)
/*******************************************************************************
LAST MODIFIED : 2 February 2004

DESCRIPTION :
Returns the focus input for the
<variable_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	Variable_prolate_spheroidal_to_rectangular_cartesian_handle
		variable_prolate_spheroidal_to_rectangular_cartesian_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		boost::dynamic_pointer_cast<
		Variable_prolate_spheroidal_to_rectangular_cartesian,Variable>(*
		variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		dynamic_cast<Variable_prolate_spheroidal_to_rectangular_cartesian *>(
		variable_address))
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
			variable_prolate_spheroidal_to_rectangular_cartesian_handle->
			input_focus()
#if defined (USE_SMART_POINTER)
			)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}
