/*******************************************************************************
FILE : callback.h

LAST MODIFIED : 23 March 2000

DESCRIPTION :
Macro definition for lists of callbacks between objects.
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
#if !defined (CALLBACK_H)
#define CALLBACK_H
#include "general/list.h"
#include "general/object.h"

#if defined (WIN32_USER_INTERFACE)
#include <windows.h>
#define UWM_IDLE	(WM_APP + 1)
#define UWM_NETWORK     (WM_APP + 2)
#endif /* (WIN32_USER_INTERFACE) */

/*
Global Types
------------
*/

#define CMISS_CALLBACK_FUNCTION( callback_type ) callback_function_ ## callback_type

#define TYPEDEF_CMISS_CALLBACK_FUNCTION( callback_type , object_type , \
	call_data_type , callback_function_return_type ) \
typedef callback_function_return_type CMISS_CALLBACK_FUNCTION(callback_type)(object_type,call_data_type,void *)

#define CMISS_CALLBACK_FUNCTION_RETURN_TYPE( callback_type ) callback_function_return_type__ ## callback_type
	
#define DECLARE_CMISS_CALLBACK_FUNCTION_RETURN_TYPE( callback_type , callback_function_return_type ) callback_function_return_type

#define CMISS_CALLBACK_ITEM( callback_type ) callback_item_ ## callback_type

#define DECLARE_CMISS_CALLBACK_TYPE( callback_type ) \
struct CMISS_CALLBACK_ITEM(callback_type) \

/*
Global functions
----------------
*/

#define CMISS_CALLBACK_LIST_CALL( callback_type ) callback_list_call_ ## callback_type

#define PROTOTYPE_CMISS_CALLBACK_LIST_CALL_FUNCTION( callback_type , object_type , \
	call_data_type ) \
int CMISS_CALLBACK_LIST_CALL(callback_type)( \
	struct LIST(CMISS_CALLBACK_ITEM(callback_type)) *callback_list, \
	object_type object,call_data_type call_data)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Calls every callback in <callback_list> with <object> and <call_data>.
==============================================================================*/

#define CMISS_CALLBACK_LIST_ADD_CALLBACK( callback_type ) \
	callback_list_add_callback_ ## callback_type

#define PROTOTYPE_CMISS_CALLBACK_LIST_ADD_CALLBACK_FUNCTION(callback_type ) \
int CMISS_CALLBACK_LIST_ADD_CALLBACK(callback_type)( \
	struct LIST(CMISS_CALLBACK_ITEM(callback_type)) *callback_list, \
	CMISS_CALLBACK_FUNCTION(callback_type) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Adds a callback = <function> + <user_data> to the end of <callback_list>.
==============================================================================*/

#define CMISS_CALLBACK_LIST_ADD_CALLBACK_TO_FRONT( callback_type ) \
	callback_list_add_callback_to_front_ ## callback_type

#define PROTOTYPE_CMISS_CALLBACK_LIST_ADD_CALLBACK_TO_FRONT_FUNCTION(callback_type ) \
int CMISS_CALLBACK_LIST_ADD_CALLBACK_TO_FRONT(callback_type)( \
	struct LIST(CMISS_CALLBACK_ITEM(callback_type)) *callback_list, \
	CMISS_CALLBACK_FUNCTION(callback_type) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Adds a callback = <function> + <user_data> to the front of <callback_list>.
==============================================================================*/

#define CMISS_CALLBACK_LIST_REMOVE_CALLBACK( callback_type ) \
	callback_list_remove_callback_ ## callback_type

#define PROTOTYPE_CMISS_CALLBACK_LIST_REMOVE_CALLBACK_FUNCTION(callback_type) \
int CMISS_CALLBACK_LIST_REMOVE_CALLBACK(callback_type)( \
	struct LIST(CMISS_CALLBACK_ITEM(callback_type)) *callback_list, \
	CMISS_CALLBACK_FUNCTION(callback_type) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Removes a callback = <function> + <user_data> from <callback_list>.
=============================================================================*/

#define DECLARE_CMISS_CALLBACK_TYPES( callback_type , object_type , call_data_type , callback_function_return_type ) \
	TYPEDEF_CMISS_CALLBACK_FUNCTION(callback_type,object_type,call_data_type,callback_function_return_type); \
DECLARE_CMISS_CALLBACK_TYPE(callback_type); \
DECLARE_LIST_TYPES(CMISS_CALLBACK_ITEM(callback_type))

/* note that most modules will not need to prototype functions in headers as
	 will use the following in wrappers or module functions */
#define PROTOTYPE_CMISS_CALLBACK_FUNCTIONS( callback_type , object_type , \
	call_data_type ) \
PROTOTYPE_LIST_FUNCTIONS(CMISS_CALLBACK_ITEM(callback_type)); \
PROTOTYPE_ADD_OBJECT_TO_FRONT_OF_LIST_FUNCTION(CMISS_CALLBACK_ITEM(callback_type)); \
PROTOTYPE_CMISS_CALLBACK_LIST_CALL_FUNCTION(callback_type,object_type, \
	call_data_type); \
PROTOTYPE_CMISS_CALLBACK_LIST_ADD_CALLBACK_FUNCTION(callback_type); \
PROTOTYPE_CMISS_CALLBACK_LIST_REMOVE_CALLBACK_FUNCTION(callback_type)

#endif /* !defined (CALLBACK_H) */
