/*******************************************************************************
FILE : api/cmiss_function_variable.h

LAST MODIFIED : 5 March 2004

DESCRIPTION :
The public interface to the Cmiss_function_variable object.  Variables specify
inputs and/or outputs of functions.  Variables can be evaluated, differentiated
or set to another value.
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
#ifndef __API_CMISS_FUNCTION_VARIABLE_H__
#define __API_CMISS_FUNCTION_VARIABLE_H__

#include "api/cmiss_function_base.h"

/*
Global types
------------
*/
typedef struct Cmiss_function_variable_list * Cmiss_function_variable_list_id;
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
An identifier for a list of variables.
==============================================================================*/

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int Cmiss_function_variable_destroy(
	Cmiss_function_variable_id *variable_address);
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
Destroys the variable.  Returns a non-zero if successful and zero otherwise.
==============================================================================*/

int Cmiss_function_variable_get_string_representation(
	Cmiss_function_variable_id variable,char **result);
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
Creates a string representation of the <variable> useful for output.  Returns a
non-zero if successful and zero otherwise.  If successful <*result> contains an
ALLOCATED string, it is up to the calling function to DEALLOCATE the string when
it is no longer required.
==============================================================================*/

Cmiss_function_id Cmiss_function_variable_evaluate(
	Cmiss_function_variable_id variable,Cmiss_function_variable_id input,
	Cmiss_function_id value);
/*******************************************************************************
LAST MODIFIED : 5 March 2004

DESCRIPTION :
Calculates and returns the value of the <variable> with the <input> set to the
<value> during the calculation and then reset to its original value.

For an input <variable>, this function will get its current value.
==============================================================================*/

Cmiss_function_id Cmiss_function_variable_evaluate_derivative(
	Cmiss_function_variable_id variable,
	Cmiss_function_variable_list_id independent_variables,
	Cmiss_function_variable_id input,Cmiss_function_id value);
/*******************************************************************************
LAST MODIFIED : 5 March 2004

DESCRIPTION :
Calculates and returns the derivative of the <variable> with the respect to the
<independent_variables>, with the <input> set to the <value> during the
calculation and then reset to its original value.
==============================================================================*/

int Cmiss_function_variable_set_value(Cmiss_function_variable_id variable,
	Cmiss_function_id value);
/*******************************************************************************
LAST MODIFIED : 5 March 2004

DESCRIPTION :
Changes the <variable> to have the given <value>.  Returns a non-zero if the
<variable> was changed and zero otherwise.
==============================================================================*/

Cmiss_function_variable_list_id Cmiss_function_variable_list_create(void);
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
Creates a variable list.
==============================================================================*/

int Cmiss_function_variable_list_destroy(
	Cmiss_function_variable_list_id *list_address);
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
Destroys a variable list.
==============================================================================*/

int Cmiss_function_variable_list_add(Cmiss_function_variable_list_id list,
	Cmiss_function_variable_id variable);
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
Adds a <variable> to a <list>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_VARIABLE_H__ */
