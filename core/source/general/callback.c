/*******************************************************************************
FILE : callback.c

LAST MODIFIED : 17 March 2000

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
#include "general/callback.h"
#include "general/debug.h"
#include "general/list_private.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

#if defined (OLD_CODE)
FULL_DECLARE_LIST_TYPE(Callback_data);

struct Callback_data_call_list_struct
{
	Widget calling_widget;
	void *new_data;
};
#endif /* defined (OLD_CODE) */

/*
Module functions
----------------
*/

#if defined (OLD_CODE)
DECLARE_SIMPLE_LIST_OBJECT_FUNCTIONS(Callback_data)

DECLARE_LIST_FUNCTIONS(Callback_data)

static int callback_data_call_list(struct Callback_data *current_callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Changes a data item of the input_module widget.
==============================================================================*/
{
	int return_code;
	struct Callback_data_call_list_struct *callback_info = user_data;

	ENTER(callback_data_call_list);
	/*???DB.  The callback procedure should have a return_code. */
	return_code=1;
	(current_callback->procedure)(callback_info->calling_widget,
		current_callback->data,callback_info->new_data);
	LEAVE;

	return (return_code);
} /* callback_data_call_list */
#endif /* defined (OLD_CODE) */

/*
Global functions
----------------
*/

#if defined (OLD_CODE)
void callback_call_list(struct LIST(Callback_data) *callback_list,
	Widget calling_widget,void *new_data)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Changes a data item of the input_module widget.
==============================================================================*/
{
	struct Callback_data_call_list_struct callback_info;

	ENTER(callback_call_list);
	callback_info.calling_widget=calling_widget;
	callback_info.new_data=new_data;
	FOR_EACH_OBJECT_IN_LIST(Callback_data)(callback_data_call_list,&callback_info,
		callback_list);
	LEAVE;
} /* callback_call_list */
#endif /* defined (OLD_CODE) */
