/*******************************************************************************
FILE : api/cmiss_variable_new_input.h

LAST MODIFIED : 21 October 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new_input object.

???DB.  new refers to
- combining Cmiss_variable and Cmiss_value
- adding a input specifier
- changing to C++

???DB.  Will have to change *_id when make *_handle classes

???DB.  Lose all the smart pointer abilities and end up memory leaks?
	- this is why have destroy functions

???DB.  id is not right because many ids (different values) can be for the same
	Variable
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
#ifndef __API_CMISS_VARIABLE_NEW_INPUT_H__
#define __API_CMISS_VARIABLE_NEW_INPUT_H__

/*
Global types
------------
*/
typedef struct Cmiss_variable_new_input * Cmiss_variable_new_input_id;
/*******************************************************************************
LAST MODIFIED : 25 September 2003

DESCRIPTION :
An object that specifies an input for a Cmiss_variable_new_id.
==============================================================================*/

typedef struct Cmiss_variable_new_input_list * Cmiss_variable_new_input_list_id;
/*******************************************************************************
LAST MODIFIED : 25 September 2003

DESCRIPTION :
An object that specifies a list of inputs.
==============================================================================*/

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int Cmiss_variable_new_input_destroy(
	Cmiss_variable_new_input_id *input_address);
/*******************************************************************************
LAST MODIFIED : 20 October 2003

DESCRIPTION :
Destroys an input.
==============================================================================*/

Cmiss_variable_new_input_list_id Cmiss_variable_new_input_list_create(void);
/*******************************************************************************
LAST MODIFIED : 7 September 2003

DESCRIPTION :
Creates an input list.
==============================================================================*/

int Cmiss_variable_new_input_list_destroy(
	Cmiss_variable_new_input_list_id *list_address);
/*******************************************************************************
LAST MODIFIED : 10 September 2003

DESCRIPTION :
Destroys an input list.
==============================================================================*/

int Cmiss_variable_new_input_list_add(Cmiss_variable_new_input_list_id list,
	Cmiss_variable_new_input_id input);
/*******************************************************************************
LAST MODIFIED : 7 September 2003

DESCRIPTION :
Adds an input to a list.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_VARIABLE_NEW_INPUT_H__ */
