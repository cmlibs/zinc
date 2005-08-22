/*******************************************************************************
FILE : api/cmiss_variable_new_scalar.cpp

LAST MODIFIED : 2 February 2004

DESCRIPTION :
The public interface to the Cmiss_variable_new scalar object.
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
#include "api/cmiss_variable_new_scalar.h"
#include "computed_variable/variable_scalar.hpp"

/*
Global functions
----------------
*/
Cmiss_variable_new_id Cmiss_variable_new_scalar_create(Scalar value)
/*******************************************************************************
LAST MODIFIED : 11 October 2003

DESCRIPTION :
Creates a Cmiss_variable_new scalar with the supplied <value>.
==============================================================================*/
{
#if defined (USE_SMART_POINTER)
	Variable_scalar *variable_scalar;

	variable_scalar=new Variable_scalar(value);

	return (reinterpret_cast<Cmiss_variable_new_id>(new Variable_handle(
		variable_scalar)));
#else /* defined (USE_SMART_POINTER) */
	return (reinterpret_cast<Cmiss_variable_new_id>(new Variable_scalar(value)));
#endif /* defined (USE_SMART_POINTER) */
}

Cmiss_variable_new_input_id Cmiss_variable_new_input_scalar_value(
	Cmiss_variable_new_id variable_scalar)
/*******************************************************************************
LAST MODIFIED : 2 February 2004

DESCRIPTION :
Returns the value input for the <variable_scalar>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	Variable_scalar_handle variable_scalar_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_scalar))&&
		(variable_scalar_handle=boost::dynamic_pointer_cast<Variable_scalar,
		Variable>(*variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(variable_scalar))&&
		(variable_scalar_handle=dynamic_cast<Variable_scalar *>(variable_address))
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
			variable_scalar_handle->input_value()
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
