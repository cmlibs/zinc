/*******************************************************************************
FILE : api/cmiss_variable_new_inverse.h

LAST MODIFIED : 16 December 2003

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
#ifndef __API_CMISS_VARIABLE_NEW_INVERSE_H__
#define __API_CMISS_VARIABLE_NEW_INVERSE_H__

#include "api/cmiss_variable_new.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_variable_new_id Cmiss_variable_new_inverse_create(
	Cmiss_variable_new_input_id dependent_variable,
	Cmiss_variable_new_id independent_variable);
/*******************************************************************************
LAST MODIFIED : 11 December 2003

DESCRIPTION :
Creates a Cmiss_variable_new inverse with the supplied <dependent_variable>
and <independent_variable>.
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_inverse_independent(
	Cmiss_variable_new_id variable_inverse);
/*******************************************************************************
LAST MODIFIED : 11 December 2003

DESCRIPTION :
Returns the independent input for the <variable_inverse>.
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_inverse_step_tolerance(
	Cmiss_variable_new_id variable_inverse);
/*******************************************************************************
LAST MODIFIED : 16 December 2003

DESCRIPTION :
Returns the step tolerance input for the <variable_inverse>.
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_inverse_value_tolerance(
	Cmiss_variable_new_id variable_inverse);
/*******************************************************************************
LAST MODIFIED : 16 December 2003

DESCRIPTION :
Returns the value tolerance input for the <variable_inverse>.
==============================================================================*/

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_inverse_maximum_iterations(
	Cmiss_variable_new_id variable_inverse);
/*******************************************************************************
LAST MODIFIED : 11 December 2003

DESCRIPTION :
Returns the maximum_iterations input for the <variable_inverse>.
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_inverse_dependent_estimate(
	Cmiss_variable_new_id variable_inverse);
/*******************************************************************************
LAST MODIFIED : 11 December 2003

DESCRIPTION :
Returns the dependent_estimate input for the <variable_inverse>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_VARIABLE_NEW_INVERSE_H__ */
