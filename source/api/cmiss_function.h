/*******************************************************************************
FILE : api/cmiss_function.h

LAST MODIFIED : 14 June 2004

DESCRIPTION :
The public interface to the Cmiss_function object.
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
#ifndef __API_CMISS_FUNCTION_H__
#define __API_CMISS_FUNCTION_H__

#include "api/cmiss_function_base.h"

/*
Global types
------------
*/
typedef struct Cmiss_function_list * Cmiss_function_list_id;
/*******************************************************************************
LAST MODIFIED : 14 June 2004

DESCRIPTION :
An identifier for a list of functions.
==============================================================================*/

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int Cmiss_function_destroy(Cmiss_function_id *function_address);
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
Destroys the function.  Returns a non-zero if successful and zero otherwise.
==============================================================================*/

const char *Cmiss_function_get_type_id_string(Cmiss_function_id function);
/*******************************************************************************
LAST MODIFIED : 10 March 2004

DESCRIPTION :
Returns the string which identifies the type.  The calling function must not
DEALLOCATE the returned string.
==============================================================================*/

int Cmiss_function_get_string_representation(Cmiss_function_id function,
	char **result);
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
Creates a string representation of the <function> useful for output.  Returns a
non-zero if successful and zero otherwise.  If successful <*result> contains an
ALLOCATED string, it is up to the calling function to DEALLOCATE the string when
it is no longer required.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_input(Cmiss_function_id function);
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
Returns all the inputs of the <function> as a variable.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_output(Cmiss_function_id function);
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
Returns all the outputs of the <function> as a variable.
==============================================================================*/

Cmiss_function_list_id Cmiss_function_list_create(void);
/*******************************************************************************
LAST MODIFIED : 14 June 2004

DESCRIPTION :
Creates a function list.
==============================================================================*/

int Cmiss_function_list_destroy(Cmiss_function_list_id *list_address);
/*******************************************************************************
LAST MODIFIED : 14 June 2004

DESCRIPTION :
Destroys a function list.
==============================================================================*/

int Cmiss_function_list_add(Cmiss_function_list_id list,
	Cmiss_function_id function);
/*******************************************************************************
LAST MODIFIED : 14 June 2004

DESCRIPTION :
Adds a <function> to a <list>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_H__ */
